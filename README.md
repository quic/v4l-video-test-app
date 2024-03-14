# V4l2 Video Test Application

## 1. What Is This

This is a simple video test application which uses Linux V4l2 interfaces to
simulate several basic encoder and decoder behaviors.

The main purpose of this test application is to test the compatibility and
validity of video drivers on Unix-like operating systems.

## 2. How To Build This Project

This project is not a standalone project, but uses several dependencies.:

Few Opensource dependencies are :

1. FFMpeg - used as input demuxer.
2. JsonCpp -  used as configuration parser.

### 2.1 Setup Development Environment

To compile this project we need to setup the cross compiling environment for ARM architecture.
This project was written in Ubuntu 20.04-WSL (Linux Kernel newer or equal to 6.2(6.6 recommended)).

1. Install ARM Cross-compiler toolchain : https://developer.arm.com/downloads/-/gnu-a
   Recommended Compiler : gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu.tar.xz

2. Install the latest CMake for compilation of project.

```bash
sudo apt-get -y install cmake
```

3. Follow the below commands to setup your compiler

```bash
# Unzip tool in /local/
tar -xvf ./gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu.tar.xz

cd gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu

# Create toolchain directory
sudo mkdir -p /usr/aarch64-linux-gnu

# Copy toolchain files to dst directory
sudo cp -r ./* /usr/aarch64-linux-gnu

# Add paths for the environment
export PATH="$PATH:/usr/aarch64-linux-gnu/bin"
export ARCH=arm64
export CROSS_COMPILE=/local/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/gcc-arm-10.2-2020.11-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
export PATH="$PATH:/pkg/asw/compilers/gnu/linaro-toolchain/5.1/bin/"

# To check if the compiler is properly set use the below commands
aarch64-none-linux-gnu-gcc -v
aarch64-none-linux-gnu-g++ -v

# Setup the include file for linux headers
cd /usr/aarch64-linux-gnu/aarch64-none-linux-gnu/libc/usr/include
sudo mv linux linux_bak
sudo ln -s /usr/include/linux ./linux

```

### 2.2 Install and compile the dependent projects

Run the script "prepare_3p_module.sh" present in "v4l_video_test_app/third-parties"
folder to clone, compile and install the shared libraries and include file.

The install location of output headers & libs can be modified in this script.
But please remeber to update the top-level CMakeLists.txt to make sure that they can be found.

```bash
cd v4l_video_test_app/third-parties
chmod +x prepare_3p_module.sh
./prepare_3p_module.sh
```

### 2.3 Build & Test The Project

Once the environment is set and dependent projects are install we are ready for compiling our project.

Run script "build.sh" to build the project.
This would build the project into an executable file "iris_v4l2_test" in "v4l_video_test_app/build" folder.

```bash
cd v4l_video_test_app/
chmod +x build.sh
./build.sh
cd build/
chmod +x iris_v4l2_test
```

Upload this executable file and data folder to the device (using "adb push ..." or some other methods according to your device and platform).

```bash
./iris_v4l2_test --help
./iris_v4l2_test -e -c ./data/config/h264Encoder.json
./iris_v4l2_test -d -c ./data/config/h264Decoder.json
```

## 3. Tags Table

