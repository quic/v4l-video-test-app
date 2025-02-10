/*
 **************************************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 **************************************************************************************************
*/

#ifndef _TEST_DRIVER_H_
#define _TEST_DRIVER_H_

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include "Buffer.h"
#include "ConfigParser.h"
#include "Log.h"

#ifndef V4L2_CID_MPEG_VIDEO_LTR_COUNT
#define V4L2_CID_MPEG_VIDEO_LTR_COUNT                       (V4L2_CID_MPEG_BASE + 232)
#endif

#ifndef V4L2_CID_MPEG_VIDEO_FRAME_LTR_INDEX
#define V4L2_CID_MPEG_VIDEO_FRAME_LTR_INDEX                 (V4L2_CID_MPEG_BASE + 233)
#endif

#ifndef V4L2_CID_MPEG_VIDEO_USE_LTR_FRAMES
#define V4L2_CID_MPEG_VIDEO_USE_LTR_FRAMES                  (V4L2_CID_MPEG_BASE + 234)
#endif

#define V4L2_CID_MPEG_VIDEO_CONSTANT_QUALITY		        (V4L2_CID_MPEG_BASE + 645)

#define V4L2_CTRL_CLASS_CODEC		                        0x00990000	/* Stateful codec controls */
#define V4L2_CID_CODEC_BASE			                        (V4L2_CTRL_CLASS_CODEC | 0x900)
#define V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD	        (V4L2_CID_CODEC_BASE + 236)


#define V4L2_MPEG_VIDEO_BITRATE_MODE_CQ                     2
#define V4L2_MPEG_VIDEO_BITRATE_MODE_MBR                    3

#ifndef V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD_TYPE
#define V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD_TYPE       (V4L2_CID_CODEC_BASE + 237)
enum v4l2_mpeg_video_intra_refresh_period_type {
    V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD_TYPE_RANDOM = 0,
    V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD_TYPE_CYCLIC = 1,
};
#endif

#ifndef V4L2_PIX_FMT_QC08C
#define V4L2_PIX_FMT_QC08C                                  v4l2_fourcc('Q', '0', '8', 'C')
#endif

#ifndef V4L2_PIX_FMT_QC10C
#define V4L2_PIX_FMT_QC10C                                  v4l2_fourcc('Q', '1', '0', 'C')
#endif

#ifndef V4L2_PIX_FMT_AV1
#define V4L2_PIX_FMT_AV1                                    v4l2_fourcc('A', 'V', '1', '0')
#endif

#define INPUT_MPLANE V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE
#define OUTPUT_MPLANE V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE

#define INPUT_PLANES 1

#ifndef VERSION 
#define VERSION 1.0
#endif

#ifndef V4L2_MPEG_VIDEO_H264_LEVEL_5_2
#define V4L2_MPEG_VIDEO_H264_LEVEL_5_2                      16
#endif

#ifndef V4L2_MPEG_VIDEO_H264_LEVEL_6_0
#define V4L2_MPEG_VIDEO_H264_LEVEL_6_0                      17
#endif

#ifndef V4L2_MPEG_VIDEO_H264_LEVEL_6_1
#define V4L2_MPEG_VIDEO_H264_LEVEL_6_1                      18
#endif

#ifndef V4L2_MPEG_VIDEO_H264_LEVEL_6_2
#define V4L2_MPEG_VIDEO_H264_LEVEL_6_2                      19
#endif

#ifndef V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MIN_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MIN_QP     (V4L2_CID_MPEG_BASE + 647)
#endif

#ifndef V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MAX_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MAX_QP     (V4L2_CID_MPEG_BASE + 648)
#endif

#ifndef V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MIN_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MIN_QP     (V4L2_CID_MPEG_BASE + 649)
#endif

#ifndef V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MAX_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MAX_QP     (V4L2_CID_MPEG_BASE + 650)
#endif

#ifndef V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MIN_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MIN_QP     (V4L2_CID_MPEG_BASE + 651)
#endif

#ifndef V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MAX_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MAX_QP     (V4L2_CID_MPEG_BASE + 652)
#endif

#ifndef V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MIN_QP
#define V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MIN_QP     (V4L2_CID_MPEG_BASE + 389)
#endif

#ifndef V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MAX_QP
#define V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MAX_QP     (V4L2_CID_MPEG_BASE + 390)
#endif

#ifndef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L0_BR
#define V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L0_BR  (V4L2_CID_MPEG_BASE + 391)
#endif

#ifndef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L1_BR
#define V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L1_BR  (V4L2_CID_MPEG_BASE + 392)
#endif

#ifndef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L2_BR
#define V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L2_BR  (V4L2_CID_MPEG_BASE + 393)
#endif

#ifndef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L3_BR
#define V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L3_BR  (V4L2_CID_MPEG_BASE + 394)
#endif

