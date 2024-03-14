/*
 **************************************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 **************************************************************************************************
*/

#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <climits>

#include "FFStreamParser.h"
#include "V4l2Decoder.h"

#define MAX_COLOR_FMTS 7
#define INPUT_TIMEOUT INT_MAX

V4l2Decoder::V4l2Decoder(unsigned int codec, unsigned int pixel,
                         std::string sessionId)
    : V4l2Codec(codec, pixel, sessionId), mStreamParser(nullptr) {}

V4l2Decoder::~V4l2Decoder() {}

int V4l2Decoder::init() {
    struct v4l2_format fmt;
    struct v4l2_control ctrl;
    struct v4l2_selection sel;
    struct v4l2_capability caps;
    int ret = 0;
    mDomain = V4L2_CODEC_TYPE_DECODER;

    ret = mV4l2Driver->Open(mDomain);
    if (ret) {
        return ret;
    }
    ret = mV4l2Driver->setCodecPixelFmt(INPUT_MPLANE, mCodecFmt);
    if (ret) {
        return ret;
    }
    ret = mV4l2Driver->setCodecPixelFmt(OUTPUT_MPLANE, mPixelFmt);
    if (ret) {
        return ret;
    }
    ret = mV4l2Driver->OpenDMAHeap("system");
    if (ret) {
        return ret;
    }
    ret = mV4l2Driver->subscribeEvent(V4L2_EVENT_SOURCE_CHANGE);
    if (ret) {
        return ret;
    }
    ret = mV4l2Driver->subscribeEvent(V4L2_EVENT_EOS);
    if (ret) {
        return ret;
    }
    ret = mV4l2Driver->createPollThread();
    if (ret) {
        return ret;
    }
    memset(&caps, 0, sizeof(caps));
    ret = mV4l2Driver->queryCapabilities(&caps);
    if (ret) {
        return ret;
    }

    return 0;
}

int V4l2Decoder::initFFStreamParser(std::string inputPath) {
    int ret = 0;

    mStreamParser = std::make_shared<FFStreamParser>(inputPath, mSessionId);

    ret = mStreamParser->init();
    if (ret) {
        return ret;
    }

    ret = mStreamParser->loopPackets();
    if (ret) {
        return ret;
    }

    return 0;
}

void V4l2Decoder::deinitFFStreamParser() {
    mStreamParser->deinit();
}

void V4l2Decoder::deinit() {
    mV4l2Driver->stopPollThread();
    mV4l2Driver->unsubscribeEvent(V4L2_EVENT_EOS);
    mV4l2Driver->unsubscribeEvent(V4L2_EVENT_SOURCE_CHANGE);
    mV4l2Driver->Close();
    mV4l2Driver->CloseDMAHeap();
}

int V4l2Decoder::configureInput() {
    LOG("V4l2Decoder::configureInput().\n");
    struct v4l2_format fmt;
    struct v4l2_requestbuffers reqBufs;
    struct v4l2_control ctrl;
    int ret = 0;

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = INPUT_MPLANE;
    ret = mV4l2Driver->getFormat(&fmt);
    if (ret) {
        return ret;
    }

    fmt.fmt.pix_mp.width = mWidth;
    fmt.fmt.pix_mp.height = mHeight;
    ret = mV4l2Driver->setFormat(&fmt);
    if (ret) {
        return ret;
    }

    mStride = fmt.fmt.pix_mp.plane_fmt[0].bytesperline;
    mInputSize = fmt.fmt.pix_mp.plane_fmt[0].sizeimage;
    LOG("configureInput: width(%d),height(%d),stride(%d),inputSize(%d)\n",
        mWidth, mHeight, mStride, mInputSize);

#if 0
    ctrl.id = V4L2_CID_MIN_BUFFERS_FOR_OUTPUT;
    ret = mV4l2Driver->getControl(&ctrl);
    if (ret) {
        return ret;
    }
    mMinInputCount = ctrl.value;
#endif

    if (mActualInputCount < mMinInputCount) {
        LOG("update input count from %d to %d\n", mActualInputCount,
            mMinInputCount);
        mActualInputCount = mMinInputCount;
    }

    LOG("configureInput: MinInputCount(%d),ActualInputCount(%d)\n",
        mMinInputCount, mActualInputCount);
    memset(&reqBufs, 0, sizeof(reqBufs));
    reqBufs.type = INPUT_MPLANE;
    reqBufs.memory = V4L2_MEMORY_DMABUF;
    reqBufs.count = mActualInputCount;
    ret = mV4l2Driver->reqBufs(&reqBufs);
    if (ret) {
        return ret;
    }
    mActualInputCount = reqBufs.count;
    LOG("configureInput: %d input buffers got from reqBufs\n",
        mActualInputCount);

    return 0;
}