| S.No. | Tag name               | Tag Description                                                | Type           | Possible values                | Mandatory / Optional       |
|:-----:|:----------------------:|:--------------------------------------------------------------:|:--------------:|:------------------------------:|:--------------------------:|
| 1     | "ExecutionMode"        | Execution Mode in case of Multiple testcases execution         | String         | "Sequential" / "Concurrent"    | Optional                   |
|       |                        |                                                                |                |                                |                            |
| 2     | "TestCases"            | Array of testcases we need to execute                          | Array          | -                              | Mandatory                  |
|       |                        |                                                                |                |                                |                            |
| 3     | "Name"                 | Unique testcase name                                           | String         | -                              | Mandatory                  |
|       |                        |                                                                |                |                                |                            |
| 4     | "TestConfigs"          | Mention the configs required for running the tests             | Array          | -                              | Mandatory                  |
|       |                        |                                                                |                |                                |                            |
| 5     | "Domain"               | Test type                                                      | String         | "Decoder" / "Encoder"          | Mandatory                  |
|       |                        |                                                                |                |                                |                            |
| 6     | "InputPath"            | Absolute file path of Input Bitstream                          | String         | Any accessable path in device  | Mandatory                  |
|       |                        |                                                                |                |                                |                            |
| 7     | "NumFrames"            | Number of frames to be executed                                | Integer        | -1 (All frames) / Non-Zero     | Mandatory                  |
|       |                        |                                                                |                |                                |                            |
| 8     | "Width"                | Width of Input bitstream                                       | Integer        | Actual Width of the Input      | Mandatory                  |
|       |                        |                                                                |                |                                |                            |
| 9     | "Height"               | Height of Input bitstream                                      | Integer        | Actual Height of the Input     | Mandatory                  |
|       |                        |                                                                |                |                                |                            |
| 10    | "CodecName"            | Codec of Input bitstream for Decoder  Testcasse                | String         | "HEVC" / "AVC" / "VP9"         | Mandatory                  |
|       |                        |                                                                |                |                                |                            |
| 11    | "PixelFormat"          | PixelFormat of Input bitstream for Encoder Testcase            | String         | "NV12" / "QC08C" / "QC10C"     | Mandatory                  |
|       |                        |                                                                |                |                                |                            |
| 12    | "OperatingRate"        | Operating Rate for Encoder testcsases                          | Integer        | Default: 30 |  Max: 240        | Mandatory                  |
|       |                        |                                                                |                |                                |                            |
| 13    | "FrameRate"            | Frame rate to Encode the bitstream                             | Integer        | Default: 30 |  Max: 240        | Mandatory                  |
|       |                        |                                                                |                |                                |                            |
| 14    | "StaticControls"       | Configurations to be set during initialization of the VPU      | Array          | Values from Controls Table     | Mandatory                  |
|       |                        |                                                                |                |                                |                            |
| 15    | "DynamicControls"      | Dynamic Configurations to be set to VPU during                 |                |                                |                            |
|       |                        | testcase execution. (Only applicable for Encoders)             | Array          | Values from Controls Table     | Optional                   |
|       |                        |                                                                |                |                                |                            |
| 16    | "Outputpath"           | Absolute file path of Output Dump file                         | String         | Any accessable path in device  | Optional                   |
|       |                        |                                                                |                |                                |                            |
| 17    | "DumpInputPath"        | Absolute file path of Input Dump file                          | String         | Any accessable path in device  | Optional                   |
|       |                        |                                                                |                |                                |                            |
| 18    | "InputBufferCount"     | Max Number of Input Buffers to circulate for test execution    | Integer        | Dec: [1,16] Enc: [1,32]        | Optional                   |
|       |                        |                                                                |                |                                |                            |
| 19    | "OutputBufferCount"    | Max Number of Output Buffers to circulate for test execution   | Integer        | Dec: [1,16] Enc: [1,32]        | Optional                   |

## 4. Controls Table

