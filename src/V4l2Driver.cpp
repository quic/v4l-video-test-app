/*
 **************************************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 **************************************************************************************************
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include <linux/media.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "V4l2Codec.h"
#include "V4l2Driver.h"

#define MAX_VID_DEV_CNT 64

const char* ctrl_name(int id) {
    const char* name = "unknown";
    switch (id) {
        case V4L2_CID_MIN_BUFFERS_FOR_CAPTURE:
            name = "min output";
            break;
        case V4L2_CID_MIN_BUFFERS_FOR_OUTPUT:
            name = "min input";
            break;
        case V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE:
            name = "Frame RC Enable";
            break;
        case V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP:
        case V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_QP:
            name = "I Frame QP";
            break;
        case V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP:
        case V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_QP:
            name = "P Frame QP";
            break;
        case V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP:
        case V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_QP:
            name = "B Frame QP";
            break;
        case V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MAX_QP:
        case V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MAX_QP:
            name = "Max I Frame QP";
            break;
        case V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MAX_QP:
        case V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MAX_QP:
            name = "Max P Frame QP";
            break;
        case V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MAX_QP:
        case V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MAX_QP:
            name = "Max B Frame QP";
            break;
        case V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MIN_QP:
        case V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MIN_QP:
            name = "Min I Frame QP";
            break;
        case V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MIN_QP:
        case V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MIN_QP:
            name = "Min P Frame QP";
            break;
        case V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MIN_QP:
        case V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MIN_QP:
            name = "Min B Frame QP";
            break;
        case V4L2_CID_MPEG_VIDEO_H264_8X8_TRANSFORM:
            name = "H.264 8x8 Transform";
            break;
        case V4L2_CID_MPEG_VIDEO_PREPEND_SPSPPS_TO_IDR:
            name = "Prepend SPSPPS to IDR";
            break;
        case V4L2_CID_MPEG_VIDEO_HEVC_PROFILE:
            name = "HEVC Profile";
            break;
        case V4L2_CID_MPEG_VIDEO_HEVC_LEVEL:
            name = "HEVC Level";
            break;
        case V4L2_CID_MPEG_VIDEO_HEVC_TIER:
            name = "HEVC Tier";
            break;
        case V4L2_CID_MPEG_VIDEO_H264_PROFILE:
            name = "H.264 Profile";
            break;
        case V4L2_CID_MPEG_VIDEO_H264_LEVEL:
            name = "H.264 Level";
            break;
        case V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE:
            name = "H.264 EntropyCoding";
            break;
        case V4L2_CID_MPEG_VIDEO_HEADER_MODE:
            name = "Header Mode";
            break;
        case V4L2_CID_MPEG_VIDEO_BITRATE:
            name = "Bitrate";
            break;
        case V4L2_CID_MPEG_VIDEO_BITRATE_MODE:
            name = "Bitrate Mode";
            break;
        case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_TYPE:
        case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_TYPE:
            name = "Hierarchical Coding Type";
            break;
        case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER:
        case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_LAYER:
            name = "Hierarchical Coding Layer";
            break;
        case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L0_BR:
        case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L1_BR:
        case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L2_BR:
        case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L3_BR:
            name = "Hierarchical Coding Bitrate";
            break;
        case V4L2_CID_ROTATE:
            name = "Rotate";
            break;
        case V4L2_CID_HFLIP:
            name = "HFlip";
            break;
        case V4L2_CID_VFLIP:
            name = "VFlip";
            break;
        case V4L2_CID_MPEG_VIDEO_GOP_SIZE:
            name = "GOP Size";
            break;
        case V4L2_CID_MPEG_VIDEO_B_FRAMES:
            name = "B Frames";
            break;
        case V4L2_CID_MPEG_VIDEO_VBV_DELAY:
            name = "VBV Delay";
            break;
        case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE:
            name = "Multi-Slice Mode";
            break;
        case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB:
            name = "Multi-Slice Max MB";
            break;
        case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_BYTES:
            name = "Multi-Slice Max Bytes";
            break;
        case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE:
        case V4L2_CID_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE:
            name = "Loop Filter Mode";
            break;
        case V4L2_CID_MPEG_VIDEO_LTR_COUNT:
            name = "LTR Count";
            break;
        case V4L2_CID_MPEG_VIDEO_FRAME_LTR_INDEX:
            name = "Frame LTR Index";
            break;
        case V4L2_CID_MPEG_VIDEO_USE_LTR_FRAMES:
            name = "Use LTR Frame";
            break;
        case V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD:
            name = "Intra Refresh Period";
            break;
        case V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD_TYPE:
            name = "Intra Refresh Type";
            break;
        case V4L2_CID_MPEG_VIDEO_FORCE_KEY_FRAME:
            name = "Force Key Frame";
            break;
        default:
            break;
    }
    return name;
}

V4l2Driver::V4l2Driver(std::string sessionId)
    : mFd(-1),
      mPollThread(nullptr),
      mThreadRunning(false),
      mPollThreadExit(false),
      mSessionId(sessionId) {}

V4l2Driver::~V4l2Driver() {}

std::string V4l2Driver::id() {
    return mSessionId;
}

int V4l2Driver::Open(int domain) {
    char dev_video[16];
    int idx = 0, ret = 0;
    mFd = -1;

    if (domain != V4L2_CODEC_TYPE_DECODER && domain != V4L2_CODEC_TYPE_ENCODER) {
        LOGE("this domain(%d) is not for decoder and encoder\n", domain);
        return -EINVAL;
    }

    while (idx < MAX_VID_DEV_CNT) {
        memset(dev_video, 0, sizeof(dev_video));

        ret = snprintf(dev_video, sizeof(dev_video), "/dev/video%d", idx);
        if (ret <= 0) {
            return ret;
        }

        LOGI("open video device: %s\n", dev_video);
        mFd = open(dev_video, O_RDWR);
        if (mFd < 0) {
            mFd = -1;
            LOGE("Failed to open video device: %s (%s)\n", dev_video,
                strerror(errno));
            idx++;
            continue;
        }

        ret = isMatchVideoDevice(domain, mFd, dev_video);
        if (ret < 0) {
            close(mFd);
            mFd = -1;
            idx++;
            LOGW("(%s) is not for (%s), so close it\n", dev_video,
                domain == V4L2_CODEC_TYPE_DECODER ? "decoder" : "encoder");
            continue;
        }
        break;
    }
    if (idx >= MAX_VID_DEV_CNT || mFd < 0 || ret < 0) {
        LOGE("Failed to open video device for %s (%s)\n",
            domain == V4L2_CODEC_TYPE_DECODER ? "decoder" : "encoder",
            strerror(errno));
        return -EINVAL;
    }

    LOGW("open %s successful for %s fd: %d\n", dev_video,
        domain == V4L2_CODEC_TYPE_DECODER ? "decoder" : "encoder", mFd);
    return 0;
}

void V4l2Driver::Close() {
    if (mFd >= 0) {
        close(mFd);
        mFd = -1;
    }
    LOGI("driver closed.\n");
}

int V4l2Driver::isMatchVideoDevice(int domain, int fd, const char* name) {
    struct v4l2_capability cap;
    struct v4l2_fmtdesc fdesc;
    bool found = false;
    int ret = 0;

    memset(&cap, 0, sizeof(cap));
    ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if (ret < 0) {
        LOGE("Failed to query capabilities: %d\n", ret);
        return -1;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING) ||
        !(cap.capabilities & V4L2_CAP_VIDEO_M2M_MPLANE)) {
        LOGE("this device is not for video(%s)\n", name);
        return -1;
    }

    memset(&fdesc, 0, sizeof(fdesc));
    if (domain == V4L2_CODEC_TYPE_DECODER) {
        fdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    } else if (domain == V4L2_CODEC_TYPE_ENCODER) {
        fdesc.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    } else {
        LOGE("this domain(%d) is error\n", domain);
        return -1;
    }

    while (!ioctl(fd, VIDIOC_ENUM_FMT, &fdesc)) {
        if (fdesc.pixelformat == V4L2_PIX_FMT_NV12 || fdesc.pixelformat == V4L2_PIX_FMT_NV21 ||
            fdesc.pixelformat == V4L2_PIX_FMT_QC08C || fdesc.pixelformat == V4L2_PIX_FMT_QC10C) {
            LOGI("find pixelformat for %s description: %s\n",
                domain == V4L2_CODEC_TYPE_DECODER ? "decoder output"
                                                  : "encoder input",
                fdesc.description);
            found = true;
            break;
        }
        fdesc.index++;
    }

    if (!found) {
        LOGI("%s is not for %s from format: %d\n", name,
            domain == V4L2_CODEC_TYPE_DECODER ? "decoder output"
                                              : "encoder input",
            fdesc.pixelformat);
        return -1;
    }

    found = false;
    memset(&fdesc, 0, sizeof(fdesc));
    if (domain == V4L2_CODEC_TYPE_DECODER) {
        fdesc.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    } else if (domain == V4L2_CODEC_TYPE_ENCODER) {
        fdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    } else {
        LOGE("this domain(%d) is error\n", domain);
        return -1;
    }

    while (!ioctl(fd, VIDIOC_ENUM_FMT, &fdesc)) {
        if (fdesc.pixelformat == V4L2_PIX_FMT_H264 || fdesc.pixelformat == V4L2_PIX_FMT_HEVC ||
            fdesc.pixelformat == V4L2_PIX_FMT_VP9 || fdesc.pixelformat == V4L2_PIX_FMT_VP8 ||
            fdesc.pixelformat == V4L2_PIX_FMT_H263 || fdesc.pixelformat == V4L2_PIX_FMT_MPEG) {
            LOGI("find codec for %s description: %s\n",
                domain == V4L2_CODEC_TYPE_DECODER ? "decoder input"
                                                  : "encoder output",
                fdesc.description);
            found = true;
            break;
        }
        fdesc.index++;
    }

    if (!found) {
        LOGE("%s is not for %s from format: %d\n", name,
            domain == V4L2_CODEC_TYPE_DECODER ? "decoder intput"
                                              : "encoder output",
            fdesc.pixelformat);
        return -1;
    }

    return 0;
}

int V4l2Driver::subscribeEvent(unsigned int event_type) {
    int ret = 0;
    struct v4l2_event_subscription event;
    memset(&event, 0, sizeof(event));
    event.type = event_type;
    LOGD("subscribeEvent: type %d\n", event_type);
    ret = ioctl(mFd, VIDIOC_SUBSCRIBE_EVENT, &event);
    if (ret) {
        LOGE("subscribeEvent: error %d\n", ret);
        return ret;
    }
    return 0;
}

int V4l2Driver::unsubscribeEvent(unsigned int event_type) {
    int ret = 0;
    struct v4l2_event_subscription event;
    memset(&event, 0, sizeof(event));
    event.type = event_type;
    LOGD("unsubscribeEvent: type %d\n", event_type);
    ret = ioctl(mFd, VIDIOC_UNSUBSCRIBE_EVENT, &event);
    if (ret) {
        LOGE("unsubscribeEvent: error %d\n", ret);
        return ret;
    }
    return 0;
}

int V4l2Driver::OpenDMAHeap(std::string device) {
    std::string dma_dir = std::string("/dev/dma_heap/");
    std::string dma_path = dma_dir + device;
    mHeapFd = open(dma_path.c_str(), O_RDWR);
    if (mHeapFd <= 0) {
        LOGE("Error: Failed to open %s\n", dma_path.c_str());
        return -EINVAL;
    }
    return 0;
}

void V4l2Driver::CloseDMAHeap() {
    if (mHeapFd >= 0) {
        close(mHeapFd);
        mHeapFd = -1;
    }
    std::cerr << "Heap fd closed." << std::endl;
}

int V4l2Driver::AllocDMABuffer(uint64_t size, int* fd) {
    int ret = 0;
    struct dma_heap_allocation_data alloc = {
        .len = size,
        .fd = 0,
        .fd_flags = O_RDWR | O_CLOEXEC,
        .heap_flags = 0,
    };
    ret = ioctl(mHeapFd, DMA_HEAP_IOCTL_ALLOC, &alloc);
    if (ret < 0) {
        std::cerr << "Error: alloc heap buf failed." << std::endl;
        return ret;
    }
    *fd = alloc.fd;

    return ret;
}

void ThreadFunc(V4l2Driver& driver) {
    driver.threadLoop();
}

int V4l2DriverCallback::onV4l2BufferDone(struct v4l2_buffer* buffer) {
    return mV4l2CodecCB->onBufferDone(buffer);
}

int V4l2DriverCallback::onV4l2EventDone(struct v4l2_event* event) {
    return mV4l2CodecCB->onEventDone(event);
}

int V4l2DriverCallback::onV4l2Error(int error) {
    return mV4l2CodecCB->onError(error);
}

int V4l2Driver::registerCallbacks(std::shared_ptr<V4l2DriverCallback> cb) {
    LOGV("V4l2Driver::registerCallbacks\n");
    mCb = cb;
    return 0;
}

int V4l2Driver::threadLoop() {
    struct v4l2_buffer buffer;
    struct v4l2_plane plane[INPUT_PLANES];
    struct v4l2_event event;
    struct pollfd pollFds[2];
    LOGV("V4l2Driver::threadLoop() begins.\n");

    mThreadRunning = true;
    pollFds[0].events = POLLIN | POLLRDNORM | POLLOUT | POLLWRNORM | POLLRDBAND | POLLPRI | POLLERR;
    pollFds[0].fd = mFd;

    while (!mPollThreadExit) {
        int ret = poll(pollFds, 1, 1000);
        if (ret == -ETIMEDOUT) {
            LOGW("V4l2Driver: poll timedout\n");
            continue;
        } else if (ret < 0 && errno != EINTR && errno != EAGAIN) {
            LOGE("V4l2Driver: poll error %d\n", ret);
            mCb->onV4l2Error(EAGAIN);
            break;
        }
        if (pollFds[0].revents & POLLERR) {
            LOGI("V4l2Driver: poll error received\n");
            mError = true;
            mCb->onV4l2Error(POLLERR);
            break;
        }
        if (pollFds[0].revents & POLLPRI) {
            LOGI("V4l2Driver: PRI received.\n");
            memset(&event, 0, sizeof(event));
            if (!ioctl(mFd, VIDIOC_DQEVENT, &event)) {
                LOGI("V4l2Driver: Received v4l2 event, type %#x\n", event.type);
                mCb->onV4l2EventDone(&event);
            }
        }
        if ((pollFds[0].revents & POLLIN) || (pollFds[0].revents & POLLRDNORM)) {
            LOGV("V4l2Driver: IN/RDNORM received.\n");
            memset(&buffer, 0, sizeof(buffer));
            memset(&plane[0], 0, sizeof(plane));
            buffer.type = OUTPUT_MPLANE;
            buffer.m.planes = plane;
            buffer.length = 1;
            buffer.memory = V4L2_MEMORY_DMABUF;
            do {
                if (ioctl(mFd, VIDIOC_DQBUF, &buffer)) {
                    break;
                }

                if (mCb->onV4l2BufferDone(&buffer)) {
                    mError = true;
                }
            } while (1);
        }
        if ((pollFds[0].revents & POLLOUT) || (pollFds[0].revents & POLLWRNORM)) {
            LOGV("V4l2Driver: OUT/WRNORM received.\n");
            memset(&buffer, 0, sizeof(buffer));
            memset(&plane[0], 0, sizeof(plane));
            buffer.type = INPUT_MPLANE;
            buffer.m.planes = plane;
            buffer.length = 1;
            buffer.memory = V4L2_MEMORY_DMABUF;
            do {
                if (ioctl(mFd, VIDIOC_DQBUF, &buffer)) {
                    break;
                }
                if (mCb->onV4l2BufferDone(&buffer)) {
                    mError = true;
                }
            } while (1);
        }
        {
            std::unique_lock<std::mutex> lock(mPollThreadLock);
            if (mPollThreadPaused) {
                // Wait for resume.
                mPauser.wait(lock);
            }
        }
    }
    LOGV("V4l2Driver::threadLoop() ends.\n");
    mThreadRunning = false;
    return 0;
}

int V4l2Driver::createPollThread() {
    mPollThread = std::make_shared<std::thread>(ThreadFunc, std::ref(*this));
    if (!mPollThread) {
        LOGE("poll thread create failed\n");
        return -EINVAL;
    } else {
        int count = 0;
        while (!mThreadRunning) {
            LOGD("wait for poll thread running\n");
            usleep(10 * 1000);  // 10 ms
            count++;
            if (count >= 100) {
                break;
            }
        }
        if (!mThreadRunning) {
            LOGE("poll thread not running\n");
            return -EINVAL;
        }
    }
    LOGV("createPollThread: poll thread started\n");
    return 0;
}

int V4l2Driver::pausePollThread() {
    if (!mPollThread || mPollThreadExit) {
        LOGE("pausePollThread: invalid poll thread. exit %d\n", mPollThreadExit);
        return -EINVAL;
    }
    std::unique_lock<std::mutex> lock(mPollThreadLock);
    mPollThreadPaused = true;
    return 0;
}

int V4l2Driver::resumePollThread() {
    if (!mPollThread || mPollThreadExit) {
        LOGE("resumePollThread: invalid poll thread. exit %d\n",
            mPollThreadExit);
        return -EINVAL;
    }
    std::unique_lock<std::mutex> lock(mPollThreadLock);
    mPollThreadPaused = false;
    mPauser.notify_one();
    return 0;
}

int V4l2Driver::stopPollThread() {
    if (!mPollThread || mPollThreadExit) {
        LOGE("stopPollThread: invalid poll thread. exit %d\n", mPollThreadExit);
        return -EINVAL;
    }
    mPollThreadExit = true;
    LOGV("stopPollThread: join thread\n");
    if (mPollThread != nullptr && mPollThread->joinable()) {
        mPollThread->join();
    }
    mPollThread = nullptr;
    LOGV("stopPollThread: exit poll thread\n");
    return 0;
}

int V4l2Driver::streamOn(int port) {
    LOGV("streamon: port %d\n", port);
    int ret = ioctl(mFd, VIDIOC_STREAMON, &port);
    if (ret) {
        LOGE("streamon failed for port %d\n", port);
        return -EINVAL;
    }
    return 0;
}

int V4l2Driver::streamOff(int port) {
    LOGV("streamoff: port %d\n", port);
    int ret = ioctl(mFd, VIDIOC_STREAMOFF, &port);
    if (ret) {
        LOGE("streamoff failed for port %d\n", port);
        return -EINVAL;
    }
    return 0;
}

int V4l2Driver::getFormat(v4l2_format* fmt) {
    int ret = ioctl(mFd, VIDIOC_G_FMT, fmt);
    if (ret) {
        LOGE("getFormat failed for type %d\n", fmt->type);
        return -EINVAL;
    }
    LOGD("getFormat: type %d, [wxh] %dx%d, fmt %#x, size %d\n", fmt->type,
        fmt->fmt.pix_mp.width, fmt->fmt.pix_mp.height,
        fmt->fmt.pix_mp.pixelformat, fmt->fmt.pix_mp.plane_fmt[0].sizeimage);
    return 0;
}

int V4l2Driver::setCodecPixelFmt(uint32_t planeType, uint32_t codecPixFmt) {
    struct v4l2_fmtdesc fmtdesc;
    struct v4l2_format fmt;
    bool found = false;
    int ret = 0;

    /* check if driver supports client requested fomat */
    memset(&fmtdesc, 0, sizeof(fmtdesc));
    fmtdesc.index = 0;
    fmtdesc.type = planeType;
    while (!ret) {
        ret = ioctl(mFd, VIDIOC_ENUM_FMT, &fmtdesc);
        if (ret) {
            break;
        }
        if (fmtdesc.pixelformat == codecPixFmt) {
            found = true;
            break;
        }
        fmtdesc.index++;
    }
    if (!found) {
        LOGE("client format %#x not supported\n", codecPixFmt);
        return -EINVAL;
    }

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = planeType;
    ret = ioctl(mFd, VIDIOC_G_FMT, &fmt);
    if (ret) {
        LOGE("getFormat failed for type %d\n", fmt.type);
        return -EINVAL;
    }
    fmt.fmt.pix_mp.pixelformat = codecPixFmt;

    ret = ioctl(mFd, VIDIOC_S_FMT, &fmt);
    if (ret) {
        LOGE("setFormat failed for type %d\n", fmt.type);
        return -EINVAL;
    }
    LOGD("pixelCodecFmt: type %d, [wxh] %dx%d, fmt %#x, size %d\n", fmt.type,
        fmt.fmt.pix_mp.width, fmt.fmt.pix_mp.height, fmt.fmt.pix_mp.pixelformat,
        fmt.fmt.pix_mp.plane_fmt[0].sizeimage);

    return ret;
}

