/*
 **************************************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 **************************************************************************************************
*/

#ifndef _CONFIG_PARSER_H_
#define _CONFIG_PARSER_H_

#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <json/json.h>
#include <json/reader.h>

enum ValueType {
    INTEGER = 0,
    STRING,
};

struct EventConfig {
    std::string Id;
    std::string valueStr;
    enum ValueType vtype;
    int valueInt;
    int fnum;
};

struct StaticV4L2CtrlInfo {
    unsigned int id = 0;
    int value = 0;
};

struct DynamicV4L2CtrlInfo {
    unsigned int id = 0;
    int value = 0;
    unsigned int fnum = 0;
};

struct DynamicCommandInfo {
    std::string id = "";
    std::string value = "";
    unsigned int fnum = 0;
};

struct ConfigureStruct {
    int Width;
    int Height;
    int NumFrames;
    int FrameRate;
    int PauseAtFrame;
    int OperatingRate;
    int PauseDurationMS;
    int InputBufferCount;
    int OutputBufferCount;

    std::string Domain;
    std::string CodecName;
    std::string InputPath;
    std::string Outputpath;
    std::string PixelFormat;
    std::string VideoDevice;
    std::string DumpInputPath;

    std::list<std::shared_ptr<EventConfig>> staticControls;
    std::list<std::shared_ptr<EventConfig>> dynamicControls;
    std::list<std::shared_ptr<EventConfig>> dynamicCommands;
};

static std::string ExecutionMode = "Sequential";
static std::unordered_map<std::string, ConfigureStruct> mapTestCasesConfig;

int parseJsonConfigs(
    const std::string& path, std::string& ExecutionMode,
    std::unordered_map<std::string, ConfigureStruct>& mapTestCasesConfig);

#endif  //_CONFIG_PARSER_H_