int V4l2Decoder::configureOutput() {
    struct v4l2_format fmt;
    struct v4l2_frmsizeenum fsize;
    struct v4l2_requestbuffers reqBufs;
    struct v4l2_control ctrl;
    struct v4l2_queryctrl queryctrl;
    struct v4l2_selection sel;
    int ret = 0;
    LOG("V4l2Decoder::configureOutput()\n");

    /* set output format */
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = OUTPUT_MPLANE;
    ret = mV4l2Driver->getFormat(&fmt);
    if (ret) {
        return ret;
    }
    fmt.fmt.pix_mp.width = mWidth;
    fmt.fmt.pix_mp.height = mHeight;

    auto mOutputMatrixCoeff = fmt.fmt.pix_mp.ycbcr_enc;
    auto mOutputTransferChar = fmt.fmt.pix_mp.xfer_func;
    auto mOutputVideoRange = fmt.fmt.pix_mp.quantization;
    auto mOutputColorPrimaries = fmt.fmt.pix_mp.colorspace;

    LOG("%s: O/P buffer getFmt: MatrixCoeff[%d] TransferChar[%d]",
            __func__, mOutputMatrixCoeff, mOutputTransferChar);

    LOG("%s: O/P buffer getFmt: VideoRange[%d] ColorPrimaries[%d]",
            __func__, mOutputVideoRange, mOutputColorPrimaries);

    ret = mV4l2Driver->setFormat(&fmt);
    if (ret) {
        return ret;
    }
    mOBufWidth = fmt.fmt.pix_mp.width;
    mOBufHeight = fmt.fmt.pix_mp.height;
    mStride = fmt.fmt.pix_mp.plane_fmt[0].bytesperline;
    mOutputSize = fmt.fmt.pix_mp.plane_fmt[0].sizeimage;
    LOG("%s: output Buffer(%dx%d), Stride(%d), OutputSize (%d)\n", __func__,
        mOBufWidth, mOBufHeight, mStride, mOutputSize);

    /* query driver recommended framesizes */
    memset(&fsize, 0, sizeof(fsize));
    fsize.index = 0;
    fsize.pixel_format = mPixelFmt;
    ret = mV4l2Driver->enumFramesize(&fsize);
    if (ret) {
        return ret;
    }

#if 0
    ctrl.id = V4L2_CID_MIN_BUFFERS_FOR_CAPTURE;
    ret = mV4l2Driver->getControl(&ctrl);
    if (ret) {
        return ret;
    }
    mMinOutputCount = ctrl.value;
#endif

    if (mActualOutputCount < mMinOutputCount) {
        LOG("update output count from %d to %d\n", mActualOutputCount,
            mMinOutputCount);
        mActualOutputCount = mMinOutputCount;
    }

    memset(&reqBufs, 0, sizeof(reqBufs));
    reqBufs.type = OUTPUT_MPLANE;
    reqBufs.memory = V4L2_MEMORY_DMABUF;
    reqBufs.count = mActualOutputCount;
    ret = mV4l2Driver->reqBufs(&reqBufs);
    if (ret) {
        return ret;
    }
    mActualOutputCount = reqBufs.count;
    LOG("%s: %d input buffers got from reqBufs\n", __func__,
        mActualOutputCount);

    return 0;
}

static inline bool isCompressedColorFmt(unsigned int colorformat) {
    return colorformat == V4L2_PIX_FMT_QC08C || colorformat == V4L2_PIX_FMT_QC10C;
}

static inline bool isLinearColorFmt(unsigned int colorformat) {
    return colorformat == V4L2_PIX_FMT_NV12 || colorformat == V4L2_PIX_FMT_NV21 ||
           // colorformat == V4L2_PIX_FMT_VIDC_P010 ||
           colorformat == V4L2_PIX_FMT_RGBA32;
}