int V4l2Driver::getSelection(v4l2_selection* sel) {
    int ret = ioctl(mFd, VIDIOC_G_SELECTION, sel);
    if (ret) {
        LOGE("getSelection failed for type %d, target %d\n", sel->type,
            sel->target);
        return -EINVAL;
    }
    LOGD("getSelection: type %d, target %d, left %d top %d width %d height %d\n",
        sel->type, sel->target, sel->r.left, sel->r.top, sel->r.width,
        sel->r.height);
    return 0;
}

int V4l2Driver::getControl(v4l2_control* ctrl) {
    int ret = ioctl(mFd, VIDIOC_G_CTRL, ctrl);
    if (ret) {
        LOGE("getCotrol failed for \"%s\"\n", ctrl_name(ctrl->id));
        return -EINVAL;
    }
    LOGD("getControl: \"%s\", value %d\n", ctrl_name(ctrl->id), ctrl->value);
    return 0;
}

int V4l2Driver::setControl(v4l2_control* ctrl) {
    LOGD("setControl: \"%s\", value %d\n", ctrl_name(ctrl->id), ctrl->value);
    int ret = ioctl(mFd, VIDIOC_S_CTRL, ctrl);
    if (ret) {
        LOGE("setCotrol failed for \"%s\"\n", ctrl_name(ctrl->id));
        return -EINVAL;
    }

    return 0;
}

