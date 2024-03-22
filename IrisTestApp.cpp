/*
 **************************************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 **************************************************************************************************
*/

#include <execinfo.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>

#include "ConfigParser.h"
#include "Log.h"
#include "V4l2Decoder.h"
#include "V4l2Driver.h"
#include "V4l2Encoder.h"

#define SUCCESS 0
#define BACKTRACE_SIZE 1024

#define TEST_APP_VERSION 1.6

uint32_t gLogLevel = 0x7;

std::unordered_map<std::string, std::string> gCodecMap = {
    {"AVC", "VIDEO_CodingAVC"},
    {"VP9", "VIDEO_CodingVP9"},
    {"HEVC", "VIDEO_CodingHEVC"},
};

std::unordered_map<std::string, unsigned int> gCodecIDMap = {
    {"AVC", V4L2_PIX_FMT_H264},
    {"VP9", V4L2_PIX_FMT_VP9},
    {"HEVC", V4L2_PIX_FMT_HEVC},
};

std::unordered_map<std::string, unsigned int> gColorFormatIDMap = {
    {"NV12", V4L2_PIX_FMT_NV12},
    {"QC08C", V4L2_PIX_FMT_QC08C},
    {"QC10C", V4L2_PIX_FMT_QC10C},
};

int64_t getMSec() {
    auto time_now = std::chrono::system_clock::now();
    auto duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        time_now.time_since_epoch());
    return duration_in_ms.count();
}

void handle(int signal_num) {
#ifdef DEBUG
    char msg[BACKTRACE_SIZE] = {0};
    std::string backTraceName = "./back_trace_" + std::to_string(getMSec());
    FILE* fp = fopen(backTraceName.c_str(), "a+");
    if (fp == nullptr) {
        return;
    }
    snprintf(msg, sizeof(msg), "signal:%d\n", signal_num);
    fwrite(msg, 1, strlen(msg), fp);
    memset(msg, 0, sizeof(msg));

    int nptrs;
    void* buffer[BACKTRACE_SIZE];
    char** strings;

    nptrs = backtrace(buffer, BACKTRACE_SIZE);
    snprintf(msg, sizeof(msg), "backtrace() return %d address\n", nptrs);
    fwrite(msg, 1, strlen(msg), fp);
    memset(msg, 0, sizeof(msg));

    strings = backtrace_symbols(buffer, nptrs);
    if (strings == nullptr) {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < nptrs; i++) {
        snprintf(msg, sizeof(msg), "[%02d] %s\n", i, strings[i]);
        fwrite(msg, 1, strlen(msg), fp);
        memset(msg, 0, sizeof(msg));
    }

    fclose(fp);
    free(strings);
#endif
    signal(signal_num, SIG_DFL);
}

void InitSignalHandler() {
    signal(SIGINT, &handle);
    signal(SIGSEGV, &handle);
    signal(SIGABRT, &handle);
    signal(SIGPIPE, &handle);
}

static int TestingDecoder(ConfigureStruct& config, std::string sessionId) {
    std::shared_ptr<V4l2Decoder> mDecoder = nullptr;
    std::shared_ptr<V4l2DecoderCB> mDecoderCB = nullptr;
    unsigned int codecFmt, pixelFmt;
    int ret = 0;

    codecFmt = gCodecIDMap[config.CodecName];
    pixelFmt = gColorFormatIDMap[config.PixelFormat];

    mDecoder = std::make_shared<V4l2Decoder>(codecFmt, pixelFmt, sessionId);
    mDecoderCB = std::make_shared<V4l2DecoderCB>(mDecoder.get(), sessionId);

    ret = mDecoder->init();
    if (ret) {
        return ret;
    }
    ret = mDecoder->initFFStreamParser(config.InputPath);
    if (ret) {
        return ret;
    }
    ret = mDecoder->registerCallbacks(mDecoderCB);
    if (ret) {
        return ret;
    }

    ret = mDecoder->populateDynamicCommands(config.dynamicCommands);
    if (ret) {
        return ret;
    }

    mDecoder->setDump(config.DumpInputPath, config.Outputpath);

    ret |= mDecoder->setInputSizeOverWrite(2 * 1024 * 1024);
    ret |= mDecoder->setInputActualCount(config.InputBufferCount);
    ret |= mDecoder->setOutputActualCount(config.OutputBufferCount);
    ret |= mDecoder->setResolution(config.Width, config.Height);
    ret |= mDecoder->configureInput();
    ret |= mDecoder->allocateBuffers(INPUT_PORT);
    ret |= mDecoder->startInput();
    ret |= mDecoder->queueBuffers(config.NumFrames);

    mDecoder->stopOutput();
    mDecoder->stopInput();
    mDecoder->deinitFFStreamParser();
    mDecoder->freeBuffers(OUTPUT_PORT);
    mDecoder->freeBuffers(INPUT_PORT);
    mDecoder->deinit();
    if (!ret) {
        printf("**************\nSUCCESS!\n**************\n");
    } else {
        printf("!!!!!!!!!!!!!!\nFAILED!\n!!!!!!!!!!!!!!\n");
    }
    return ret;
}

