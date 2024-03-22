/*
 **************************************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 **************************************************************************************************
*/

#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <iostream>

extern uint32_t gLogLevel;

/**
 *
 *  controls logging debug messages
 *
 *  ....................................................
 *   level   LOGV     LOGD     LOGI     LOGW     LOGE
 *  ....................................................
 *    0x0   silent   silent   slient   slient   slient
 *    0x1   silent   slient   slient   slient   printed
 *    0x3   slient   slient   slient   printed  printed
 *    0x7   slient   slient   printed  printed  printed
 *    0xF   slient   printed  printed  printed  printed
 *    0x1F  printed  printed  printed  printed  printed
 *  .....................................................
 *
 */
enum LogLevel : uint32_t {
    LOG_MSGLEVEL_ERROR    = 0x01,    ///< error logs
    LOG_MSGLEVEL_WARN     = 0x02,    ///< warning logs
    LOG_MSGLEVEL_INFO     = 0x04,    ///< info logs
    LOG_MSGLEVEL_DEBUG    = 0x08,    ///< debug logs
    LOG_MSGLEVEL_VERBOSE  = 0x10,    ///< verbose logs
};


#define LOGV(format, args...)                               \
    if (gLogLevel & LOG_MSGLEVEL_VERBOSE) {                 \
        printf("[V] [%s] " format, this->id().c_str(), ##args); \
    }

#define LOGD(format, args...)                               \
    if (gLogLevel & LOG_MSGLEVEL_DEBUG) {                 \
        printf("[D] [%s] " format, this->id().c_str(), ##args); \
    }

#define LOGI(format, args...)                               \
    if (gLogLevel & LOG_MSGLEVEL_INFO) {                 \
        printf("[I] [%s] " format, this->id().c_str(), ##args); \
    }

#define LOGW(format, args...)                               \
    if (gLogLevel & LOG_MSGLEVEL_WARN) {                 \
        printf("[W] [%s] " format, this->id().c_str(), ##args); \
    }

#define LOGE(format, args...)                               \
    if (gLogLevel & LOG_MSGLEVEL_ERROR) {                 \
        printf("[E] [%s] " format, this->id().c_str(), ##args); \
    }

#endif  //_LOG_H_
