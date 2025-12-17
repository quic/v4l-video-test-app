cmake_minimum_required(VERSION 3.5)

project(iris_v4l2_test)

# Compiler flags
set(CMAKE_CXX_STANDARD 17)

# Build debug version
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    message(STATUS "Debug mode")
    add_definitions(-DDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O0 -g -rdynamic -Wl,--no-as-needed")
else()
    message(STATUS "Release mode")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O2 -Wl,--no-as-needed")
endif()

find_package(jsoncpp REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(AVCODEC REQUIRED IMPORTED_TARGET GLOBAL libavcodec libavutil libavformat)


# Include directories
set(VIDC_TEST_INCLUDES
    inc/
)
include_directories(${VIDC_TEST_INCLUDES})

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
target_link_libraries(iris_v4l2_test PkgConfig::AVCODEC pthread jsoncpp_lib)
