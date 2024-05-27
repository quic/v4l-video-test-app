/*
 **************************************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 **************************************************************************************************
*/

#include <linux/videodev2.h>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include "V4l2Driver.h"
#include "FFStreamParser.h"

FFStreamParser::FFStreamParser(std::string inputPath, std::string sessionId)
    : mInputPath(inputPath), mSessionId(sessionId) {}

FFStreamParser::~FFStreamParser() {}

static bool isRawVideo(std::string long_name) {
    if (long_name == "raw H.264 video" || long_name == "raw HEVC video" || long_name == "On2 IVF") {
        return true;
    }
    return false;
}

std::string FFStreamParser::id() {
    return mSessionId;
}

int FFStreamParser::init() {
    AVCodecParameters* codecpar;
    const AVBitStreamFilter* filter = nullptr;
    int video_idx = 0, ret = 0;

    ret = avformat_open_input(&mFmtCtx, mInputPath.c_str(), nullptr, nullptr);
    if (ret) {
        std::cerr << "[" << mSessionId << "]: Error: Open input file failed"
                  << std::endl;
        return ret;
    }

    ret = avformat_find_stream_info(mFmtCtx, nullptr);
    if (ret) {
        std::cerr << "[" << mSessionId
                  << "]: Error: Cannot find stream information" << std::endl;
        return ret;
    }

    av_dump_format(mFmtCtx, 0, mInputPath.c_str(), 0);

    if (!mFmtCtx->iformat) {
        std::cerr << "[" << mSessionId << "]: Error: empty input format!\n";
        return -EINVAL;
    }
    if (mFmtCtx->iformat->long_name) {
        mRawVideo = isRawVideo(mFmtCtx->iformat->long_name);
        std::cout << "[" << mSessionId << "]: Is raw video: " << mRawVideo
                  << std::endl;
    }

    ret = av_find_best_stream(mFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (ret) {
        std::cerr << "[" << mSessionId
                  << "]: Error: Cannot find a video stream in the input file"
                  << std::endl;
        return ret;
    }
    video_idx = ret;

    mStream = mFmtCtx->streams[video_idx];
    codecpar = mStream->codecpar;
    mWidth = codecpar->width ? codecpar->width : 320;
    mHeight = codecpar->width ? codecpar->width : 320;

    AVRational framerate = av_guess_frame_rate(mFmtCtx, mStream, nullptr);
    mFps_n = framerate.num;
    mFps_d = framerate.den;

    switch (codecpar->codec_id) {
        case AV_CODEC_ID_H264:
            mCodecFmt = V4L2_PIX_FMT_H264;
            filter = av_bsf_get_by_name("h264_mp4toannexb");
            break;
        case AV_CODEC_ID_HEVC:
            mCodecFmt = V4L2_PIX_FMT_HEVC;
            filter = av_bsf_get_by_name("hevc_mp4toannexb");
            break;
        case AV_CODEC_ID_VP9:
            mCodecFmt = V4L2_PIX_FMT_VP9;
            break;
        case AV_CODEC_ID_AV1:
            mCodecFmt = V4L2_PIX_FMT_AV1;
            break;
        default:
            std::cerr << "[" << mSessionId << "]: Error: unsupported codec."
                      << std::endl;
            return -EINVAL;
    }

    if (filter) {
        ret = av_bsf_alloc(filter, &mBsf);
        if (ret) {
            std::cerr << "[" << mSessionId
                      << "]: Error: Alloc bitstream filter failed" << std::endl;
            return ret;
        }

        avcodec_parameters_copy(mBsf->par_in, codecpar);
        mBsf->time_base_in = mStream->time_base;
        ret = av_bsf_init(mBsf);
        if (ret) {
            std::cerr << "[" << mSessionId
                      << "]: Error: Init bitstream filter failed" << std::endl;
            return ret;
        }
    }

    mPkt = av_packet_alloc();
    if (!mPkt) {
        std::cerr << "[" << mSessionId << "]: Error: cannot allocate AVPacket"
                  << std::endl;
        return -ENOMEM;
    }

    return 0;
}

int FFStreamParser::getNextPacket() {
    int ret = 0;

    if (!mBsfDataPending) {
        ret = av_read_frame(mFmtCtx, mPkt);
        if (ret) {
            std::cerr << "[" << mSessionId << "]: Error: Read frame failed"
                      << std::endl;
            return ret;
        }

        if (mPkt->stream_index != mStream->index) {
            av_packet_unref(mPkt);
            return -EINVAL;
        }

        if (mBsf) {
            ret = av_bsf_send_packet(mBsf, mPkt);
            if (ret) {
                std::cerr << "[" << mSessionId
                          << "]: Error: Send pkt to bitstream filter failed"
                          << std::endl;
                return ret;
            }
            mBsfDataPending = true;
        }
    }

    if (mBsf) {
        ret = av_bsf_receive_packet(mBsf, mPkt);
        if (ret == AVERROR(EAGAIN)) {
            mBsfDataPending = false;
        }
        if (ret < 0) {
            return ret;
        }
    }

    return ret;
}

int FFStreamParser::fillPacketData(void* dst, bool& eos) {
    int pktSize = 0;

    while (1) {
        int parserRet = getNextPacket();
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

        memcpy(dst, mPkt->data, mPkt->size);
        pktSize = mPkt->size;
        av_packet_unref(mPkt);
        break;
    }

    return pktSize;
}

void FFStreamParser::deinit() {
    mStream = nullptr;
    if (mBsf) {
        av_bsf_free(&mBsf);
        mBsf = nullptr;
    }
    if (mFmtCtx) {
        avformat_close_input(&mFmtCtx);
        mFmtCtx = nullptr;
    }
    if (mPkt) {
        av_packet_free(&mPkt);
        mPkt = nullptr;
    }
}

int FFStreamParser::seekToFrame(int frame) {
    if (!mRawVideo) {
        int64_t seekPos = frame * AV_TIME_BASE * mFps_d / mFps_n;
        std::cout << "[" << mSessionId << "]: Seek position:" << seekPos
                  << std::endl;
        int ret = av_seek_frame(mFmtCtx, -1, seekPos, AVSEEK_FLAG_ANY | AVSEEK_FLAG_BACKWARD);
        if (ret < 0) {
            std::cout << "[" << mSessionId
                      << "]: Error: failed to seek to frame " << frame
                      << std::endl;
            return ret;
        }
    } else {
        uint64_t pos = 0;
        if (mPktPosition.find(frame) == mPktPosition.end()) {
            return -1;
        } else {
            pos = mPktPosition[frame];
            std::cout << "[" << mSessionId << "]: Seek to pos:" << pos
                      << std::endl;
        }

        int ret = av_seek_frame(mFmtCtx, -1, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_BYTE);
        if (ret < 0) {
            std::cout << "[" << mSessionId
                      << "]: Error: failed to seek to frame " << frame
                      << std::endl;
            return ret;
        }
    }

    return 0;
}

int FFStreamParser::randomSeek() {
    std::srand(std::time(nullptr));
    int rand_seekto = std::rand() % mTotalFrameCnt;
    if (seekToFrame(rand_seekto) < 0) {
        return -1;
    }
    return rand_seekto;
}

int FFStreamParser::loopPackets() {
    int framecnt = 0;
    if (!mRawVideo) {
        std::cout << "[" << mSessionId << "]: Container format, just exit."
                  << std::endl;
        return 0;
    }
    while (1) {
        int parserRet = getNextPacket();
        if (parserRet == AVERROR(EAGAIN)) {
            continue;
        }
        if (parserRet < 0) {
            if (parserRet == AVERROR_EOF) {
                std::cout << "[" << mSessionId << "]: EOF." << std::endl;
            } else {
                std::cerr << "[" << mSessionId << "]: Error: parse failed."
                          << std::endl;
            }
            break;
        }
        mPktPosition[framecnt++] = mPkt->pos;
        av_packet_unref(mPkt);
    }
    mTotalFrameCnt = framecnt;
    std::cout << "[" << mSessionId << "]: Total frame count:" << mTotalFrameCnt
              << std::endl;
    seekToFrame(0);
    return 0;
}