static int TestingEncoder(ConfigureStruct& config, std::string sessionId) {
    std::shared_ptr<V4l2Encoder> mEncoder = nullptr;
    std::shared_ptr<V4l2EncoderCB> mEncoderCB = nullptr;
    unsigned int codecFmt, pixelFmt;
    int ret = 0;

    codecFmt = gCodecIDMap[config.CodecName];
    pixelFmt = gColorFormatIDMap[config.PixelFormat];

    mEncoder = std::make_shared<V4l2Encoder>(codecFmt, pixelFmt, sessionId);
    mEncoderCB = std::make_shared<V4l2EncoderCB>(mEncoder.get(), sessionId);

    ret = mEncoder->init();
    if (ret) {
        return ret;
    }
    ret = mEncoder->initFFYUVParser(config.InputPath, config.Width,
                                    config.Height, config.PixelFormat);
    if (ret) {
        return ret;
    }

    ret = mEncoder->registerCallbacks(mEncoderCB);
    if (ret) {
        return ret;
    }

    ret = mEncoder->populateStaticConfigs(config.staticControls);
    if (ret) {
        return ret;
    }

    ret = mEncoder->populateDynamicConfigs(config.dynamicControls);
    if (ret) {
        return ret;
    }

    ret = mEncoder->populateDynamicCommands(config.dynamicCommands);
    if (ret) {
        return ret;
    }

    if (config.InputBufferCount > 0) {
        mEncoder->setInputActualCount(config.InputBufferCount);
    }
    if (config.OutputBufferCount > 0) {
        mEncoder->setOutputActualCount(config.OutputBufferCount);
    }
    mEncoder->setResolution(config.Width, config.Height);
    mEncoder->setNALEncoding(false);
    mEncoder->setDump(config.DumpInputPath, config.Outputpath);

    ret |= mEncoder->setOperatingRate(1, config.OperatingRate);
    ret |= mEncoder->setFrameRate(1, config.FrameRate);
    ret |= mEncoder->setStaticControls();
    ret |= mEncoder->configureInput();
    ret |= mEncoder->configureOutput();
    ret |= mEncoder->allocateBuffers(OUTPUT_PORT);
    ret |= mEncoder->allocateBuffers(INPUT_PORT);
    ret |= mEncoder->startOutput();
    ret |= mEncoder->startInput();
    ret |= mEncoder->queueBuffers(config.NumFrames);

    mEncoder->stopInput();
    mEncoder->stopOutput();
    mEncoder->deinitFFYUVParser();
    mEncoder->freeBuffers(INPUT_PORT);
    mEncoder->freeBuffers(OUTPUT_PORT);
    mEncoder->deinit();
    if (!ret) {
        printf("**************\nSUCCESS!\n**************\n");
    } else {
        printf("!!!!!!!!!!!!!!\nFAILED!\n!!!!!!!!!!!!!!\n");
    }
    return ret;
}

int getRegexMatchFileNames(std::string regexPath,
                           std::vector<std::string>& matched_files,
                           std::string& pathToFile) {
    int i;

    if (regexPath.empty()) {
        printf("Error: no configure file found. Run \"./iris_v4l2_test --help\" for more info.\n");
        return -EINVAL;
    }

    for (i = regexPath.size() - 1; i >= 0; i--) {
        if (regexPath[i] == '/') {
            break;
        }
    }

    std::string file_name_regex = regexPath.substr(i + 1);
    std::cout << "File name regex : " << file_name_regex << std::endl;
    std::string directory_path = regexPath.substr(0, i);
    pathToFile = directory_path;
    std::cout << "Directory path : " << directory_path << std::endl;
    std::regex star_replace("\\*");
    std::regex questionmark_replace("\\?");

    auto wildcard_pattern = std::regex_replace(
        std::regex_replace(file_name_regex, star_replace, ".*"), questionmark_replace, ".");

    std::cout << "Wildcard: " << file_name_regex << std::endl
              << "Wildcard Pattern: " << wildcard_pattern << std::endl;

    std::regex wildcard_regex(wildcard_pattern);

    for (const auto& entry : std::filesystem::directory_iterator(directory_path)) {
        if (entry.is_regular_file() &&
            std::regex_match(entry.path().filename().string(), wildcard_regex)) {
            matched_files.push_back(entry.path().filename().string());
        }
    }

    std::cout << "Matched files:\n";
    for (const auto& filename : matched_files) {
        std::cout << filename << '\n';
    }

    return 0;
}

