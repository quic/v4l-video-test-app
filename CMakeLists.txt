cmake_minimum_required(VERSION 3.13)

# Optional override:
#   cmake -DIRIS_FORCE_NATIVE=ON ...
option(IRIS_FORCE_NATIVE "Force native build even on x86_64 host" OFF)

# Auto-select cross toolchain on x86_64 Linux host if available.
# This must happen BEFORE project().
if(NOT IRIS_FORCE_NATIVE AND NOT DEFINED CMAKE_TOOLCHAIN_FILE AND CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    cmake_host_system_information(RESULT IRIS_HOST_ARCH QUERY OS_PLATFORM)
    string(TOLOWER "${IRIS_HOST_ARCH}" IRIS_HOST_ARCH)
    message(STATUS "Host architecture: ${IRIS_HOST_ARCH}")

    set(IRIS_CROSS_C   "/usr/bin/aarch64-linux-gnu-gcc")
    set(IRIS_CROSS_CXX "/usr/bin/aarch64-linux-gnu-g++")
    set(IRIS_TOOLCHAIN "${CMAKE_CURRENT_LIST_DIR}/third-parties/aarch64_toolchain.cmake")

    if(IRIS_HOST_ARCH MATCHES "^(x86_64|amd64)$"
       AND EXISTS "${IRIS_CROSS_C}"
       AND EXISTS "${IRIS_CROSS_CXX}"
       AND EXISTS "${IRIS_TOOLCHAIN}")
        set(CMAKE_TOOLCHAIN_FILE "${IRIS_TOOLCHAIN}" CACHE FILEPATH "Cross toolchain file")
        message(STATUS "x86_64 host detected -> using aarch64 cross toolchain")
    else()
        message(STATUS "Using native toolchain")
    endif()
endif()

project(iris_v4l2_test C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug mode")
    add_definitions(-DDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")
else()
    message(STATUS "Release mode")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

set(CMAKE_EXE_LINKER_FLAGS "-static")

# Includes
include_directories(
    inc/
)

set(VIDC_TEST_SOURCES
    IrisTestApp.cpp
    src/ConfigParser.cpp
    src/FFStreamParser.cpp
    src/FFYUVParser.cpp
    src/Trace.cpp
    src/UBWC_Utils.cpp
    src/V4l2Driver.cpp
    src/V4l2Codec.cpp
    src/V4l2Decoder.cpp
    src/V4l2Encoder.cpp
)

add_executable(iris_v4l2_test ${VIDC_TEST_SOURCES})

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(iris_v4l2_test PRIVATE -fno-omit-frame-pointer)
    target_link_options(iris_v4l2_test PRIVATE -rdynamic)
endif()

set_target_properties(iris_v4l2_test PROPERTIES
    BUILD_RPATH "\$ORIGIN/lib"
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "\$ORIGIN/lib"
    INSTALL_RPATH_USE_LINK_PATH FALSE
)

find_package(Threads REQUIRED)

if(CMAKE_CROSSCOMPILING)
    message(STATUS "Cross-compiling for ${CMAKE_SYSTEM_PROCESSOR}")

    set(IRIS_3P_LIB_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/third-parties/jsoncpp/output/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/third-parties/ffmpeg/output/lib
    )

    set(IRIS_3P_STATIC_LIBS
        ${CMAKE_CURRENT_SOURCE_DIR}/third-parties/ffmpeg/output/lib/libavformat.a
        ${CMAKE_CURRENT_SOURCE_DIR}/third-parties/ffmpeg/output/lib/libavcodec.a
        ${CMAKE_CURRENT_SOURCE_DIR}/third-parties/ffmpeg/output/lib/libswresample.a
        ${CMAKE_CURRENT_SOURCE_DIR}/third-parties/ffmpeg/output/lib/libavutil.a
        ${CMAKE_CURRENT_SOURCE_DIR}/third-parties/jsoncpp/output/lib/libjsoncpp.a
    )

    target_include_directories(iris_v4l2_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third-parties/jsoncpp/output/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third-parties/ffmpeg/output/include
    )

    target_link_libraries(iris_v4l2_test
        ${IRIS_3P_STATIC_LIBS}
        Threads::Threads
        ${CMAKE_DL_LIBS}
    )

else()
    message(STATUS "Native build on ${CMAKE_HOST_SYSTEM_PROCESSOR}")

    find_package(PkgConfig REQUIRED)
    find_package(jsoncpp REQUIRED)

    # Add libswresample too, to match your cross-build link set
    pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
        libavformat
        libavcodec
        libswresample
        libavutil
    )

    target_link_libraries(iris_v4l2_test
        PkgConfig::FFMPEG
        Threads::Threads
        ${CMAKE_DL_LIBS}
        jsoncpp_lib
    )
endif()
