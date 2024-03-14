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

#define LOG(format, args...) printf(" [%s] " format, this->id().c_str(), ##args)

#endif  //_LOG_H_
