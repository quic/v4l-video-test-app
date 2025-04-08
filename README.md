# V4L Video Test Application

## 1. What Is This ?

This is a simple video test application which uses Linux V4L interfaces to simulate several basic Encoder and Decoder behaviors. The main purpose of this test application is to test the compatibility and validity of video drivers on Unix-like operating systems.

## 2. How To Build This Project ?

This project requires few opensource dependencies for compilation and execution :

1. [**FFMpeg**](https://git.ffmpeg.org/ffmpeg.git) - used as input demuxer.
2. [**JsonCpp**](https://github.com/open-source-parsers/jsoncpp) -  used as configuration parser.

### 2.1. Setup Development Environment

To compile this project we need to setup the cross compiling environment for ARM architecture. Here we are using Ubuntu 22.04 as an example:

Firstly, make sure you have setup the commonly-used development tools using the following commands:

```bash
sudo apt update
sudo apt upgrade
sudo apt install autoconf automake build-essential cmake git wget yasm # you may install more packages if you need them.
```

Secondly, search the toolchain version that the system can support using the following command:

```bash
sudo apt search aarch64-linux-gnu
```

You may see the following results of the searching:
```
g++-10-aarch64-linux-gnu/jammy-updates,jammy-security 10.5.0-1ubuntu1~22.04cross1 amd64
  GNU C++ compiler (cross compiler for arm64 architecture)

g++-11-aarch64-linux-gnu/jammy-updates,jammy-security,now 11.4.0-1ubuntu1~22.04cross1 amd64
  GNU C++ compiler (cross compiler for arm64 architecture)

g++-12-aarch64-linux-gnu/jammy-updates,jammy-security 12.3.0-1ubuntu1~22.04cross1 amd64
  GNU C++ compiler (cross compiler for arm64 architecture)

g++-9-aarch64-linux-gnu/jammy-updates,jammy-security 9.5.0-1ubuntu1~22.04cross1 amd64
  GNU C++ compiler (cross compiler for arm64 architecture)

g++-aarch64-linux-gnu/jammy,now 4:11.2.0-1ubuntu1 amd64 [installed]
  GNU C++ compiler for the arm64 architecture

gcc-10-aarch64-linux-gnu/jammy-updates,jammy-security 10.5.0-1ubuntu1~22.04cross1 amd64
  GNU C compiler (cross compiler for arm64 architecture)

gcc-10-aarch64-linux-gnu-base/jammy-updates,jammy-security 10.5.0-1ubuntu1~22.04cross1 amd64
  GCC, the GNU Compiler Collection (base package)

gcc-10-plugin-dev-aarch64-linux-gnu/jammy-updates,jammy-security 10.5.0-1ubuntu1~22.04cross1 amd64
  Files for GNU GCC plugin development.

gcc-11-aarch64-linux-gnu/jammy-updates,jammy-security,now 11.4.0-1ubuntu1~22.04cross1 amd64
  GNU C compiler (cross compiler for arm64 architecture)

gcc-11-aarch64-linux-gnu-base/jammy-updates,jammy-security,now 11.4.0-1ubuntu1~22.04cross1 amd64
  GCC, the GNU Compiler Collection (base package)

gcc-11-plugin-dev-aarch64-linux-gnu/jammy-updates,jammy-security 11.4.0-1ubuntu1~22.04cross1 amd64
  Files for GNU GCC plugin development.

gcc-12-aarch64-linux-gnu/jammy-updates,jammy-security 12.3.0-1ubuntu1~22.04cross1 amd64
  GNU C compiler (cross compiler for arm64 architecture)

gcc-12-aarch64-linux-gnu-base/jammy-updates,jammy-security 12.3.0-1ubuntu1~22.04cross1 amd64
  GCC, the GNU Compiler Collection (base package)

gcc-12-plugin-dev-aarch64-linux-gnu/jammy-updates,jammy-security 12.3.0-1ubuntu1~22.04cross1 amd64
  Files for GNU GCC plugin development.

gcc-9-aarch64-linux-gnu/jammy-updates,jammy-security 9.5.0-1ubuntu1~22.04cross1 amd64
  GNU C compiler (cross compiler for arm64 architecture)

gcc-9-aarch64-linux-gnu-base/jammy-updates,jammy-security 9.5.0-1ubuntu1~22.04cross1 amd64
  GCC, the GNU Compiler Collection (base package)

gcc-9-plugin-dev-aarch64-linux-gnu/jammy-updates,jammy-security 9.5.0-1ubuntu1~22.04cross1 amd64
  Files for GNU GCC plugin development.

gcc-aarch64-linux-gnu/jammy,now 4:11.2.0-1ubuntu1 amd64 [installed]
  GNU C compiler for the arm64 architecture
```

Thirdly, using the following command to install aarch64 cross-compiling toolchain:

```bash
sudo apt install gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu g++-aarch64-linux-gnu
```

You are free to use other version of gcc/g++ if you have any perticular requirements.

After the installation completes, you may using the following commands to check if the installation succeeded:

```bash
aarch64-linux-gnu-gcc --version
aarch64-linux-gnu-g++ --version
```

If the commands return the version number of gcc/g++, then the installation completed.

### 2.2 Install and compile the dependent projects

Run the script [**prepare_3p_module.sh**](https://github.com/quic/v4l-video-test-app/blob/master/third-parties/prepare_3p_module.sh) present in [**v4l_video_test_app/third-parties**](https://github.com/quic/v4l-video-test-app/tree/master/third-parties) folder to clone, compile and install the shared libraries and include file.

##### **NOTE: The install location of output headers & libs can be modified in this script. Update the top-level [**CMakeLists.txt**](https://github.com/quic/v4l-video-test-app/blob/master/CMakeLists.txt) to make sure that they can be found.**

#### 2.2.1. Follow the below commands to compile the third party dependencies
##### Update compiler path according to your environment
You should check where your compilers locate. If you use the previous commands above, you may see them in the following location with the following command:
```bash
$ which aarch64-linux-gnu-gcc
/usr/bin/aarch64-linux-gnu-gcc
$ which aarch64-linux-gnu-g++
/usr/bin/aarch64-linux-gnu-g++
```
**NOTE: The compiler path in aarch64_toolchain.cmake should be set as these paths.**

**NOTE: The same path should be set to CMAKE_C_COMPILER and CMAKE_CXX_COMPILER in project CMakelists.txt.**

##### Give prepare_3p_module.sh executable rights and run the scripts
```bash
cd v4l-video-test-app/third-parties
chmod +x prepare_3p_module.sh
./prepare_3p_module.sh
```

### 2.3. Build & Test The Project

Once the environment is set and dependent projects are install we are ready for compiling our project.

Run script [**build.sh**](https://github.com/quic/v4l-video-test-app/blob/master/build.sh) to build the project. This would build the project into an executable file **"iris_v4l2_test"** in "v4l_video_test_app/build" folder.

#### 2.3.1. Follow the below commands to compile the executable

##### Give build.sh executable rights and run the scripts
```bash
cd v4l-video-test-app/
chmod +x build.sh
./build.sh
```
##### Give newly created binary iris_v4l2_test executable rights
```bash
cd build/
chmod +x iris_v4l2_test
```

##### Upload this executable file and data folder to the device *(using "adb push ..." or some other methods according to your device and platform)*.

#### 2.3.2. Follow the below commands to run the tests

##### This commands helps in understanding the requirement and options for running the test
```bash
./iris_v4l2_test --help
```

##### Command to run the Encoder testcase.
```bash
./iris_v4l2_test --config ./data/config/h264Encoder.json
```
##### Command to run the Decoder testcase.
```bash
./iris_v4l2_test --config ./data/config/h264Decoder.json
```

##### Command to run the testcase with custom log level. Range: [0, 16]
```bash
./iris_v4l2_test --loglevel 12 --config ./data/config/h264Decoder.json
```

## 3. Tags Table

This table specify the valid set of tags and it's possible value for creation of the JSON file, which is used as a config file to run the test.


| S.No. | Tag name               | Tag Description                                                | Type           | Possible values                | Mandatory / Optional       |
|:-----:|:----------------------:|:--------------------------------------------------------------:|:--------------:|:------------------------------:|:--------------------------:|
|       |                        |                                                                |                |                                |                            |
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
| 15    | "DynamicControls"      | Dynamic Configurations to be set to VPU during testcase execution. (Only applicable for Encoders) | Array | Values from Controls Table | Optional |
|       |                        |                                                                |                |                                |                            |
| 16    | "Outputpath"           | Absolute file path of Output Dump file                         | String         | Any accessable path in device  | Optional                   |
|       |                        |                                                                |                |                                |                            |
| 17    | "DumpInputPath"        | Absolute file path of Input Dump file                          | String         | Any accessable path in device  | Optional                   |
|       |                        |                                                                |                |                                |                            |
| 18    | "InputBufferCount"     | Max Number of Input Buffers to circulate for test execution    | Integer        | Dec: [1,16] Enc: [1,32]        | Optional                   |
|       |                        |                                                                |                |                                |                            |
| 19    | "OutputBufferCount"    | Max Number of Output Buffers to circulate for test execution   | Integer        | Dec: [1,16] Enc: [1,32]        | Optional                   |
|       |                        |                                                                |                |                                |                            |

## 4. Controls Table
This table specify the vaild controls which can be used and their possible value to run an Encoder test. These controls are given as StaticControls or DynamicControls in JSON config file.


| S.No. | Static/Dynamic Control | Id                          | Vtype          | Value                                                              | Mandatory / Optional      |
|:-----:|:----------------------:|:---------------------------:|:--------------:|:------------------------------------------------------------------:|:-------------------------:|
|       |                        |                                                                |                |                                |                            |
| 1     | Static Control         | "Profile"                   | "String"       | **AVC Profiles:** {"BASELINE", "MAIN", "HIGH"} **HEVC Profiles:** {"MAIN", "MAIN10", "MAIN_SP"}| Mandatory |
|       |                        |                             |                |                                                                    |                           |
| 2     | Static Control         | "Level"                     | "String"       | **AVC Levels:** {"1.0", "1B", "1.1", "1.2", "1.3", "2.0", "2.1", "2.2", "3.0", "3.1", "3.2", "4.0", "4.1", "4.2", "5.0", "5.1"}     **HEVC Levels:** {"1.0", "2.0", "2.1", "3.0", "3.1", "4.0", "4.1", "5.0"} | Mandatory                 |
|       |                        |                             |                |                                                                    |                           |
| 3     | Static/Dynamic Control | "BitRate"                   | "Int"          | [1, 100000000]                                                     | Mandatory                 |
|       |                        |                             |                |                                                                    |                           |
| 4     | Static Control         | "BitRateMode"               | "String"       | {"VBR", "CBR", "MBR"}                                              | Mandatory                 |
|       |                        |                             |                |                                                                    |                           |
| 5     | Static Control         | "PrefixHeaderMode"          | "String"       | {"JOINED", "SEPARATE"}                                             | Mandatory                 |
|       |                        |                             |                |                                                                    |                           |
| 6     | Static Control         | "Tier"                      | "String"       | {"MAIN", "HIGH"}                                                   | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 7     | Static Control         | "MinQP"                     | "Int"          | Minimum quantization parameter for HEVC. Valid range: [0, 51] for 8 bit and [0 to 63] for 10 bit. Minimum quantization parameter for H264. Valid range: [0 to 51] | Optional                    |
|       |                        |                             |                |                                                                    |                           |
| 8     | Static Control         | "MaxQP"                     | "Int"          | Maximum quantization parameter for HEVC. Valid range: [0, 51] for 8 bit and [0 to 63] for 10 bit. Maximum quantization parameter for H264. Valid range: [0 to 51] | Optional                    |
|       |                        |                             |                |                                                                    |                           |
| 9     | Static/Dynamic Control | "IQP"                       | "Int"          | Quantization parameter for an I frame. Valid range: [MinQP, MaxQP] | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 10    | Static/Dynamic Control | "PQP"                       | "Int"          | Quantization parameter for an P frame. Valid range: [MinQP, MaxQP] | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 11    | Static/Dynamic Control | "BQP"                       | "Int"          | Quantization parameter for an B frame. Valid range: [MinQP, MaxQP] | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 12    | Static Control         | "MaxIQP"                    | "Int"          | Max quantization parameter to limit I frame quality to a range. Valid range: [MinQP, MaxQP]    | Optional  |
|       |                        |                             |                |                                                                    |                           |
| 13    | Static Control         | "MaxPQP"                    | "Int"          | Max quantization parameter to limit P frame quality to a range. Valid range: [MinQP, MaxQP]    | Optional  |
|       |                        |                             |                |                                                                    |                           |
| 14    | Static Control         | "MaxBQP"                    | "Int"          | Max quantization parameter to limit B frame quality to a range. Valid range: [MinQP, MaxQP]    | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 15    | Static Control         | "MinIQP"                    | "Int"          | Min quantization parameter to limit I frame quality to a range. Valid range: [MinQP, MaxQP]    | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 16    | Static Control         | "MinPQP"                    | "Int"          | Min quantization parameter to limit P frame quality to a range. Valid range: [MinQP, MaxQP]    | Optional                  |
|       |                        |                             |                |                                                                    |                           |
| 17    | Static Control         | "MinBQP"                    | "Int"          | Min quantization parameter to limit B frame quality to a range. Valid range: [MinQP, MaxQP]    | Optional                  |
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
|       |                        |                             |                |                                                                    |                           |

## 5. License

Project is licensed under the **BSD-3-Clause-Clear License**. See [LICENSE.txt](https://github.com/quic/v4l-video-test-app/blob/master/LICENSE.txt) for the full license text.

