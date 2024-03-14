/*
 **************************************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 **************************************************************************************************
*/

#ifndef _FF_YUV_PARSER_H_
#define _FF_YUV_PARSER_H_

#include <string>
#include "Log.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/parseutils.h>
}

class FFYUVParser {
  public:
    explicit FFYUVParser(std::string inputPath, std::string videoSize,
                         std::string pixelFmt, std::string sessionId);
    ~FFYUVParser();

    std::string id();
    int init();
    int fillPacketData(void* dst, int width, int height, int stride, int scanline, int colorFormat,
                       bool& eos);
    int deinit();
    int loopPackets();

  private:
    FILE* mInputFile = nullptr;

    int mFrameWidth = 0;
    int mFrameHeight = 0;

    std::string mPixelFmt = "";
    std::string mVideoSize = "";
    std::string mInputPath = "";
    std::string mSessionId = "";

    AVPacket* mPkt = nullptr;
    AVFormatContext* mFmtCtx = nullptr;
    AVDictionary* mFmtOptions = nullptr;
};

#endif