bool V4l2Decoder::detectBitDepthChange() {
    bool isCompressedFmt, found = false;
    struct v4l2_fmtdesc fmtdesc;
    int driverSupportedFmts[MAX_COLOR_FMTS] = {0};

    /* check if driver supports client requested colorfomat */
    memset(&fmtdesc, 0, sizeof(fmtdesc));
    fmtdesc.index = 0;
    fmtdesc.type = OUTPUT_MPLANE;
    while (!mV4l2Driver->enumFormat(&fmtdesc)) {
        driverSupportedFmts[fmtdesc.index] = fmtdesc.pixelformat;
        if (fmtdesc.pixelformat == mPixelFmt) {
            return false;
        }
        fmtdesc.index++;
    }
    /* check bitdepth change */
    isCompressedFmt = isCompressedColorFmt(mPixelFmt);
    for (int i = 0; i < fmtdesc.index; i++) {
        if ((isCompressedFmt && isCompressedColorFmt(driverSupportedFmts[i])) ||
            (!isCompressedFmt && isLinearColorFmt(driverSupportedFmts[i]))) {
            found = true;
            mPixelFmt = driverSupportedFmts[i];
            break;
        }
    }
    if (!found) {
        LOG("client colorformat %#x not supported\n", mPixelFmt);
    }

    return true;
}

int V4l2Decoder::detectResolutionChange(bool* hasResolutionChanged) {
    struct v4l2_format fmt;
    int width, height, ret = 0;

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = INPUT_MPLANE;
    ret = mV4l2Driver->getFormat(&fmt);
    if (ret) {
        return ret;
    }

    width = fmt.fmt.pix_mp.width;
    height = fmt.fmt.pix_mp.height;
    if (mWidth != width || mHeight != height) {
        LOG("%s: Update bitstream resolution to wxh %dx%d from %dx%d\n",
            __func__, width, height, mWidth, mHeight);
        mWidth = width;
        mHeight = height;
        *hasResolutionChanged = true;
    }
    return 0;
}

int V4l2Decoder::reconfigureOutput() {
    int ret = 0;
    int latestOutputSize;
    int latestOutputMinCount;
    struct v4l2_format fmt;
    struct v4l2_control ctrl;
    bool isBitDepthChanged = false;
    bool hasResolutionChanged = false;

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = OUTPUT_MPLANE;
    ret = mV4l2Driver->getFormat(&fmt);
    if (ret) {
        return ret;
    }
    latestOutputSize = fmt.fmt.pix_mp.plane_fmt[0].sizeimage;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_MIN_BUFFERS_FOR_CAPTURE;
    ret = mV4l2Driver->getControl(&ctrl);
    if (ret) {
        return ret;
    }
    latestOutputMinCount = ctrl.value;

    isBitDepthChanged = detectBitDepthChange();

    ret = detectResolutionChange(&hasResolutionChanged);
    if (ret) {
        return ret;
    }

    LOG("reconfigureOutput: curent min cnt %d, latest min cnt %d\n",
        mMinOutputCount, latestOutputMinCount);
    LOG("reconfigureOutput: current o/p buffersize %d, latest output size %d, "
        "\n",
        mOutputSize, latestOutputSize);

    if (latestOutputMinCount <= mMinOutputCount && latestOutputSize <= mOutputSize &&
        !isBitDepthChanged && !hasResolutionChanged) {
        ret = start();
        if (ret) {
            return ret;
        }
    } else {
        ret = stopOutput();
        if (ret) {
            return ret;
        }
        ret = configureOutput();
        if (ret) {
            return ret;
        }
        freeBuffers(OUTPUT_PORT);

        ret = allocateBuffers(OUTPUT_PORT);
        if (ret) {
            return ret;
        }
        ret = startOutput();
        if (ret) {
            return ret;
        }
    }
    return 0;
}

