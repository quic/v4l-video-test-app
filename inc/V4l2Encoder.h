/*
 **************************************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 **************************************************************************************************
*/

#ifndef _V4L2_ENCODER_H_
#define _V4L2_ENCODER_H_

#include <memory>
#include <unordered_set>

#include "ConfigParser.h"
#include "V4l2Codec.h"
#include "V4l2Driver.h"

#define DUMP_BUF_DATA 0

class FFYUVParser;

class V4l2Encoder : public V4l2Codec {
  public:
    V4l2Encoder() = delete;

    explicit V4l2Encoder(unsigned int codec, unsigned int pixel,
                         std::string sessionId);

    virtual ~V4l2Encoder();

    /** V4L2 Codec Hooks **/
    int init() override;
    void deinit() override;

    int stop() override;
    int start() override;
    int configureInput() override;
    int configureOutput() override;
    int queueBuffers(int maxFrameCnt) override;
    int queryControlsAVC(uint32_t level) override;
    int queryControlsHEVC(uint32_t level, uint32_t tier) override;

    int reconfigureOutput() override { return 0; }
    int writeDumpDataToFile(struct v4l2_buffer* buffer) override;
    int feedInputDataToV4l2Buffer(std::shared_ptr<v4l2_buffer> buf, bool& eos,
                                  uint32_t frameCount) override;

    bool detectBitDepthChange() override { return false; }

    /** V4L2 Encoder Hooks **/
    int setFrameRate(unsigned int numer, unsigned int denom);
    int setOperatingRate(unsigned int numer, unsigned int denom);
    int replaceNalSizeWAndWrite(std::uint8_t* basePtr, unsigned int filledLen);
    int initFFYUVParser(std::string inputPath, int width, int height, std::string pixfmt);

    void deinitFFYUVParser();
    void setNALEncoding(bool enable) { mNALEncodingEnabled = enable; }
    void logV4l2BufferDataToFile(std::uint8_t* buffer, int buffer_len, int idx);

    bool isNALEncodingEnabled() const { return mNALEncodingEnabled; }

  private:
    friend class V4l2EncoderCB;

    std::shared_ptr<FFYUVParser> mYUVParser;

    std::unordered_set<int> mLTRIndex;
    std::unordered_map<int, int> mUseLTR;

    uint32_t mFrameRate = 30;
    bool mNALEncodingEnabled = false;
    int32_t mEncodedBufferReceieved = 0;

#if DUMP_BUF_DATA
    FILE* gDumpV4L2BufferDataFile = nullptr;
#endif
};

class V4l2EncoderCB : public V4l2CodecCallback {
  public:
    V4l2EncoderCB() = delete;
    explicit V4l2EncoderCB(V4l2Encoder* enc, std::string sessionId)
        : V4l2CodecCallback(sessionId), mEnc(enc) {}
    virtual ~V4l2EncoderCB() = default;

    int onBufferDone(struct v4l2_buffer* buffer) override;
    int onEventDone(struct v4l2_event* event) override { return 0; }
    int onError(int error) override;

  private:
    V4l2Encoder* mEnc;
};

#endif