int V4l2Driver::reqBufs(struct v4l2_requestbuffers* reqbufs) {
    int ret = 0;

    LOGD("reqBufs: type %d, count %d memory %d\n", reqbufs->type, reqbufs->count,
        reqbufs->memory);
    ret = ioctl(mFd, VIDIOC_REQBUFS, reqbufs);
    if (ret) {
        LOGE("reqBufs failed for type %d, count %d memory %d\n", reqbufs->type,
            reqbufs->count, reqbufs->memory);
        return -EINVAL;
    }
    return 0;
}

int V4l2Driver::queueBuf(v4l2_buffer* buf) {
    int ret = ioctl(mFd, VIDIOC_QBUF, buf);
    if (ret) {
        LOGE("failed to QBUF: %s\n", strerror(ret));
        return -EINVAL;
    }
    return 0;
}

int V4l2Driver::decCommand(v4l2_decoder_cmd* cmd) {
    int ret = ioctl(mFd, VIDIOC_DECODER_CMD, cmd);
    if (ret) {
        LOGE("decCommand: error %d\n", ret);
        return -EINVAL;
    }
    return 0;
}

int V4l2Driver::encCommand(v4l2_encoder_cmd* cmd) {
    int ret = ioctl(mFd, VIDIOC_ENCODER_CMD, cmd);
    if (ret) {
        LOGE("encCommand: error %d\n", ret);
        return -EINVAL;
    }
    return 0;
}

