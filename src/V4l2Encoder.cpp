/*
 **************************************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 **************************************************************************************************
*/

#include <unistd.h>
#include <algorithm>
#include <sys/ioctl.h>
#include <linux/dma-buf.h>

#include "FFYUVParser.h"
#include "V4l2Encoder.h"

#define ALIGN(num, to) (((num) + (to - 1)) & (~(to - 1)))
#define CONVERT_TO_LITTLE_ENDIAN(x)                                           \
    (((x)&0x000000FF) << 24 | ((x)&0x0000FF00) << 8 | ((x)&0x00FF0000) >> 8 | \
     ((x)&0XFF000000) >> 24)

V4l2Encoder::V4l2Encoder(unsigned int codec, unsigned int pixel,
                         std::string sessionId)
    : V4l2Codec(codec, pixel, sessionId) {
#if DUMP_BUF_DATA
    gDumpV4L2BufferDataFile = fopen("./DumpV4l2BufferData.txt", "w");
    if (gDumpV4L2BufferDataFile == nullptr) {
        printf("Error: failed to open DumpV4L2BufferDataFile.\n");
    }
#endif
}

V4l2Encoder::~V4l2Encoder() {
#if DUMP_BUF_DATA
    if (gDumpV4L2BufferDataFile) {
        fclose(gDumpV4L2BufferDataFile);
        gDumpV4L2BufferDataFile = nullptr;
    }
#endif
}