int V4l2Decoder::feedInputDataToV4l2Buffer(std::shared_ptr<v4l2_buffer> buf,
                                           bool& eos, uint32_t frameCount) {
    auto itr = mInputDMABuffersPool.find(buf->index);
    if (itr == mInputDMABuffersPool.end()) {
        LOG("Error: no DMA buffer found for buffer index: %d\n", buf->index);
        return -EINVAL;
    }
    auto& dmaBuf = (*itr).second;

    MapBuf map(NULL, dmaBuf->mSize, PROT_READ | PROT_WRITE, MAP_SHARED, dmaBuf->mFd, 0);
    if (!map.isMapSucess()) {
        LOG("Error: failed to mmap output buffer\n");
        return -EINVAL;
    }
    void* bufAddr = map.getMappedAddr();
    // LOG("%d Mapped input buffer ptr: %p\n", buf->index, bufAddr);

    int pktSize = mStreamParser->fillPacketData(bufAddr, eos);
    buf->m.planes[0].bytesused = pktSize;
    buf->m.planes[0].data_offset = 0;
    buf->m.planes[0].length = getInputSize();
    buf->m.planes[0].m.fd = dmaBuf->mFd;
    // LOG("Filled pkg size: %d, length: %d, fd: %d\n", pktSize,
    // buf->m.planes[0].length, buf->m.planes[0].m.fd);
    if (mInputDumpFile != nullptr && pktSize) {
        fwrite(bufAddr, pktSize, 1, mInputDumpFile);
        fflush(mInputDumpFile);
    }
    return 0;
}

int V4l2Decoder::seekToFrame(int frame) {
    return mStreamParser->seekToFrame(frame);
}

int V4l2Decoder::randomSeek() {
    return mStreamParser->randomSeek();
}

int V4l2Decoder::start() {
    int ret;

    struct v4l2_decoder_cmd decCmd;
    memset(&decCmd, 0, sizeof(decCmd));
    decCmd.cmd = V4L2_DEC_CMD_START;
    ret = mV4l2Driver->decCommand(&decCmd);
    if (ret) {
        return ret;
    }
    return 0;
}

int V4l2Decoder::stop() {
    struct v4l2_decoder_cmd decCmd;
    int ret = 0;

    memset(&decCmd, 0, sizeof(decCmd));
    decCmd.cmd = V4L2_DEC_CMD_STOP;
    ret = mV4l2Driver->decCommand(&decCmd);
    if (ret) {
        return ret;
    }
    return 0;
}

int V4l2Decoder::pause() {
    mV4l2Driver->pausePollThread();
    return 0;
}

int V4l2Decoder::resume() {
    mV4l2Driver->resumePollThread();
    return 0;
}

int V4l2Decoder::handleSeek(int seekTo) {
    int ret = stopInput();
    if (ret) {
        LOG("Error: handle_seek stopInput failed\n");
        return ret;
    }
    ret = startInput();
    if (ret) {
        LOG("Error: handle_seek startInput failed\n");
        return ret;
    }
    ret = seekToFrame(seekTo);
    if (ret) {
        LOG("Error: handle_seek StreamParser seekto failed\n");
        return ret;
    }
    return 0;
}

int V4l2Decoder::handleRandomSeek(int& seekPos) {
    int ret = stopInput();
    if (ret) {
        LOG("Error: handle_seek stopInput failed\n");
        return ret;
    }
    ret = startInput();
    if (ret) {
        LOG("Error: handle_seek startInput failed\n");
        return ret;
    }
    seekPos = randomSeek();
    LOG("Random seek to %d\n", seekPos);

    return 0;
}