int V4l2Driver::setFormat(struct v4l2_format* fmt) {
    int ret = ioctl(mFd, VIDIOC_S_FMT, fmt);
    if (ret) {
        LOGE("setFormat failed for type %d\n", fmt->type);
        return -EINVAL;
    }
    LOGV("setFormat: type %d, [wxh] %dx%d, fmt %#x, size %d\n", fmt->type,
        fmt->fmt.pix_mp.width, fmt->fmt.pix_mp.height,
        fmt->fmt.pix_mp.pixelformat, fmt->fmt.pix_mp.plane_fmt[0].sizeimage);
    return 0;
}

int V4l2Driver::setParm(v4l2_streamparm* sparm) {
    int ret = ioctl(mFd, VIDIOC_S_PARM, sparm);
    if (ret) {
        LOGE("setParm failed for type %u\n", sparm->type);
        return -EINVAL;
    }
    return 0;
}

int V4l2Driver::setSelection(v4l2_selection* sel) {
    int ret = ioctl(mFd, VIDIOC_S_SELECTION, sel);
    if (ret) {
        LOGE("setSelection failed for type %d, target %d\n", sel->type,
            sel->target);
        return -EINVAL;
    }
    LOGV("setSelection: type %d, target %d, left %d top %d width %d height %d\n",
        sel->type, sel->target, sel->r.left, sel->r.top, sel->r.width,
        sel->r.height);
    return 0;
}

