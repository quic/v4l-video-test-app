/*
 **************************************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 **************************************************************************************************
*/

#ifndef _FFPARSER_H_
#define _FFPARSER_H_

#include <string>
#include <unordered_map>

#include "Log.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/bsf.h>
#include <libavformat/avformat.h>
}

class FFStreamParser {
  public:
    FFStreamParser() = delete;
    explicit FFStreamParser(std::string inputPath, std::string sessionId);
    ~FFStreamParser();

    std::string id();

    int init();
    void deinit();

    int randomSeek();
    int loopPackets();
    int getNextPacket();
    int seekToFrame(int frame);
    int fillPacketData(void* dst, bool& eos);

  private:
    AVPacket* mPkt = nullptr;
    AVStream* mStream = nullptr;
    AVBSFContext* mBsf = nullptr;
    AVFormatContext* mFmtCtx = nullptr;

    bool mRawVideo = true;
    bool mBsfDataPending = false;

    std::string mInputPath = "";
    std::string mSessionId = "";

    int mFps_n = 0;
    int mFps_d = 1;
    int mWidth = 0;
    int mHeight = 0;
    int mCodecFmt = 0;
    int mTotalFrameCnt = 0;

    std::unordered_map<int, uint64_t> mPktPosition;
};

#endif