int V4l2Decoder::queueBuffers(int maxFrameCnt) {
    int ret = 0;
    int frameCounter = 0;
    int seekFrom = mIDRSeek.size() == 0 ? -1 : mIDRSeek.begin()->first;
    int seekTo = mIDRSeek.size() == 0 ? -1 : mIDRSeek.begin()->second;
    int randomSeekFrom =
        mRandomSeek.size() == 0 ? -1 : mRandomSeek.begin()->first;
    int randomSeekTo =
        mRandomSeek.size() == 0 ? -1 : mRandomSeek.begin()->second;

    auto handleDrainEvent = [&]() -> int {
        int ret = 0;
        LOG("queueBuffers: draining pending\n");
        ret = stop();
        if (ret) {
            LOG("Error: queueBuffers: draining failed\n");
            return ret;
        }
        return ret;
    };
    auto isInputAvailable = [&]() -> bool { return !mInputBufs.empty(); };
    auto isOutputAvailable = [&]() -> bool { return !mOutputBufs.empty(); };
    auto waitForCondition = [&](int sleepMs, int maxRetry, auto condition) -> int {
        int retry = 0;
        int midtry = maxRetry / 10 + 1;
        do {
            if (condition()) {
                return 0;
            }
            usleep(sleepMs * 1000);
            if ((retry % midtry) == 0) {
                LOG("Waiting:%d tries\n", retry);
            }
        } while (++retry < maxRetry);

        return -EINVAL;
    };
    auto configureAndStartOutput = [&]() -> int {
        int ret = 0;
        ret = configureOutput();
        if (ret) {
            return ret;
        }
        ret = allocateBuffers(OUTPUT_PORT);
        if (ret) {
            return ret;
        }
        ret = startOutput();
        if (ret) {
            return ret;
        }
        return 0;
    };
    auto getInputBuffer = [&]() -> std::shared_ptr<v4l2_buffer> {
        std::unique_lock<std::mutex> lock(mBufLock);
        auto buf = mInputBufs.front();
        mInputBufs.pop_front();
        mPendingInputBufs.push_back(buf);
        return buf;
    };
    auto getOutputBufferLocked = [&]() -> std::shared_ptr<v4l2_buffer> {
        auto buf = mOutputBufs.front();
        mOutputBufs.pop_front();
        mPendingOutputBufs.push_back(buf);
        return buf;
    };
    auto isEndReached = [&maxFrameCnt](bool eos, int frameNum) -> bool {
        return (eos || frameNum >= maxFrameCnt);
    };
    auto queueAvailableOutputBuffers = [&]() -> int {
        std::shared_ptr<v4l2_buffer> output = nullptr;
        int ret = 0;
        std::unique_lock<std::mutex> lock(mBufLock);

        if (mPendingOutputBufs.size() >= getMinOutputCount()) {
            return 0;
        }

        while (isOutputAvailable()) {
            output = getOutputBufferLocked();
            ret = setOutputBufferData(output);
            if (ret) {
                return ret;
            }
            usleep(1 * 1000);
            ret = queueBuffer(output);
            if (ret) {
                LOG("Error: %s: output failed\n", __func__);
                return ret;
            }
        }
        return ret;
    };
    auto prepareAndQueueInputBuffer = [&]() -> int {
        std::shared_ptr<v4l2_buffer> input = nullptr;
        bool eosReached = false;

        input = getInputBuffer();
        ret = feedInputDataToV4l2Buffer(input, eosReached, frameCounter);
        if (ret) {
            LOG("Error: feed input data failed.\n");
            return ret;
        }
        if (!isEndReached(eosReached, frameCounter)) {
            if (!isOutputPortStarted()) {
                usleep(1 * 1000);
            }
            ret = queueBuffer(input);
            if (ret) {
                LOG("Error: queueBuffer input failed.\n");
                return ret;
            }
        } else {
            if (isOutputPortStarted()) {
                setDrainSent(true);
                setDrainPending(false);
                ret = handleDrainEvent();
                if (ret) {
                    return ret;
                }
            } else {
                LOG("%s: postpone drain. output not started\n", __func__);
                setDrainPending(true);
            }
        }
        return ret;
    };
    auto handleReconfigEvent = [&]() -> int {
        int ret = 0;
        if (!isOutputPortStarted()) {
            LOG("%s: 1st src change event arrived. start output\n", __func__);
            setReconfigEventReceived(false);
            ret = configureAndStartOutput();
            if (ret) {
                return ret;
            }
        } else if (isDrcLastFlagReceived()) {
            setReconfigEventReceived(false);
            setDrcLastFlagReceived(false);
            ret = reconfigureOutput();
            if (ret) {
                LOG("Error: queueBuffers: reconfigureOutput failed.\n");
                return ret;
            }
            LOG("%s: last flag for reconfig arrived\n", __func__);
        }
        return ret;
    };
    auto handleDrainLastEvent = [&]() -> int {
        int ret = 0;
        setDrainLastFlagReceived(false);
        setDrainSent(false);
        LOG("queueBuffers: last flag for drain arrived\n");
        ret = start();
        if (ret != 0) {
            LOG("Error: queueBuffers: resume failed.\n");
            return ret;
        }
        return ret;
    };
    auto handleRandomSeekIfNeeded = [&]() -> int {
        int ret;
        if (frameCounter != randomSeekFrom) {
            return 0;
        }
        ret = waitForCondition(100, 100, [&]() -> bool { return isFirstReconfigReceived(); });
        if (ret) {
            return ret;
        }
        ret = handleRandomSeek(randomSeekTo);
        if (ret < 0) {
            LOG("Error: failed to handle random seek. Exit.");
            return ret;
        }
        frameCounter = randomSeekTo;

        if (mRandomSeek.size()) {
            mRandomSeek.erase(mRandomSeek.begin());
            randomSeekFrom =
                mRandomSeek.size() == 0 ? -1 : mRandomSeek.begin()->first;
            randomSeekTo =
                mRandomSeek.size() == 0 ? -1 : mRandomSeek.begin()->second;
        } else {
            randomSeekFrom = -1;
            randomSeekTo = -1;
        }

        return ret;
    };

    auto handleSeekIfNeeded = [&]() -> int {
        int ret = 0;
        if (!mWillSeek) {
            return 0;
        }
        if (frameCounter != seekFrom) {
            return 0;
        }
        ret = waitForCondition(10, 5, [&]() -> bool { return isFirstReconfigReceived(); });
        if (ret) {
            return ret;
        }
        handleSeek(seekTo);
        frameCounter = seekTo;

        LOG("queueBuffers: seek from %d to %d completed.\n", seekFrom, seekTo);

        if (mIDRSeek.size()) {
            mIDRSeek.erase(mIDRSeek.begin());
            seekFrom = mIDRSeek.size() == 0 ? -1 : mIDRSeek.begin()->first;
            seekTo = mIDRSeek.size() == 0 ? -1 : mIDRSeek.begin()->second;
        } else {
            seekFrom = -1;
            seekTo = -1;
        }

        LOG("queueBuffers: UPDATED: will seek from %d to %d.\n", seekFrom,
            seekTo);
        return 0;
    };

    while (mErrorReceived == false) {
        // Handle Dynamic Commands
        ret = setDynamicCommands(frameCounter);
        if (ret) {
            return ret;
        }

        ret = handleRandomSeekIfNeeded();
        if (ret) {
            return ret;
        }

        /* handle seek scenario */
        ret = handleSeekIfNeeded();
        if (ret) {
            return ret;
        }

        if (isReconfigEventReceived()) {
            ret = handleReconfigEvent();
            if (ret) {
                return ret;
            }
        }

        if (isOutputPortStarted()) {
            ret = queueAvailableOutputBuffers();
            if (ret) {
                return ret;
            }
            if (isDrainPending()) {
                setDrainPending(false);
                setDrainSent(true);
                ret = handleDrainEvent();
                if (ret) {
                    return ret;
                }
            }
        }

        if (isDrainSent()) {
            if (!isDrainLastFlagReceived()) {
                usleep(10 * 1000);
                continue;
            }
            ret = handleDrainLastEvent();
            if (ret) {
                return ret;
            }
            break;
        }

        ret = waitForCondition(100, 1000,
                               [&]() -> bool { return isInputAvailable(); });
        if (ret) {
            return ret;
        }

        ret = prepareAndQueueInputBuffer();
        if (ret) {
            return ret;
        }

        LOG("frame count: %d\n", frameCounter);
        frameCounter++;
    }

    return ret;
}