int V4l2Encoder::init() {
    struct v4l2_control ctrl;
    struct v4l2_selection sel;
    struct v4l2_capability caps;
    int ret = 0;
    mDomain = V4L2_CODEC_TYPE_ENCODER;

    ret = mV4l2Driver->Open(mDomain);
    if (ret) {
        return ret;
    }
    ret = mV4l2Driver->setCodecPixelFmt(OUTPUT_MPLANE, mCodecFmt);
    if (ret) {
        return ret;
    }
    ret = mV4l2Driver->setCodecPixelFmt(INPUT_MPLANE, mPixelFmt);
    if (ret) {
        return ret;
    }
    ret = mV4l2Driver->OpenDMAHeap("system");
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

void V4l2Encoder::deinit() {
    mV4l2Driver->stopPollThread();
    mV4l2Driver->Close();
    mV4l2Driver->CloseDMAHeap();
}

int V4l2Encoder::initFFYUVParser(std::string inputPath, int width, int height, std::string pixfmt) {
    std::string resolution = std::to_string(width) + "x" + std::to_string(height);
    mYUVParser = std::make_shared<FFYUVParser>(inputPath, resolution, pixfmt,
                                               mSessionId);
    if (mYUVParser == nullptr) {
        return -1;
    }
    if (mYUVParser->init() < 0) {
        return -1;
    }
    return 0;
}

void V4l2Encoder::deinitFFYUVParser() {
    mYUVParser->deinit();
}

static int calc_scanline_aligned(int height, int stride, int imageSize, int colorFormat) {
    int scanline = 0;

    switch (colorFormat) {
        case V4L2_PIX_FMT_QC08C:
        case V4L2_PIX_FMT_NV12:
            scanline = ALIGN(height, 32);
            break;
        case V4L2_PIX_FMT_QC10C:
            scanline = ALIGN(height, 16);
            break;
        default:
            scanline = height;
            break;
    }
    return scanline;
}

int V4l2Encoder::queryControlsAVC(uint32_t level) {
    int ret = 0;
    bool found = false;

    struct v4l2_queryctrl queryctrl;
    memset(&queryctrl, 0, sizeof(queryctrl));

    queryctrl.id = V4L2_CID_MPEG_VIDEO_H264_LEVEL;

    ret = mV4l2Driver->queryControl(&queryctrl);
    if (ret != 0) {
        return -1;
    }

    for (auto i = queryctrl.minimum; i <= queryctrl.maximum; i++) {
        struct v4l2_querymenu querymenu;
        memset(&querymenu, 0, sizeof(querymenu));

        querymenu.id = V4L2_CID_MPEG_VIDEO_H264_LEVEL;
        querymenu.index = i;

        ret = mV4l2Driver->queryMenu(&querymenu);
        if (!ret) {
            continue;
        }

        if (level == i) {
            return 0;
        }
    }

    return -1;
}

int V4l2Encoder::queryControlsHEVC(uint32_t level, uint32_t tier) {
    int ret = 0;
    bool levFound = false, tierFound = false;

    struct v4l2_queryctrl queryCtrlTier;
    memset(&queryCtrlTier, 0, sizeof(queryCtrlTier));

    queryCtrlTier.id = V4L2_CID_MPEG_VIDEO_HEVC_TIER;

    ret = mV4l2Driver->queryControl(&queryCtrlTier);
    if (ret != 0) {
        return -1;
    }

    struct v4l2_queryctrl queryCtrlLevel;
    memset(&queryCtrlLevel, 0, sizeof(queryCtrlLevel));

    queryCtrlLevel.id = V4L2_CID_MPEG_VIDEO_HEVC_LEVEL;

    ret = mV4l2Driver->queryControl(&queryCtrlLevel);
    if (ret != 0) {
        return -1;
    }

    for (auto i = queryCtrlTier.minimum; i <= queryCtrlTier.maximum; i++) {
        struct v4l2_querymenu queryMenuTier;

        queryMenuTier.id = V4L2_CID_MPEG_VIDEO_HEVC_TIER;
        queryMenuTier.index = i;

        ret = mV4l2Driver->queryMenu(&queryMenuTier);
        if (ret != 0) {
            continue;
        }

        if (i == tier) {
            tierFound = true;
        }

        for (auto j = queryCtrlLevel.minimum; j <= queryCtrlLevel.maximum;
             j++) {
            struct v4l2_querymenu queryMenuLevel;

            queryMenuLevel.id = V4L2_CID_MPEG_VIDEO_HEVC_LEVEL;
            queryMenuLevel.index = j;

            ret = mV4l2Driver->queryMenu(&queryMenuLevel);
            if (ret != 0) {
                continue;
            }

            if (j == level) {
                levFound = true;
            }
        }
    }

    if (levFound && tierFound) {
        return 0;
    }

    return -1;
}

int V4l2Encoder::configureInput() {
    LOGV("V4l2Encoder::configureInput().\n");
    struct v4l2_format fmt;
    struct v4l2_frmsizeenum fsize;
    struct v4l2_frmivalenum fival;
    struct v4l2_requestbuffers reqBufs;
    struct v4l2_control ctrl;
    struct v4l2_selection sel;
    int ret = 0;

    if (mStride < mWidth || mScanline < mHeight) {
        mStride = mWidth;
        mScanline = mHeight;
    }

    /* query driver recommended framesizes */
    memset(&fsize, 0, sizeof(fsize));
    fsize.index = 0;
    fsize.pixel_format = mPixelFmt;
    ret = mV4l2Driver->enumFramesize(&fsize);
    if (ret) {
        return ret;
    }

    /* query driver recommended frameintervals */
    memset(&fival, 0, sizeof(fival));
    fival.index = 0;
    fival.pixel_format = mPixelFmt;
    fival.width = mWidth;
    fival.height = mHeight;
    ret = mV4l2Driver->enumFrameInterval(&fival);
    if (ret) {
        return ret;
    }

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = INPUT_MPLANE;
    ret = mV4l2Driver->getFormat(&fmt);
    if (ret) {
        return ret;
    }
    fmt.fmt.pix_mp.width = mWidth;
    fmt.fmt.pix_mp.height = mHeight;
    fmt.fmt.pix_mp.colorspace = mInputColorPrimaries;
    fmt.fmt.pix_mp.ycbcr_enc = mInputMatrixCoeff;
    fmt.fmt.pix_mp.xfer_func = mInputTransferChar;
    fmt.fmt.pix_mp.quantization = mInputVideoRange;
    ret = mV4l2Driver->setFormat(&fmt);
    if (ret) {
        return ret;
    }
    mInputColorPrimaries = fmt.fmt.pix_mp.colorspace;
    mInputMatrixCoeff = fmt.fmt.pix_mp.ycbcr_enc;
    mInputTransferChar = fmt.fmt.pix_mp.xfer_func;
    mInputVideoRange = fmt.fmt.pix_mp.quantization;
    mInputSize = fmt.fmt.pix_mp.plane_fmt[0].sizeimage;
    mStride = fmt.fmt.pix_mp.plane_fmt[0].bytesperline;
    mScanline = calc_scanline_aligned(mHeight, mStride, mInputSize, mPixelFmt);
    LOGV("%s: WxH(%dx%d), stride(%d), scanline(%d), inputSize(%d)\n", __func__,
        mWidth, mHeight, mStride, mScanline, mInputSize);

    if (mStride > mWidth || mScanline > mHeight) {
        memset(&sel, 0, sizeof(sel));
        sel.type = INPUT_MPLANE;
        sel.target = V4L2_SEL_TGT_CROP;
        sel.r.width = mWidth;
        sel.r.height = mHeight;
        if (mV4l2Driver->setSelection(&sel)) {
            return -EINVAL;
        }
        mCropLeft = sel.r.left;
        mCropTop = sel.r.top;
        mCropWidth = sel.r.width;
        mCropHeight = sel.r.height;
    } else {
        mCropLeft = 0;
        mCropTop = 0;
        mCropWidth = mWidth;
        mCropHeight = mHeight;
    }

    ctrl.id = V4L2_CID_MIN_BUFFERS_FOR_OUTPUT;
    ret = mV4l2Driver->getControl(&ctrl);
    if (ret) {
        return ret;
    }
    mMinInputCount = ctrl.value;

    if (mActualInputCount < mMinInputCount) {
        LOGV("update input count from %d to %d\n", mActualInputCount,
            mMinInputCount);
        mActualInputCount = mMinInputCount;
    }

    memset(&reqBufs, 0, sizeof(reqBufs));
    reqBufs.type = INPUT_MPLANE;
    reqBufs.memory = V4L2_MEMORY_DMABUF;
    reqBufs.count = mActualInputCount;
    ret = mV4l2Driver->reqBufs(&reqBufs);
    if (ret) {
        return ret;
    }
    mActualInputCount = reqBufs.count;
    LOGI("%s: %d input buffers got from reqBufs\n", __func__, mActualInputCount);

    return 0;
}

int V4l2Encoder::configureOutput() {
    LOGV("V4l2Encoder::configureOutput().");
    struct v4l2_format fmt;
    struct v4l2_requestbuffers reqBufs;
    struct v4l2_control ctrl;
    int ret = 0;

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = OUTPUT_MPLANE;
    ret = mV4l2Driver->getFormat(&fmt);
    if (ret) {
        return ret;
    }
    mOutputSize = fmt.fmt.pix_mp.plane_fmt[0].sizeimage;

    ctrl.id = V4L2_CID_MIN_BUFFERS_FOR_CAPTURE;
    ret = mV4l2Driver->getControl(&ctrl);
    if (ret) {
        return ret;
    }
    mMinOutputCount = ctrl.value;

    if (mActualOutputCount < mMinOutputCount) {
        LOGV("update output count from %d to %d\n", mActualOutputCount,
            mMinOutputCount);
        mActualOutputCount = mMinOutputCount;
    }
    LOGV("%s: outputsize: %d, actualoutputcount:%d\n", __func__, mOutputSize,
        mActualOutputCount);
    // LOG("configureOutput: WxH(%dx%d), fmt(%x)\n", mWidth, mHeight, mCodec);

    memset(&reqBufs, 0, sizeof(reqBufs));
    reqBufs.type = OUTPUT_MPLANE;
    reqBufs.memory = V4L2_MEMORY_DMABUF;
    reqBufs.count = mActualOutputCount;
    ret = mV4l2Driver->reqBufs(&reqBufs);
    if (ret) {
        return ret;
    }
    mActualOutputCount = reqBufs.count;
    LOGI("%s: %d output buffers got from reqBufs\n", __func__,
        mActualOutputCount);
    return 0;
}

int V4l2Encoder::feedInputDataToV4l2Buffer(std::shared_ptr<v4l2_buffer> buf,
                                           bool& eos, uint32_t frameCount) {
    struct dma_buf_sync sync;
    int ret = 0;
    auto itr = mInputDMABuffersPool.find(buf->index);
    if (itr == mInputDMABuffersPool.end()) {
        LOGE("Error: no DMA buffer found for buffer index: %d\n", buf->index);
        return -EINVAL;
    }
    auto& dmaBuf = (*itr).second;

    MapBuf map(NULL, dmaBuf->mSize, PROT_READ | PROT_WRITE, MAP_SHARED, dmaBuf->mFd, 0);
    if (!map.isMapSucess()) {
        LOGE("Error: failed to mmap output buffer\n");
        return -EINVAL;
    }
    void* bufAddr = map.getMappedAddr();

    // LOG("%d Mapped input buffer ptr: %p\n", buf->index, bufAddr);
    int frmWidth = getFrameWidth(), frmHeight = getFrameHeight();
    int frmStride = getFrameStride(), frmScanline = getFrameScanline();

    memset(bufAddr, 0, getInputSize());

    sync.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_WRITE;
    ret = ioctl(dmaBuf->mFd, DMA_BUF_IOCTL_SYNC, &sync);
    if (ret) {
        LOGD("input read DMA_BUF_SYNC_START failed with err = %d\n", ret);
    }
    int pkt_size = mYUVParser->fillPacketData(bufAddr, frmWidth, frmHeight, frmStride, frmScanline,
                                              mPixelFmt, eos);
    sync.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_WRITE;
    ret = ioctl(dmaBuf->mFd, DMA_BUF_IOCTL_SYNC, &sync);
    if (ret) {
        LOGD("input read DMA_BUF_SYNC_END failed with err = %d\n", ret);
    }

    buf->m.planes[0].bytesused = pkt_size;
    buf->m.planes[0].data_offset = 0;
    buf->m.planes[0].length = getInputSize();
    buf->m.planes[0].m.fd = dmaBuf->mFd;

    auto timePerFrame =  (float)(1000000.0 / (1.0 * mFrameRate));
    buf->timestamp.tv_sec = frameCount * (long)(timePerFrame / 1000000);
    buf->timestamp.tv_usec = frameCount * ((long)timePerFrame % 1000000);

    // LOG("Filled pkg size: %d, length: %d, fd: %d\n", pkt_size,
    // buf->m.planes[0].length, buf->m.planes[0].m.fd); int bufferSz = frmStride
    // * frmScanline + frmStride * ALIGN((frmHeight + 1) >> 1, 16);
    if (mInputDumpFile != nullptr && pkt_size) {
        fwrite(bufAddr, pkt_size, 1, mInputDumpFile);
        fflush(mInputDumpFile);
    }

    return ret;
}

int V4l2Encoder::stop() {
    struct v4l2_encoder_cmd encCmd;
    int ret = 0;

    memset(&encCmd, 0, sizeof(encCmd));
    encCmd.cmd = V4L2_ENC_CMD_STOP;
    ret = mV4l2Driver->encCommand(&encCmd);
    if (ret) {
        return ret;
    }
    return 0;
}

int V4l2Encoder::start() {
    struct v4l2_encoder_cmd encCmd;
    memset(&encCmd, 0, sizeof(encCmd));
    encCmd.cmd = V4L2_ENC_CMD_START;
    int ret = mV4l2Driver->encCommand(&encCmd);
    if (ret) {
        return ret;
    }
    return 0;
}

int V4l2Encoder::setOperatingRate(unsigned int numer, unsigned int denom) {
    struct v4l2_streamparm sParm;
    int ret = 0;

    memset(&sParm, 0, sizeof(sParm));
    sParm.type = INPUT_MPLANE;
    sParm.parm.output.timeperframe.numerator = numer;
    sParm.parm.output.timeperframe.denominator = denom;
    ret = mV4l2Driver->setParm(&sParm);
    if (ret) {
        return ret;
    }
    return 0;
}

int V4l2Encoder::setFrameRate(unsigned int numer, unsigned int denom) {
    if (denom == 0) {
        return -1;
    }
    mFrameRate = (long)denom / (long)numer;

    struct v4l2_streamparm sParm;
    memset(&sParm, 0, sizeof(sParm));

    sParm.type = OUTPUT_MPLANE;
    sParm.parm.capture.timeperframe.numerator = numer;
    sParm.parm.capture.timeperframe.denominator = denom;
    int ret = mV4l2Driver->setParm(&sParm);
    if (ret) {
        return ret;
    }
    return 0;
}

int V4l2Encoder::queueBuffers(int maxFrameCnt) {
    int ret = 0, input_try = 0;
    unsigned int frameCounter = 0, LTRIdx = 0;
    auto handleDrainLastEvent = [&]() -> int {
        int ret = 0;
        setDrainLastFlagReceived(false);
        setDrainSent(false);
        LOGW("queueBuffers: last flag for drain arrived\n");
        ret = start();
        if (ret != 0) {
            LOGE("Error: queueBuffers: resume failed.\n");
            return ret;
        }
        return ret;
    };
    auto handleDrainEvent = [&]() -> int {
        int ret = 0;
        LOGW("queueBuffers: draining pending\n");
        ret = stop();
        if (ret) {
            LOGE("Error: queueBuffers: draining failed\n");
            return ret;
        }
        return ret;
    };

    auto isInputAvailable = [&]() -> bool {
        std::unique_lock<std::mutex> lock(mBufLock);
        return !mInputBufs.empty();
    };

    auto needWaitForInput = [&]() -> bool {
        std::unique_lock<std::mutex> lock(mBufLock);
        if (mPendingInputBufs.size() >= getMinInputCount()) {
            return true;
        }
        return false;
    };

    auto isOutputAvailable = [&]() -> bool { return !mOutputBufs.empty(); };

    auto isEndReached = [&maxFrameCnt](bool eos, int frameNum) -> bool {
        return (eos || frameNum >= maxFrameCnt);
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
    auto prepareAndQueueInputBuffer = [&]() -> int {
        std::shared_ptr<v4l2_buffer> input = nullptr;
        bool eosReached = false;
        int ret = 0;

        input = getInputBuffer();
        ret = feedInputDataToV4l2Buffer(input, eosReached, frameCounter);
        if (ret) {
            LOGE("Error: feed input data failed.\n");
            return ret;
        }
        if (!isEndReached(eosReached, frameCounter)) {
            usleep(1 * 1000);
            ret = queueBuffer(input);
            if (ret) {
                LOGE("Error: queueBuffer input failed.\n");
                return ret;
            }
        } else {
            setDrainSent(true);
            ret = handleDrainEvent();
            if (ret) {
                return ret;
            }
        }
        return ret;
    };
    auto queueAvailableOutputBuffers = [&]() -> int {
        std::unique_lock<std::mutex> lock(mBufLock);
        std::shared_ptr<v4l2_buffer> output = nullptr;
        int ret = 0;

        while (isOutputAvailable()) {
            output = getOutputBufferLocked();
            ret = setOutputBufferData(output);
            if (ret) {
                LOGE("Error: failed to set output buffer data: %d\n", ret);
                return ret;
            }
            if (!isInputPortStarted()) {
                usleep(1 * 1000);
            }
            ret = queueBuffer(output);
            if (ret) {
                LOGE("Error: %s: output failed\n", __func__);
                return ret;
            }
        }
        return ret;
    };

    while (mErrorReceived == false) {
        if (isOutputPortStarted()) {
            ret = queueAvailableOutputBuffers();
            if (ret) {
                return ret;
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

        // Handle Dynamic Commands
        ret = setDynamicCommands(frameCounter);
        if (ret) {
            return ret;
        }

        // Handle Dynamic Controls
        ret = setDynamicControls(frameCounter);
        if (ret) {
            return ret;
        }

        if (!isInputAvailable()) {
            if (needWaitForInput()) {
                usleep(10 * 1000);
                continue;
            }
            return -ENOMEM;
        }

        ret = prepareAndQueueInputBuffer();
        if (ret) {
            return ret;
        }
        LOGD("%s: %u frames queued.\n", __func__, frameCounter);
        frameCounter++;
    }

    return ret;
}

int V4l2Encoder::replaceNalSizeWAndWrite(std::uint8_t* basePtr, unsigned int filledLen) {
    int ret = 0;
    unsigned int startCode[1] = {0x01000000};
    std::uint8_t* nextNalUnit = basePtr;

    while ((nextNalUnit + sizeof(startCode)) <= (basePtr + filledLen)) {
        unsigned int nalSize;
        fwrite((std::uint8_t*)startCode, 4, 1, mOutputDumpFile);
        nalSize = *(unsigned int*)nextNalUnit;
        nalSize = CONVERT_TO_LITTLE_ENDIAN(nalSize);
        if (nextNalUnit + nalSize > (basePtr + filledLen)) {
            break;
        }
        /* write NAL Unit */
        fwrite(nextNalUnit + sizeof(startCode), nalSize, 1, mOutputDumpFile);
        nextNalUnit += (nalSize + 4);
    }

    if (nextNalUnit != (basePtr + filledLen)) {
        ret = -EINVAL;
    }

    return ret;
}

void V4l2Encoder::logV4l2BufferDataToFile(std::uint8_t* buffer, int buffer_len, int idx) {
#if DUMP_BUF_DATA
    const char* header_line = "**************************************";
    fprintf(gDumpV4L2BufferDataFile, "%s %d Buffer; Size: %d %s\n", header_line,
            idx, buffer_len, header_line);
    for (size_t i = 0; i < buffer_len; i++) {
        fprintf(gDumpV4L2BufferDataFile, "%#3x", buffer[i]);
        if ((i + 1) % 32 == 0) {
            fprintf(gDumpV4L2BufferDataFile, "\n");
        }
    }
    fprintf(gDumpV4L2BufferDataFile, "\n\n");
#endif
}

int V4l2Encoder::writeDumpDataToFile(v4l2_buffer* buffer) {
    struct dma_buf_sync sync;
    std::uint8_t* pBuffer = nullptr;
    int ret = 0;
    MapBuf map(NULL, buffer->m.planes[0].length, PROT_READ, MAP_SHARED, buffer->m.planes[0].m.fd,
               0);

    if (!map.isMapSucess()) {
        LOGE("Error: failed to mmap output buffer\n");
        return -EINVAL;
    }
    pBuffer = (std::uint8_t*)map.getMappedAddr();

    LOGD("Writing %d bytes to output, first 8 bytes: [%x %x %x %x %x %x %x "
        "%x]\n",
        buffer->m.planes[0].bytesused, pBuffer[0], pBuffer[1], pBuffer[2],
        pBuffer[3], pBuffer[4], pBuffer[5], pBuffer[6], pBuffer[7]);
    if (isNALEncodingEnabled()) {
        ret = replaceNalSizeWAndWrite(pBuffer, buffer->m.planes[0].bytesused);
        if (ret != 0) {
            return ret;
        }
        return 0;
    }

    sync.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_READ;
    ret = ioctl(buffer->m.planes[0].m.fd, DMA_BUF_IOCTL_SYNC, &sync);
    if (ret) {
        LOGD("Save encode DMA_BUF_SYNC_START failed with err = %d\n",
            ret);
    }
    fwrite(pBuffer, buffer->m.planes[0].bytesused, 1, mOutputDumpFile);
    logV4l2BufferDataToFile(pBuffer, buffer->m.planes[0].bytesused, mEncodedBufferReceieved);
    sync.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_READ;
    ret = ioctl(buffer->m.planes[0].m.fd, DMA_BUF_IOCTL_SYNC, &sync);
    if (ret) {
        LOGD("Save encode DMA_BUF_SYNC_END failed with err = %d\n", ret);
    }

    return 0;
}

int V4l2EncoderCB::onBufferDone(v4l2_buffer* buffer) {
    std::unique_lock<std::mutex> lock(mEnc->mBufLock);
    int ret = 0;
    auto putInputBufferLocked = [&](v4l2_buffer* buf) -> int {
        auto& inputBufs = mEnc->mInputBufs;
        auto& pendingInputBufs = mEnc->mPendingInputBufs;

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
        auto& outputBufs = mEnc->mOutputBufs;
        auto& pendingOutputBufs = mEnc->mPendingOutputBufs;

        auto itr = std::find_if(pendingOutputBufs.begin(), pendingOutputBufs.end(),
                                [&buf](auto in) -> bool { return in->index == buf->index; });
        if (itr == pendingOutputBufs.end()) {
            return -EINVAL;
        }

        outputBufs.push_back(*itr);
        pendingOutputBufs.remove(*itr);
        return 0;
    };

    if (buffer->type == INPUT_MPLANE) {
        ret = putInputBufferLocked(buffer);
        if (ret) {
            return ret;
        }
    } else if (buffer->type == OUTPUT_MPLANE) {
        LOGD("DQBUF DONE(Output): %d, bytesused: %d\n", buffer->index,
            buffer->m.planes[0].bytesused);
        ret = putOutputBufferLocked(buffer);
        if (ret) {
            return ret;
        }
        if (mEnc->mOutputDumpFile && buffer->m.planes[0].bytesused) {
            mEnc->writeDumpDataToFile(buffer);
        }
        if (buffer->flags & V4L2_BUF_FLAG_LAST) {
            buffer->flags &= ~V4L2_BUF_FLAG_LAST;
            if (mEnc->isDrainSent()) {
                LOGW("onBufferDone: drain last flag received\n");
                mEnc->setDrainLastFlagReceived(true);
            }
        }
        mEnc->mEncodedBufferReceieved++;
    }
    return 0;
}

int V4l2EncoderCB::onError(int error) {
    LOGV("onError called\n");
    mEnc->mErrorReceived = true;
    return 0;
}
