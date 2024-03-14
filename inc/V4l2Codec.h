/*
 **************************************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 **************************************************************************************************
*/

#ifndef _TEST_CODEC_H_
#define _TEST_CODEC_H_

#include <atomic>
#include <cstring>
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "ConfigParser.h"
#include "Log.h"
#include "V4l2Driver.h"

class MapBuf {
  public:
    explicit MapBuf(void* addr, size_t len, int prot, int flags, int fd, __off_t offset)
        : mMapAddr(nullptr), mIsMappingDone(false), mLength(len) {
        mMapAddr = mmap(addr, len, prot, flags, fd, offset);
        if (mMapAddr == MAP_FAILED) {
            return;
        }
        mIsMappingDone = true;
    }
    ~MapBuf() {
        if (mIsMappingDone) {
            munmap(mMapAddr, mLength);
        }
    }
    bool isMapSucess() const { return mIsMappingDone; }
    void* getMappedAddr() const { return mMapAddr; }

  private:
    void* mMapAddr;
    size_t mLength;
    bool mIsMappingDone;
};

struct DMABuffer {
    explicit DMABuffer(uint32_t size, int fd);
    ~DMABuffer();
    uint32_t mSize;
    int mFd;
};

class V4l2CodecCallback {
  public:
    V4l2CodecCallback() = delete;
    explicit V4l2CodecCallback(std::string sessionId) : mSessionId(sessionId) {}
    virtual ~V4l2CodecCallback() = default;
    virtual int onBufferDone(struct v4l2_buffer* buffer) = 0;
    virtual int onEventDone(struct v4l2_event* event) = 0;
    virtual int onError(int error) = 0;
    std::string id() { return mSessionId; }

  private:
    std::string mSessionId;
};

class V4l2Codec {
  public:
    V4l2Codec() = delete;

    explicit V4l2Codec(unsigned int codec, unsigned int pixel,
                       std::string sessionId);

    virtual ~V4l2Codec();

    std::string id();

    virtual int init() = 0;
    virtual void deinit() = 0;

    virtual int stop() = 0;
    virtual int start() = 0;
    virtual int configureInput() = 0;
    virtual int configureOutput() = 0;
    virtual int reconfigureOutput() = 0;
    virtual bool detectBitDepthChange() = 0;
    virtual int queueBuffers(int maxFrameCnt) = 0;
    virtual int queryControlsAVC(uint32_t level) = 0;
    virtual int queryControlsHEVC(uint32_t level, uint32_t tier) = 0;

    virtual int writeDumpDataToFile(v4l2_buffer* buffer) = 0;
    virtual int feedInputDataToV4l2Buffer(std::shared_ptr<v4l2_buffer> buf,
                                          bool& eos, uint32_t frameCount) = 0;

    int registerCallbacks(std::shared_ptr<V4l2CodecCallback> cb) {
        return mV4l2Driver->registerCallbacks(std::make_shared<V4l2DriverCallback>(cb));
    }

    int getColorFormat() const { return mPixelFmt; }
    int setResolution(int width, int height) {
        mWidth = width;
        mHeight = height;
        return 0;
    }
    int getFrameWidth() const { return mWidth; }
    int getFrameHeight() const { return mHeight; }
    int getFrameStride() const { return mStride; }
    int getFrameScanline() const { return mScanline; }
    int setStaticControls();
    int setDynamicControls(unsigned int currFrame);
    int setDynamicCommands(unsigned int currFrame);
    int setControl(unsigned int ctrlId, int value);
    int setInputSizeOverWrite(int size);
    int setInputActualCount(int count);
    int setOutputActualCount(int count);
    int getMinInputCount() const { return mMinInputCount; }
    int getMinOutputCount() const { return mMinOutputCount; }
    int getInputSize() const { return mInputSize; }
    int getOutputAllocCount() const { return mMinOutputCount; }
    int getOutputSize() const { return mOutputSize; }
    int getOutputBufferWidth() const { return mOBufWidth; }
    int getOubputBufferHeight() const { return mOBufHeight; }

    int startInput();
    int startOutput();

    int stopInput();
    int stopOutput();
    int allocateBuffers(enum port_type port);
    std::shared_ptr<v4l2_buffer> allocateBuffer(int index, enum port_type port, int bufSize);
    int queueBuffer(std::shared_ptr<v4l2_buffer> buffer);
    int setOutputBufferData(std::shared_ptr<v4l2_buffer> buf);
    int setDump(std::string inputFile, std::string outputFile);

  protected:
    std::mutex mBufLock;
    std::shared_ptr<V4l2Driver> mV4l2Driver;

    std::map<int, int> mIDRSeek, mRandomSeek;

    std::unordered_map<int, std::shared_ptr<DMABuffer>> mInputDMABuffersPool;
    std::unordered_map<int, std::shared_ptr<DMABuffer>> mOutputDMABuffersPool;