int V4l2Driver::queryCapabilities(v4l2_capability* caps) {
    int ret = ioctl(mFd, VIDIOC_QUERYCAP, caps);
    if (ret) {
        LOGE("Failed to query capabilities\n");
        return -EINVAL;
    }
    LOGV("queryCapabilities: driver name: %s, card: %s, bus_info: %s, "
        "version: %d, capabilities: %#x, device_caps: %#x \n",
        caps->driver, caps->card, caps->bus_info, caps->version,
        caps->capabilities, caps->device_caps);
    return 0;
}

int V4l2Driver::queryMenu(v4l2_querymenu* querymenu) {
    int ret = ioctl(mFd, VIDIOC_QUERYMENU, querymenu);
    if (ret) {
        LOGE("Failed to query menu: %s\n", ctrl_name(querymenu->id));
        return -EINVAL;
    }
    LOGV("queryMenu: name: \"%s\", id: %#x, index: %d\n", querymenu->name,
        querymenu->id, querymenu->index);
    return 0;
}

int V4l2Driver::queryControl(v4l2_queryctrl* ctrl) {
    int ret = ioctl(mFd, VIDIOC_QUERYCTRL, ctrl);
    if (ret) {
        LOGE("Failed to query ctrl: %s\n", ctrl_name(ctrl->id));
        return -EINVAL;
    }
    LOGV("queryCotrol: name: \"%s\", min: %d, max: %d, step: %d, default: %d\n",
        ctrl->name, ctrl->minimum, ctrl->maximum, ctrl->step,
        ctrl->default_value);
    return 0;
}

