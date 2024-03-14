/*
 **************************************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 **************************************************************************************************
*/

#include <linux/videodev2.h>
#include <algorithm>
#include <iostream>
#include <string>

#include "FFYUVParser.h"
#include "UBWC_Utils.h"

#define ALIGN(num, to) (((num) + (to - 1)) & (~(to - 1)))

FFYUVParser::FFYUVParser(std::string inputPath, std::string videoSize,
                         std::string pixelFmt, std::string sessionId)
    : mInputPath(inputPath),
      mVideoSize(videoSize),
      mPixelFmt(pixelFmt),
      mSessionId(sessionId) {}

FFYUVParser::~FFYUVParser() {}

static bool IsCompressedPixelFormat(const std::string& pixelFmt) {
    return pixelFmt == std::string("qc08c") || pixelFmt == std::string("qc10c");
}

std::string FFYUVParser::id() {
    return mSessionId;
}

int FFYUVParser::init() {
    int ret = 0;

    std::transform(mPixelFmt.begin(), mPixelFmt.end(), mPixelFmt.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (av_parse_video_size(&mFrameWidth, &mFrameHeight, mVideoSize.c_str()) < 0) {
        return -EINVAL;
    }

    if (IsCompressedPixelFormat(mPixelFmt)) {
        mInputFile = fopen(mInputPath.c_str(), "rb");
        if (!mInputFile) {
            std::cerr << "[" << mSessionId << "]: Error: Open input file "
                      << mInputPath << " failed!\n";
            return -EINVAL;
        }
        std::cout << "[" << mSessionId
                  << "]: Open compressed-format input file " << mInputPath
                  << std::endl;
    } else {
        av_dict_set(&mFmtOptions, "video_size", mVideoSize.c_str(), 0);
        av_dict_set(&mFmtOptions, "pixel_format", mPixelFmt.c_str(), 0);
        ret = avformat_open_input(&mFmtCtx, mInputPath.c_str(), nullptr, &mFmtOptions);
        if (ret) {
            std::cerr << "[" << mSessionId << "]: Error: Open input file failed"
                      << std::endl;
            return ret;
        }

        ret = avformat_find_stream_info(mFmtCtx, nullptr);
        if (ret) {
            std::cerr << "[" << mSessionId
                      << "]: Error: Cannot find stream information"
                      << std::endl;
            return ret;
        }

        av_dump_format(mFmtCtx, 0, mInputPath.c_str(), 0);

        mPkt = av_packet_alloc();
        if (!mPkt) {
            std::cerr << "[" << mSessionId
                      << "]: Error: cannot allocate AVPacket" << std::endl;
            return -ENOMEM;
        }
    }

    return ret;
}

int FFYUVParser::fillPacketData(void* dst, int width, int height, int stride, int scanline,
                                int colorFormat, bool& eos) {
    uint8_t* pbuf = nullptr;
    uint8_t* ptarget = nullptr;
    int uvScanline, bufSize;
    auto fillCompressedPacketData = [&]() -> int {
        int parserRet, pktSize = 0;
        // fill ubwc data
        if (!mInputFile) {
            std::cerr << "[" << mSessionId
                      << "]: Error: input file not found!\n";
            return -EINVAL;
        }
        pktSize = getBufferSizeUsed(colorFormat, width, height);
        std::cout << "[" << mSessionId
                  << "]: Get compressed pkt size:" << pktSize << std::endl;
        parserRet = fread(dst, 1, pktSize, mInputFile);
        if (parserRet != pktSize) {
            eos = true;
            std::cout << "[" << mSessionId << "]: End of file\n";
        }
        return pktSize;
    };
    auto fillUncompressedPacketData = [&]() -> int {
        int parserRet, pktSize = 0;
        // fill nv12 data
        while (1) {
            parserRet = av_read_frame(mFmtCtx, mPkt);
            if (parserRet == AVERROR(EAGAIN)) {
                continue;
            }
            if (parserRet < 0) {
                if (parserRet == AVERROR_EOF) {
                    std::cout << "[" << mSessionId << "]: EOF." << std::endl;
                    eos = true;
                } else {
                    std::cerr << "[" << mSessionId << "]: Error: parse failed."
                              << std::endl;
                }
                break;
            }
            switch (colorFormat) {
                case V4L2_PIX_FMT_NV12: {
                    uint8_t* pData = mPkt->data;

                    std::cout << "[" << mSessionId
                              << "]: Filled pkt size:" << mPkt->size
                              << ", Width:" << width << ", height:" << height
                              << ", stride:" << stride
                              << ", scanline:" << scanline << std::endl;
                    uvScanline = ALIGN((height + 1) >> 1, 16);
                    bufSize = stride * scanline + stride * uvScanline;
                    pbuf = new (std::nothrow) uint8_t[bufSize];
                    if (pbuf == nullptr) {
                        std::cerr << "[" << mSessionId
                                  << "]: Error: failed to allocate pbuf."
                                  << std::endl;
                        return 0;
                    }
                    memset(pbuf, 0, bufSize);
                    ptarget = pbuf;
                    if (width == stride) {
                        // Y Plane
                        memcpy(ptarget, pData, width * height);
                        // UV Plane
                        ptarget += stride * scanline;
                        pData += width * height;
                        memcpy(ptarget, pData, width * height / 2);
                    } else {
                        // Y Plane
                        for (int i = 0; i < height; i++) {
                            memcpy(ptarget, pData, width);
                            ptarget += stride;
                            pData += width;
                        }
                        ptarget += (scanline - height) * stride;
                        // UV Plane
                        for (int i = 0; i < height / 2; i++) {
                            memcpy(ptarget, pData, width);
                            ptarget += stride;
                            pData += width;
                        }
                    }
                    memcpy(dst, pbuf, bufSize);
                    delete[] pbuf;
                    pbuf = nullptr;
                    break;
                }
                default:
                    break;
            }
            pktSize = mPkt->size ? bufSize : mPkt->size;
            av_packet_unref(mPkt);
            break;
        }
        return pktSize;
    };

    if (IsCompressedPixelFormat(mPixelFmt)) {
        return fillCompressedPacketData();
    } else {
        return fillUncompressedPacketData();
    }
}

int FFYUVParser::deinit() {
    if (mFmtOptions) {
        av_dict_free(&mFmtOptions);
        mFmtOptions = nullptr;
    }
    if (mFmtCtx) {
        avformat_close_input(&mFmtCtx);
        mFmtCtx = nullptr;
    }
    if (mPkt) {
        av_packet_free(&mPkt);
        mPkt = nullptr;
    }
    if (mInputFile) {
        fclose(mInputFile);
        mInputFile = nullptr;
    }
    return 0;
}

int FFYUVParser::loopPackets() {
    int frameCnt = 0;

    while (1) {
        int ret = av_read_frame(mFmtCtx, mPkt);
        if (ret) {
            std::cerr << "[" << mSessionId << "]: Error: Read frame failed"
                      << std::endl;
            return ret;
        }
        std::cout << "[" << mSessionId << "]: Cnt:" << frameCnt++
                  << ", pos:" << mPkt->pos << ", Pkt size:" << mPkt->size
                  << std::endl;
        av_packet_unref(mPkt);
    }
    return 0;
}