int V4l2Decoder::writeDumpDataToFile(v4l2_buffer* buffer) {
    // Writing one color plane.
    auto writePlane = [=](const uint8_t* p, uint32_t wBytes, uint32_t strideBytes,
                          uint32_t nLines) {
        for (uint32_t i = 0; i < nLines; ++i) {
            fwrite(p, wBytes, 1, mOutputDumpFile);
            fflush(mOutputDumpFile);
            p += strideBytes;
        }
    };
    std::uint8_t* pBuffer = nullptr;
    MapBuf map(NULL, buffer->m.planes[0].length, PROT_READ, MAP_SHARED, buffer->m.planes[0].m.fd,
               0);
    if (!map.isMapSucess()) {
        LOG("Error: failed to mmap output buffer\n");
        return -EINVAL;
    }
    pBuffer = (std::uint8_t*)map.getMappedAddr();
    int frameWidth = getFrameWidth(), frameHeight = getFrameHeight();
    int oBufWidth = getOutputBufferWidth(), oBufHeight = getOubputBufferHeight();
    switch (getColorFormat()) {
        case V4L2_PIX_FMT_NV12: {
            uint8_t* base = pBuffer;
            LOG("Dump file as NV12, frame size(%dx%d), buffer size(%dx%d).\n",
                frameWidth, frameHeight, oBufWidth, oBufHeight);
            if (frameWidth == oBufWidth) {
                // Y Plane
                fwrite(base, frameWidth * frameHeight, 1, mOutputDumpFile);
                // UV Plane
                base += oBufWidth * oBufHeight;
                fwrite(base, frameWidth * frameHeight / 2, 1, mOutputDumpFile);
            } else {
                // Y Plane
                writePlane(base, frameWidth, oBufWidth, frameHeight);
                // UV Plane
                base += oBufWidth * oBufHeight;
                writePlane(base, frameWidth, oBufWidth, frameHeight / 2);
            }
            break;
        }
        case V4L2_PIX_FMT_QC08C:
        case V4L2_PIX_FMT_QC10C: {
            fwrite(pBuffer, buffer->m.planes[0].bytesused, 1, mOutputDumpFile);
            break;
        }
        default: {
            LOG("unsupport this color format: %x\n", getColorFormat());
            fwrite(pBuffer, buffer->m.planes[0].bytesused, 1, mOutputDumpFile);
            break;
        }
    }

    return 0;
}