    std::list<std::shared_ptr<v4l2_buffer>> mInputBufs;
    std::list<std::shared_ptr<v4l2_buffer>> mPendingInputBufs;
    std::list<std::shared_ptr<v4l2_buffer>> mOutputBufs;
    std::list<std::shared_ptr<v4l2_buffer>> mPendingOutputBufs;

    std::list<std::shared_ptr<StaticV4L2CtrlInfo>> mStaticControls;
    std::list<std::shared_ptr<DynamicV4L2CtrlInfo>> mDynamicControls;
    std::list<std::shared_ptr<DynamicCommandInfo>> mDynamicCommands;

    FILE* mOutputDumpFile = nullptr;
    FILE* mInputDumpFile = nullptr;

  public:
    int populateStaticConfigs(
        std::list<std::shared_ptr<EventConfig>> cfgControls);
    int populateDynamicConfigs(
        std::list<std::shared_ptr<EventConfig>> cfgControls);
    int populateDynamicCommands(
        std::list<std::shared_ptr<EventConfig>> commands);
    bool fillCfgCtrls(std::shared_ptr<EventConfig> ctrl, unsigned int* id,
                      int* value);
    void freeBuffers(enum port_type port);
    bool isInputPortStarted() const { return mInputStreamonDone; }
    void setInputPortStarted(bool isStarted) { mInputStreamonDone = isStarted; }
    bool isOutputPortStarted() const { return mOutputStreamonDone; }
    void setOutputPortStarted(bool isStarted) { mOutputStreamonDone = isStarted; }
    bool isDrainPending() const { return mDrainPending; }
    void setDrainPending(bool postPone) { mDrainPending = postPone; }
    bool isReconfigEventReceived() const { return mReconfigEventReceived; }
    void setReconfigEventReceived(bool isReceived) { mReconfigEventReceived = isReceived; }
    bool isDrcLastFlagReceived() const { return mDrcLastFlagReceived; }
    void setDrcLastFlagReceived(bool isReceived) { mDrcLastFlagReceived = isReceived; }
    bool isDrainLastFlagReceived() const { return mDrainLastFlagReceived; }
    void setDrainLastFlagReceived(bool isReceived) { mDrainLastFlagReceived = isReceived; }
    bool isDrainSent() const { return mDrainSent; }
    void setDrainSent(bool isDrain) { mDrainSent = isDrain; }
    bool isFirstReconfigReceived() const { return mFirstReconfigReceived; }
    void setFirstReconfigReceived(bool isFirstReconfig) {
        mFirstReconfigReceived = isFirstReconfig;
    }

  public:
    std::atomic_bool mDrainLastFlagReceived = false;
    std::atomic_bool mDrcLastFlagReceived = false;
    std::atomic_bool mDrainSent = false;
    std::atomic_bool mErrorReceived = false;
    std::atomic_bool mFirstReconfigReceived = false;

  protected:
    std::atomic_bool mReconfigEventReceived = false;
    std::atomic_bool mInputStreamonDone = false;
    std::atomic_bool mOutputStreamonDone = false;
    std::atomic_bool mDrainPending = false;

    std::shared_ptr<V4l2CodecCallback> mCb;

    std::string mSessionId = 0;

    int mInputSizeOverWrite = 0;
    int mMinInputCount = 4;
    int mActualInputCount = 0;
    int mMinOutputCount = 11;
    int mActualOutputCount = 0;

    int mInputSize = 0;
    int mOutputSize = 0;
    int mWidth = 0;
    int mHeight = 0;
    int mOBufWidth = 0;
    int mOBufHeight = 0;
    int mStride = 0;
    int mScanline = 0;
    int mCropLeft = 0;
    int mCropTop = 0;
    int mCropWidth = 0;
    int mCropHeight = 0;

    unsigned int mInputColorPrimaries = V4L2_COLORSPACE_DEFAULT;
    unsigned int mInputMatrixCoeff = V4L2_YCBCR_ENC_DEFAULT;
    unsigned int mInputTransferChar = V4L2_XFER_FUNC_DEFAULT;
    unsigned int mInputVideoRange = V4L2_QUANTIZATION_DEFAULT;
    unsigned int mOutputColorPrimaries = V4L2_COLORSPACE_DEFAULT;
    unsigned int mOutputMatrixCoeff = V4L2_YCBCR_ENC_DEFAULT;
    unsigned int mOutputTransferChar = V4L2_XFER_FUNC_DEFAULT;
    unsigned int mOutputVideoRange = V4L2_QUANTIZATION_DEFAULT;

    unsigned int mDomain = 0;
    unsigned int mPixelFmt = 0;
    unsigned int mCodecFmt = 0;

};

#endif