| S.No. | Static/Dynamic Control | Id                          | Vtype          | Value                                                              | Mandatory / Optional      |
|:-----:|:----------------------:|:---------------------------:|:--------------:|:------------------------------------------------------------------:|:-------------------------:|
| 1     | Static Control         | "Profile"                   | "String"       | AVC Profiles: {"BASELINE", "MAIN", "HIGH"}                         | Mandatory                 |
|       |                        |                             |                | HEVC Profiles: {"MAIN", "MAIN10", "MAIN_SP"}                       |                           |
|       |                        |                             |                |                                                                    |                           |
| 2     | Static Control         | "Level"                     | "String"       | AVC Levels:                                                        |                           |
|       |                        |                             |                | {"1.0", "1B", "1.1", "1.2", "1.3", "2.0", "2.1", "2.2",            | Mandatory                 |
|       |                        |                             |                | "3.0", "3.1", "3.2", "4.0", "4.1", "4.2", "5.0", "5.1"}            |                           |
|       |                        |                             |                | HEVC Levels:                                                       |                           |
|       |                        |                             |                | {"1.0", "2.0", "2.1", "3.0", "3.1", "4.0", "4.1", "5.0"}           |                           |
|       |                        |                             |                |                                                                    |                           |
| 3     | Static/Dynamic Control | "BitRate"                   | "Int"          | [1, 100000000]                                                     | Mandatory                 |
|       |                        |                             |                |                                                                    |                           |
| 4     | Static Control         | "BitRateMode"               | "String"       | {"VBR", "CBR", "MBR"}                                              | Mandatory                 |
|       |                        |                             |                |                                                                    |                           |
| 5     | Static Control         | "PrefixHeaderMode"          | "String"       | {"JOINED", "SEPARATE"}                                             | Mandatory                 |
|       |                        |                             |                |                                                                    |                           |
| 6     | Static Control         | "Tier"                      | "String"       | {"MAIN", "HIGH"}                                                   | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 7     | Static Control         | "MinQP"                     | "Int"          | Minimum quantization parameter for HEVC.                           |                           |
|       |                        |                             |                | Valid range: [0, 51] for 8 bit and [0 to 63] for 10 bit.           | Optional                  |
|       |                        |                             |                | Minimum quantization parameter for H264.                           |                           |
|       |                        |                             |                | Valid range: [0 to 51]                                             |                           |
|       |                        |                             |                |                                                                    |                           |
| 8     | Static Control         | "MaxQP"                     | "Int"          | Maximum quantization parameter for HEVC.                           |                           |
|       |                        |                             |                | Valid range: from 0 to 51 for 8 bit and from 0 to 63 for 10 bit.   |                           |
|       |                        |                             |                | Maximum quantization parameter for H264. Valid range: from 0 to 51 | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 9     | Static/Dynamic Control | "IQP"                       | "Int"          | Quantization parameter for an I frame.                             |                           |
|       |                        |                             |                | Valid range: [MinQP, MaxQP].                                       | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 10    | Static/Dynamic Control | "PQP"                       | "Int"          | Quantization parameter for an P frame.                             |                           |
|       |                        |                             |                | Valid range: [MinQP, MaxQP].                                       | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 11    | Static/Dynamic Control | "BQP"                       | "Int"          | Quantization parameter for an B frame.                             |                           |
|       |                        |                             |                | Valid range: [MinQP, MaxQP].                                       | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 12    | Static Control         | "MaxIQP"                    | "Int"          | Max quantization parameter to limit I frame quality to a range.    |                           |
|       |                        |                             |                | Valid range: [MinQP, MaxQP].                                       | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 13    | Static Control         | "MaxPQP"                    | "Int"          | Max quantization parameter to limit P frame quality to a range.    |                           |
|       |                        |                             |                | Valid range: [MinQP, MaxQP].                                       | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 14    | Static Control         | "MaxBQP"                    | "Int"          | Max quantization parameter to limit B frame quality to a range.    |                           |
|       |                        |                             |                | Valid range: [MinQP, MaxQP].                                       | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 15    | Static Control         | "MinIQP"                    | "Int"          | Min quantization parameter to limit I frame quality to a range.    |                           |
|       |                        |                             |                | Valid range: [MinQP, MaxQP].                                       | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 16    | Static Control         | "MinPQP"                    | "Int"          | Min quantization parameter to limit P frame quality to a range.    |                           |
|       |                        |                             |                | Valid range: [MinQP, MaxQP].                                       | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 17    | Static Control         | "MinBQP"                    | "Int"          | Min quantization parameter to limit B frame quality to a range.    |                           |
|       |                        |                             |                | Valid range: [MinQP, MaxQP].                                       | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 18    | Static Control         | "EntropyCoding"             | "String"       | {"CABAC", "CAVLC"}                                                 | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 19    | Static Control         | "Transform8x8"              | "Int"          | Applicable to the H264 encoder. Valid range: [0, 1]                | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 20    | Static/Dynamic Control | "GOPSize"                   | "Int"          | [0, INT_MAX]                                                       | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 21    | Static Control         | "BFrames"                   | "Int"          | [0, 1]                                                             | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 22    | Static Control         | "PrependPsToIDR"            | "Int"          | [0, 1]                                                             | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 23    | Static Control         | "FrameRC"                   | "Int"          | [0, 1]                                                             | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 24    | Static Control         | "Rotate"                    | "Int"          | {0, 90, 180, 270}                                                  | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 25    | Static/Dynamic Control | "VFlip"                     | "Int"          | [0, 1]                                                             | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 26    | Static/Dynamic Control | "HFlip"                     | "Int"          | [0, 1]                                                             | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 27    | Static Control         | "MultiSliceMode"            | "String"       | {"SINGLE", "MAX_MB", "MAX_BYTES"}                                  | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 28    | Static Control         | "MaxMacroBlocksPerSlice"    | "Int"          | [1, 36864]                                                         | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 29    | StaticControl          | "MaxBytesPerSlice"          | "Int"          | [512, 12500000]                                                    | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 30    | Static/Dynamic Control | "IntraRefreshPeriod"        | "Int"          | [0, INT_MAX]                                                       | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 31    | Static Control         | "IntraRefreshType"          | "String"       | {"CYCLIC", "RANDOM"}                                               | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 32    | Static Control         | "LTRCount"                  | "Int"          | [0, 2]                                                             | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 33    | Static/Dynamic Control | "UseLTR"                    | "Int"          | [0, LTRCount - 1]                                                  | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 34    | Static/Dynamic Control | "LTRIndex"                  | "Int"          | [1, LTRCount]                                                      | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 35    | Static Control         | "HierarchicalCoding"        | "Int"          | [0, 1]                                                             | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 36    | Static Control         | "HierarchicalCodingType"    | "String"       | {"HierP", "HierB"}                                                 | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 37    | Static/Dynamic Control | "HierarchicalLayerCount"    | "Int"          | [0, 5]                                                             | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 38    | Static/Dynamic Control | "HierCodingL0BR"            | "Int"          | [1, 100000000]                                                     | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 39    | Static/Dynamic Control | "HierCodingL1BR"            | "Int"          | [1, 100000000]                                                     | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 40    | Static/Dynamic Control | "HierCodingL2BR"            | "Int"          | [1, 100000000]                                                     | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 41    | Static/Dynamic Control | "HierCodingL3BR"            | "Int"          | [1, 100000000]                                                     | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 42    | Static/Dynamic Control | "HierCodingL4BR"            | "Int"          | [1, 100000000]                                                     | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 43    | Static/Dynamic Control | "HierCodingL5BR"            | "Int"          | [1, 100000000]                                                     | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 44    | Static/Dynamic Control | "PeakBitrate"               | "Int"          | [1, 100000000]                                                     | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 45    | Static/Dynamic Control | "InjectKeyFrame"            | "Int"          | Any Frame Number                                                   | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 46    | Static Control         | "BasePriorityID"            | "Int"          | [0, 63]                                                            | Optional                  |

## 5. Licence

Project is licensed under the BSD-3-Clause-Clear License. See LICENSE.txt for the full license text.
