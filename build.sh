#
 #**************************************************************************************************
 # Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 # SPDX-License-Identifier: BSD-3-Clause-Clear
 #**************************************************************************************************
#

#! /bin/bash
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