#ifndef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L4_BR
#define V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L4_BR  (V4L2_CID_MPEG_BASE + 395)
#endif

#ifndef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L5_BR
#define V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L5_BR  (V4L2_CID_MPEG_BASE + 396)
#endif

#ifndef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L6_BR
#define V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L6_BR  (V4L2_CID_MPEG_BASE + 397)
#endif

#ifndef V4L2_CID_MPEG_VIDEO_BASELAYER_PRIORITY_ID
#define V4L2_CID_MPEG_VIDEO_BASELAYER_PRIORITY_ID   (V4L2_CID_MPEG_BASE + 230)
#endif

#ifndef V4L2_CID_MPEG_VIDEO_FRAME_SKIP_MODE
#define V4L2_CID_MPEG_VIDEO_FRAME_SKIP_MODE         (V4L2_CID_MPEG_BASE + 646)
enum v4l2_mpeg_video_frame_skip_mode {
    V4L2_MPEG_VIDEO_FRAME_SKIP_MODE_DISABLED    = 0,
    V4L2_MPEG_VIDEO_FRAME_SKIP_MODE_LEVEL_LIMIT = 1,
    V4L2_MPEG_VIDEO_FRAME_SKIP_MODE_BUF_LIMIT   = 2,
};
#endif

enum port_type {
    INPUT_PORT = 0,
    OUTPUT_PORT,
    MAX_PORT,
};

enum codec_type {
    V4L2_CODEC_TYPE_DECODER = 1,
    V4L2_CODEC_TYPE_ENCODER,
};

class V4l2CodecCallback;

class V4l2DriverCallback {
  public:
    explicit V4l2DriverCallback(std::shared_ptr<V4l2CodecCallback> codecCB)
        : mV4l2CodecCB(codecCB) {}
    virtual ~V4l2DriverCallback() = default;

    int onV4l2BufferDone(struct v4l2_buffer* buffer);
    int onV4l2EventDone(struct v4l2_event* event);
    int onV4l2Error(int error);

  private:
    std::shared_ptr<V4l2CodecCallback> mV4l2CodecCB;
};

class V4l2Driver {
  public:
    bool mError = false;

    V4l2Driver() = delete;
    explicit V4l2Driver(std::string sessionId);
    ~V4l2Driver();

    std::string id();

    int Open(int domain);
    void Close();
    int setCodecPixelFmt(uint32_t planeType, uint32_t codecFmt);
    int subscribeEvent(unsigned int event_type);
    int unsubscribeEvent(unsigned int event_type);

    int OpenDMAHeap(std::string device);
    void CloseDMAHeap();
    int AllocDMABuffer(uint64_t size, int* fd);
    int AllocMMAPBuffer(std::shared_ptr<MMAPBuffer> mmapBuf,
                        std::shared_ptr<v4l2_buffer> buf);

    int registerCallbacks(std::shared_ptr<V4l2DriverCallback> cb);
    int threadLoop();
    int createPollThread();
    int pausePollThread();
    int resumePollThread();
    int stopPollThread();

    int streamOn(int port);
    int streamOff(int port);

    int getFormat(struct v4l2_format* fmt);
    int setFormat(struct v4l2_format* fmt);
    int setParm(struct v4l2_streamparm* sparm);
    int setSelection(struct v4l2_selection* sel);
    int getSelection(struct v4l2_selection* sel);
    int getControl(struct v4l2_control* ctrl);
    int setControl(struct v4l2_control* ctrl);
    int setMemoryType(unsigned int memoryType);
    int reqBufs(struct v4l2_requestbuffers* reqBufs);
    int queueBuf(v4l2_buffer* buf);

    int decCommand(struct v4l2_decoder_cmd* cmd);
    int encCommand(struct v4l2_encoder_cmd* cmd);

    int queryCapabilities(struct v4l2_capability* caps);
    int queryControl(struct v4l2_queryctrl* ctrl);
    int enumFormat(struct v4l2_fmtdesc* fmtdesc);
    int enumFramesize(struct v4l2_frmsizeenum* frmsize);
    int enumFrameInterval(struct v4l2_frmivalenum* fival);

    int isMatchVideoDevice(int domain, int fd, const char* name);
    int queryMenu(v4l2_querymenu* querymenu);

  private:
    int mFd = -1;
    int mHeapFd = -1;

    std::string mSessionId;

    bool mThreadRunning = false;
    bool mPollThreadExit = false;
    bool mPollThreadPaused = false;

    unsigned int mMemoryType = 0;

    std::mutex mPollThreadLock;
    std::condition_variable mPauser;

    std::shared_ptr<V4l2DriverCallback> mCb;
    std::shared_ptr<std::thread> mPollThread;

    std::atomic_bool mBufferQueued = false;
};

#endif
