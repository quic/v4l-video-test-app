#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <stdint.h>
#include <unistd.h>

#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include <linux/dma-heap.h>
#include <sys/mman.h>

struct Buffer {
    virtual ~Buffer() = default;
};

struct DMABuffer : public Buffer {
    explicit DMABuffer(uint32_t size, int fd) : mSize(size), mFd(dup(fd)) {}
    ~DMABuffer() {
        if (mFd >= 0) {
            close(mFd);
            mFd = -1;
        }
    }
    uint32_t mSize;
    int mFd;
};

struct MMAPBuffer : public Buffer {
    MMAPBuffer() {
        for (size_t i = 0; i < VIDEO_MAX_PLANES; i++) {
            start[i] = nullptr;
            length[i] = 0;
        }
    }
    ~MMAPBuffer() {
        for (size_t i = 0; i < VIDEO_MAX_PLANES; i++) {
            if (start[i] != nullptr) {
                munmap(start[i], length[i]);
            }
            start[i] = nullptr;
            length[i] = 0;
        }
    }
    void *start[VIDEO_MAX_PLANES];
    size_t length[VIDEO_MAX_PLANES];
};

#endif