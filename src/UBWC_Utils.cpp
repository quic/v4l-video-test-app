/*
 **************************************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 **************************************************************************************************
*/

#include <linux/videodev2.h>
#include <numeric>

#include "UBWC_Utils.h"
#include "V4l2Driver.h"

const std::unordered_map<int, BufferLayoutInfo> kBufferLayoutInfoMap = {
    {V4L2_PIX_FMT_QC08C,
     {{
         {true, {1, 1, 1, 32, 64}, {1, 1, 1, 8, 16}},    // primaryMetaPlane
         {true, {128, 1, 1, 1, 128}, {1, 1, 1, 1, 32}},  // primaryPlane
         {true, {1, 1, 2, 16, 64}, {1, 1, 2, 8, 16}},    // secondaryMetaPlane
         {true, {1, 1, 1, 1, 128}, {1, 1, 2, 1, 32}},    // secondaryPlane
     }}},
    {V4L2_PIX_FMT_QC10C,
     {{
         {true, {1, 1, 1, 48, 64}, {1, 1, 1, 4, 16}},    // primaryMetaPlane
         {true, {192, 4, 3, 1, 256}, {1, 1, 1, 1, 16}},  // primaryPlane
         {true, {1, 1, 2, 24, 64}, {1, 1, 2, 4, 16}},    // secondaryMetaPlane
         {true, {192, 4, 3, 1, 256}, {1, 1, 2, 1, 16}},  // secondaryPlane
     }}},
};

static constexpr uint32_t kBufferSizeAlignment = 4096;
static constexpr uint32_t kMaxInterlaceWidth = 1920;
static constexpr uint32_t kMaxInterlaceHeight = 1920;
static constexpr uint32_t kInterlaceMBSize = 256;
static constexpr uint32_t kMaxInterlaceMB = (1920 * 1088) / kInterlaceMBSize;
static constexpr BufferLayoutInfo kInvalidBufferLayoutInfo;

static uint32_t align(uint32_t size, uint32_t alignment) {
    if (size == 0) {
        return 0;
    }
    if (alignment <= 1) {
        return size;
    }
    return (size - 1) / alignment * alignment + alignment;
}

static uint32_t divRoundUp(uint32_t dividend, uint32_t divisor) {
    if (divisor <= 1) {
        return dividend;
    }
    return (dividend + divisor - 1) / divisor;
}

const BufferLayoutInfo& getBufferLayoutInfo(int fmt) {
    auto search = kBufferLayoutInfoMap.find(fmt);
    if (search != kBufferLayoutInfoMap.end()) {
        return search->second;
    }
    return kInvalidBufferLayoutInfo;
}

static uint32_t getPlaneStride(const PlaneInfo& planeInfo, uint32_t w) {
    if (!planeInfo.valid) {
        return 0;
    }
    const auto& strideInfo = planeInfo.strideInfo;
    uint32_t stride = align(w, strideInfo.pixelAlignment);
    stride = stride * strideInfo.bytesPerPixel_Num;
    stride = divRoundUp(stride, strideInfo.bytesPerPixel_Den);
    stride = divRoundUp(stride, strideInfo.tile);
    stride = align(stride, strideInfo.alignment);
    return stride;
}

static uint32_t getPlaneScanlines(const PlaneInfo& planeInfo, uint32_t h) {
    if (!planeInfo.valid) {
        return 0;
    }
    const auto& scInfo = planeInfo.scanlinesInfo;
    uint32_t scanlines = align(h, scInfo.pixelAlignment);
    scanlines = scanlines * scInfo.bytesPerPixel_Num;
    scanlines = divRoundUp(scanlines, scInfo.bytesPerPixel_Den);
    scanlines = divRoundUp(scanlines, scInfo.tile);
    scanlines = align(scanlines, scInfo.alignment);
    return scanlines;
}

static bool canInterlace(int fmt, uint32_t w, uint32_t h) {
    return fmt == V4L2_PIX_FMT_QC08C && w <= kMaxInterlaceWidth && h <= kMaxInterlaceHeight &&
           (w * h) / kInterlaceMBSize <= kMaxInterlaceMB;
}

uint32_t getPlaneSize(const PlaneInfo& planeInfo, uint32_t w, uint32_t h) {
    if (!planeInfo.valid) {
        return 0;
    }
    auto stride = getPlaneStride(planeInfo, w);
    auto scanlines = getPlaneScanlines(planeInfo, h);
    return align(stride * scanlines, kBufferSizeAlignment);
}

uint32_t getBufferSize(int fmt, uint32_t pixelWidth, uint32_t pixelHeight) {
    auto& layoutInfo = getBufferLayoutInfo(fmt);
    uint32_t size = 0;
    if (canInterlace(fmt, pixelWidth, pixelHeight)) {
        uint32_t w = pixelWidth;
        uint32_t h = divRoundUp(pixelHeight, 2);
        size = std::accumulate(layoutInfo.begin(), layoutInfo.end(), 0,
                               [&](uint32_t total, auto& planeInfo) -> uint32_t {
                                   return total + getPlaneSize(planeInfo, w, h);
                               });
        size *= 2;
    } else {
        size = std::accumulate(layoutInfo.begin(), layoutInfo.end(), 0,
                               [&](uint32_t total, auto& planeInfo) -> uint32_t {
                                   return total + getPlaneSize(planeInfo, pixelWidth, pixelHeight);
                               });
    }
    return size;
}

uint32_t getBufferSizeUsed(int fmt, uint32_t pixelWidth, uint32_t pixelHeight) {
    uint32_t size = 0;
    if (fmt == V4L2_PIX_FMT_QC08C) {
        auto& layoutInfo = getBufferLayoutInfo(fmt);
        size = std::accumulate(layoutInfo.begin(), layoutInfo.end(), 0,
                               [&](uint32_t total, auto& planeInfo) -> uint32_t {
                                   return total + getPlaneSize(planeInfo, pixelWidth, pixelHeight);
                               });
    } else if (fmt == V4L2_PIX_FMT_QC10C) {
        size = getBufferSize(fmt, pixelWidth, pixelHeight);
    }
    return size;
}
