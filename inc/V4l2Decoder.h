/*
 **************************************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 **************************************************************************************************
*/

#ifndef _V4L2_DECODER_H_
#define _V4L2_DECODER_H_
#include <memory>

#include <deque>

#include "ConfigParser.h"
#include "V4l2Codec.h"
#include "V4l2Driver.h"

class FFStreamParser;

class V4l2Decoder : public V4l2Codec {
  public:
    V4l2Decoder() = delete;
    explicit V4l2Decoder(unsigned int codec, unsigned int pixel,
                         std::string sessionId);
    virtual ~V4l2Decoder();

    /** V4L2 Codec Hooks **/
    int init() override;
    void deinit() override;

    int stop() override;
    int start() override;
    int configureInput() override;
    int configureOutput() override;
    int reconfigureOutput() override;
    int queueBuffers(int maxFrameCnt) override;
    int queryControlsAVC(uint32_t level) override { return -1; }
    int queryControlsHEVC(uint32_t level, uint32_t tier) override {
        return -1;
    };

    int writeDumpDataToFile(struct v4l2_buffer* buffer) override;
    int feedInputDataToV4l2Buffer(std::shared_ptr<v4l2_buffer> buf, bool& eos,
                                  uint32_t frameCount) override;

    bool detectBitDepthChange() override;

    /** V4L2 Decoder Hooks **/
    int pause();
    int resume();

    int initFFStreamParser(std::string inputPath);

    void deinitFFStreamParser();
    void setPause(int pause, int duration);

    int randomSeek();
    int handleSeek(int seekTo);
    int seekToFrame(int frame);
    int handleRandomSeek(int& seekPos);
    int detectResolutionChange(bool* hasResolutionChanged);

  private:
    friend class V4l2DecoderCB;
    std::shared_ptr<FFStreamParser> mStreamParser;
    bool mWillSeek = true;
};

class V4l2DecoderCB : public V4l2CodecCallback {
  public:
    V4l2DecoderCB() = delete;
    explicit V4l2DecoderCB(V4l2Decoder* dec, std::string sessionId);
    virtual ~V4l2DecoderCB() = default;

    int onBufferDone(struct v4l2_buffer* buffer) override;
    int onEventDone(struct v4l2_event* event) override;
    int onError(int error) override;

  private:
    V4l2Decoder* mDec;
};

#endif