void runAndWaitForComplete(
    std::string& ExecutionMode,
    std::unordered_map<std::string, ConfigureStruct>& mapTestCasesConfig) {
    auto runTest = [&](std::string test) -> void {
        int ret = 0;
        auto& config = mapTestCasesConfig[test];

        if (config.Domain.compare("Decoder") == 0) {
            ret = TestingDecoder(config, test);
        } else {
            ret = TestingEncoder(config, test);
        }

        if (ret) {
            std::cout << "Testcase[ " << test << "] : Failed" << std::endl;
        } else {
            std::cout << "Testcase[ " << test << "] : Passed" << std::endl;
        }
    };

    auto waitFunc = [&](std::string test) -> void { return runTest(test); };

    std::vector<std::shared_ptr<std::thread>> threads;
    if (ExecutionMode == "Concurrent") {
        threads.reserve(mapTestCasesConfig.size());
    }

    for (auto [test, config] : mapTestCasesConfig) {
        if (ExecutionMode == "Concurrent") {
            threads.push_back(std::make_shared<std::thread>(waitFunc, test));
        } else {
            runTest(test);
        }
    }

    if (ExecutionMode == "Concurrent") {
        for (auto thread : threads) {
            if (thread->joinable()) {
                thread->join();
            }
        }
    }

    return;
}

static void showUsage() {
    printf("iris_v4l2_test [V4L Video Test app] \n");
    printf("Usage : iris_v4l2_test [OPTIONS] CONFIG.json\n");
    printf("[OPTIONS] : --help       : No Argument Required         : Display the options\n");
    printf("[OPTIONS] : --config     : Argument Required            : Absolute path of config file\n");
    printf("[OPTIONS] : --loglevel   : Optional Argument Required   : Absolute path of config file\n");
}

int main(int argc, char** argv) {
    int ret, option, codec = 0;
    std::string configPath;

    InitSignalHandler();

    while (1) {
        int optIndex = 0;
        static struct option longOpts[] = {
            {"help",        no_argument,       0,  'h' },
            {"config",      required_argument, 0,  'c' },
            {"loglevel",    optional_argument, 0,  'l' },
            {0,             0,                 0,   0  }
        };

        int opt = getopt_long(argc, argv, "h:c:l:",
                longOpts, &optIndex);

        if (opt == -1) {
            break;
        }

        switch (opt) {
            case 'h':
                showUsage();
                return 0;
            case 'c':
                configPath = optarg;
                printf("Config path: %s\n", configPath.c_str());
                break;
            case 'l':
                gLogLevel = atoi(argv[optind++]);
                printf("Log Level : 0x%x\n", gLogLevel);
                break;
            default:
                printf("Error: invalid option. Run \"./iris_v4l2_test --help\" for more info.\n");
                return -1;
        }
    }

    std::string pathToFile;
    std::vector<std::string> matched_files;

    ret = getRegexMatchFileNames(configPath, matched_files, pathToFile);
    if (ret) {
        return ret;
    }

    std::string ExecutionMode = "Sequential";
    std::unordered_map<std::string, ConfigureStruct> mapTestCasesConfig;

    for (const auto& filename : matched_files) {
        std::cout << "parse " << filename << '\n';

        ret = parseJsonConfigs(pathToFile + "/" + filename, ExecutionMode,
                               mapTestCasesConfig);
        if (ret) {
            printf("Error: Json parsing failed - %s\n", filename.c_str());
            return ret;
        }

        runAndWaitForComplete(ExecutionMode, mapTestCasesConfig);
    }

    std::cout << "Testapp Version " << TEST_APP_VERSION << std::endl;

    return 0;
}