V4l2DecoderCB::V4l2DecoderCB(V4l2Decoder* dec, std::string sessionId)
    : V4l2CodecCallback(sessionId), mDec(dec) {}

int V4l2DecoderCB::onBufferDone(v4l2_buffer* buffer) {
    std::unique_lock<std::mutex> lock(mDec->mBufLock);
    int ret = 0;
    auto putInputBufferLocked = [&](v4l2_buffer* buf) -> int {
        auto& inputBufs = mDec->mInputBufs;
        auto& pendingInputBufs = mDec->mPendingInputBufs;

        auto itr = std::find_if(pendingInputBufs.begin(), pendingInputBufs.end(),
                                [&buf](auto in) -> bool { return in->index == buf->index; });
        if (itr == pendingInputBufs.end()) {
            return -EINVAL;
        }

        inputBufs.push_back(*itr);
        pendingInputBufs.remove(*itr);
        return 0;
    };
    auto putOutputBufferLocked = [&](v4l2_buffer* buf) -> int {
        auto& outputBufs = mDec->mOutputBufs;
        auto& pendingOutputBufs = mDec->mPendingOutputBufs;

        auto itr = std::find_if(pendingOutputBufs.begin(), pendingOutputBufs.end(),
                                [&buf](auto in) -> bool { return in->index == buf->index; });
        if (itr == pendingOutputBufs.end()) {
            return -EINVAL;
        }

        outputBufs.push_back(*itr);
        pendingOutputBufs.remove(*itr);
        return 0;
    };

    // LOG("V4l2DecoderCB::onBufferDone()\n");
    if (buffer->type == INPUT_MPLANE) {
        ret = putInputBufferLocked(buffer);
        if (ret) {
            return ret;
        }
    } else if (buffer->type == OUTPUT_MPLANE) {
        LOG("%s: DQBUF DONE(output): %d, bytesused: %d\n", __func__,
            buffer->index, buffer->m.planes[0].bytesused);
        ret = putOutputBufferLocked(buffer);
        if (ret) {
            return ret;
        }

        if (mDec->mOutputDumpFile && buffer->m.planes[0].bytesused) {
            mDec->writeDumpDataToFile(buffer);
        }

        if (buffer->flags & V4L2_BUF_FLAG_LAST) {
            buffer->flags &= ~V4L2_BUF_FLAG_LAST;

            if (mDec->isReconfigEventReceived()) {
                LOG("onBufferDone: Drc last flag received\n");
                mDec->setDrcLastFlagReceived(true);
            }

            if (mDec->isDrainSent()) {
                LOG("onBufferDone: drain last flag received\n");
                mDec->setDrainLastFlagReceived(true);
            }
        }
    }

    return 0;
}

int V4l2DecoderCB::onEventDone(v4l2_event* event) {
    std::unique_lock<std::mutex> lock(mDec->mBufLock);
    LOG("V4l2DecoderCB::onEventDone()\n");
    if (event == nullptr) {
        LOG("V4l2DecoderCB::onEventDone: error, null event!\n");
        return -EINVAL;
    } else if (event->type == V4L2_EVENT_SOURCE_CHANGE &&
               event->u.src_change.changes == V4L2_EVENT_SRC_CH_RESOLUTION) {
        LOG("onEventDone : source change event received\n");
        mDec->setReconfigEventReceived(true);
        mDec->setFirstReconfigReceived(true);
    }
    return 0;
}

int V4l2DecoderCB::onError(int error) {
    LOG("onError called\n");
    mDec->mErrorReceived = true;
    return 0;
}
