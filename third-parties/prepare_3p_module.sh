#
 #**************************************************************************************************
 # Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 # SPDX-License-Identifier: BSD-3-Clause-Clear
 #**************************************************************************************************
#

#! /bin/bash

# jsoncpp
rm -rf ./jsoncpp
git clone https://github.com/open-source-parsers/jsoncpp.git
cd jsoncpp
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../../aarch64_toolchain.cmake -DCMAKE_INSTALL_PREFIX=../output -DJSONCPP_WITH_TESTS=OFF -DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF
make && make install
cd ../../

# FFmpeg
rm -rf ./ffmpeg
git clone https://git.ffmpeg.org/ffmpeg.git ffmpeg
cd ffmpeg
./configure --enable-shared --enable-static --prefix=./output --cross-prefix=aarch64-linux-gnu- --arch=arm64 --target-os=linux --enable-ffmpeg
make && make install
