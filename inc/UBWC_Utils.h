/*
 **************************************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 **************************************************************************************************
*/

#ifndef _UBWC_UTILS_H_
#define _UBWC_UTILS_H_

#include <stdint.h>

#include <array>
#include <unordered_map>

#define PLANE_MAXSIZE 4

struct PlaneInfo {
    bool valid = false;
    struct {
        uint32_t pixelAlignment = 1;  // must not be 0
        uint32_t bytesPerPixel_Num = 1;
        uint32_t bytesPerPixel_Den = 1;  // must not be 0
        uint32_t tile = 1;
        uint32_t alignment = 1;  // must not be 0
    } strideInfo{}, scanlinesInfo{};
};

using BufferLayoutInfo = std::array<PlaneInfo, PLANE_MAXSIZE>;

const BufferLayoutInfo& getBufferLayoutInfo(int fmt);

uint32_t getPlaneSize(const PlaneInfo& planeInfo, uint32_t w, uint32_t h);

uint32_t getBufferSize(int fmt, uint32_t pixelWidth, uint32_t pixelHeight);

uint32_t getBufferSizeUsed(int fmt, uint32_t pixelWidth, uint32_t pixelHeight);

#endif  // !_UBWC_UTILS_H_
