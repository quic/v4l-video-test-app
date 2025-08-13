cmake_minimum_required(VERSION 3.5)

project(iris_v4l2_test)

# Compiler flags
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_EXE_LINKER_FLAGS "-static")
set(CMAKE_EXECUTABLE_ENABLE_EXPORTS TRUE)
set(CMAKE_C_COMPILER /usr/aarch64-linux-gnu/bin/aarch64-none-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /usr/aarch64-linux-gnu/bin/aarch64-none-linux-gnu-g++)

# Build debug version
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    message(STATUS "Debug mode")
    add_definitions(-DDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O0 -g -rdynamic -Wl,--no-as-needed")
else()
    message(STATUS "Release mode")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O2 -Wl,--no-as-needed")
endif()


# Include directories
set(VIDC_TEST_INCLUDES
    inc/
    third-parties/jsoncpp/output/include
    third-parties/ffmpeg/output/include
)
include_directories(${VIDC_TEST_INCLUDES})
link_directories(
    third-parties/jsoncpp/output/lib
    third-parties/ffmpeg/output/lib
)

set(VIDC_TEST_SOURCES
    IrisTestApp.cpp
    src/ConfigParser.cpp
    src/FFStreamParser.cpp
    src/FFYUVParser.cpp
    src/UBWC_Utils.cpp
    src/V4l2Driver.cpp
    src/V4l2Codec.cpp
    src/V4l2Decoder.cpp
    src/V4l2Encoder.cpp
)

add_executable(iris_v4l2_test ${VIDC_TEST_SOURCES})
target_link_libraries(iris_v4l2_test avformat avcodec swresample avutil pthread jsoncpp)