int V4l2Driver::enumFormat(v4l2_fmtdesc* fmtdesc) {
    int ret = ioctl(mFd, VIDIOC_ENUM_FMT, fmtdesc);
    if (ret) {
        LOGE("enumFormat ended for index %d\n", fmtdesc->index);
        return -ENOTSUP;
    }
    LOGV("enumFormat: index %d, description: \"%s\", pixelFmt: %#x, flags: "
        "%#x\n",
        fmtdesc->index, fmtdesc->description, fmtdesc->pixelformat,
        fmtdesc->flags);
    return 0;
}

int V4l2Driver::enumFramesize(v4l2_frmsizeenum* frmsize) {
    int ret = ioctl(mFd, VIDIOC_ENUM_FRAMESIZES, frmsize);
    if (ret) {
        LOGE("enumFramesize failed for pixel_format %#x\n",
            frmsize->pixel_format);
        return -EINVAL;
    }
    if (frmsize->type != V4L2_FRMSIZE_TYPE_STEPWISE) {
        LOGE("enumFramesize: type (%d) returned in not stepwise\n",
            frmsize->type);
        return -EINVAL;
    }
    LOGV("enumFramesize: [%u x %u] to [%u x %u]\n", frmsize->stepwise.min_width,
        frmsize->stepwise.min_height, frmsize->stepwise.max_width,
        frmsize->stepwise.max_height);
    return 0;
}

int V4l2Driver::enumFrameInterval(v4l2_frmivalenum* fival) {
    int ret = ioctl(mFd, VIDIOC_ENUM_FRAMEINTERVALS, fival);
    if (ret) {
        LOGE("enumFrameInterval failed for pixel_format %#x\n",
            fival->pixel_format);
        return -EINVAL;
    }
    if (fival->type != V4L2_FRMIVAL_TYPE_STEPWISE) {
        LOGE("enumFramesize: type (%d) returned in not stepwise\n", fival->type);
        return -EINVAL;
    }
    LOGV("enumFrameInterval: resoltion [%u x %u], interval [%u / %u] to [%u / "
        "%u]\n",
        fival->width, fival->height, fival->stepwise.min.numerator,
        fival->stepwise.min.denominator, fival->stepwise.max.numerator,
        fival->stepwise.max.denominator);

    return 0;
}
