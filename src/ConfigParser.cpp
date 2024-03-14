/*
 **************************************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 **************************************************************************************************
*/

#include <limits.h>
#include <stdio.h>
#include <fstream>

#include "ConfigParser.h"
#include "V4l2Codec.h"

#ifndef _LINUX_
#define PRINT std::cout
#else
#define PRINT std::cerr
#endif

std::unordered_set<std::string> CodecIdType = {
    "IQP",
    "PQP",
    "BQP",
    "Tier",
    "Level",
    "MinQP",
    "MaxQP",
    "MaxIQP",
    "MaxPQP",
    "MaxBQP",
    "MinIQP",
    "MinPQP",
    "MinBQP",
    "Profile",
    "LosslessCU",
    "LFTCOffset",
    "Transform8x8",
    "LFBetaOffset",
    "LFAlphaOffset",
    "EntropyCoding",
    "LoopFilterMode",
    "HierCodingL0BR",
    "HierCodingL1BR",
    "HierCodingL2BR",
    "HierCodingL3BR",
    "HierCodingL4BR",
    "HierCodingL5BR",
    "HierCodingL6BR",
    "HierarchicalCoding",
    "HierarchicalCodingType",
    "HierarchicalLayerCount",
};

std::unordered_set<std::string> MandatoryCtrls = {
    "Profile", "Level", "BitRate", "BitRateMode", "PrefixHeaderMode",
};

#define CHECK_TRUE(cond, msg)       \
    if ((cond) != true) {           \
        printf("Error: %s\n", msg); \
        return -EINVAL;             \
    }

#define CHECK_MANDATORY(jconfigs, item, type)                                          \
    {                                                                                  \
        CHECK_TRUE(!jconfigs[#item].empty(), "Check config " #item " not empty");      \
        CHECK_TRUE(jconfigs[#item].is##type(), "Check config " #item " is " #type ""); \
        config.item = jconfigs[#item].as##type();                                      \
    }

#define CHECK_OPTIONAL(jconfigs, item, type, dVal)                                         \
    {                                                                                      \
        if (!jconfigs[#item].empty()) {                                                    \
            CHECK_TRUE(jconfigs[#item].is##type(), "Check config " #item " is " #type ""); \
            config.item = jconfigs[#item].as##type();                                      \
        } else {                                                                           \
            config.item = dVal;                                                            \
            printf("No " #item ".\n");                                                     \
        }                                                                                  \
    }

static int getConfigs(Json::Value allConfigs, ConfigureStruct& config, std::string Configs) {
    int count = 0;

    if (allConfigs[Configs].empty()) {
        printf("No Configs provided\n");
        return 0;
    }

    CHECK_TRUE(allConfigs[Configs].isArray(), "Check config is array");

    for (auto cfgIdx : allConfigs[Configs]) {
        auto lCtrls = std::make_shared<EventConfig>();

        CHECK_TRUE(cfgIdx.isObject(), "Configs is Object");
        CHECK_TRUE(!cfgIdx["Id"].empty(), "Configs::Id is not empty");
        CHECK_TRUE(cfgIdx["Id"].isString(), "Configs::Id is string");

        if (CodecIdType.find(cfgIdx["Id"].asString()) != CodecIdType.end()) {
            lCtrls->Id = config.CodecName + "_" + cfgIdx["Id"].asString();
        } else {
            lCtrls->Id = cfgIdx["Id"].asString();
        }

        CHECK_TRUE(!cfgIdx["Vtype"].empty(), "Configs::VType is not empty");
        CHECK_TRUE(cfgIdx["Vtype"].isString(), "Configs::Vtype is string");

        std::string vtype = cfgIdx["Vtype"].asString();
        CHECK_TRUE(!cfgIdx["Value"].empty(), "Configs::Value is not empty");

        if ((cfgIdx["Vtype"].asString().compare("Int")) == 0) {
            CHECK_TRUE(cfgIdx["Value"].isInt(), "Configs::Value is Int");
            lCtrls->vtype = INTEGER;
            lCtrls->valueInt = cfgIdx["Value"].asInt();

        } else {
            CHECK_TRUE(cfgIdx["Value"].isString(), "Configs::Value is String");
            lCtrls->vtype = STRING;
            if (CodecIdType.find(cfgIdx["Id"].asString()) != CodecIdType.end()) {
                lCtrls->valueStr = config.CodecName + "_" + cfgIdx["Value"].asString();
            } else {
                lCtrls->valueStr = cfgIdx["Value"].asString();
            }
        }

        printf("ID str : %s\n", lCtrls->Id.c_str());

        if (Configs.compare("StaticControls") == 0) {
            if (MandatoryCtrls.find(cfgIdx["Id"].asString()) != MandatoryCtrls.end()) {
                count++;
            }
            CHECK_TRUE(cfgIdx["Fnum"].empty(), "StaticConfigs doesnt need Fnum");
            lCtrls->fnum = -1;
            config.staticControls.push_back(lCtrls);

        } else if (Configs.compare("DynamicControls") == 0) {
            CHECK_TRUE(!cfgIdx["Fnum"].empty(), "DynamicConfigs need Fnum");
            CHECK_TRUE(cfgIdx["Fnum"].isInt(), "Configs::Fnum is Int");

            lCtrls->fnum = cfgIdx["Fnum"].asInt();
            config.dynamicControls.push_back(lCtrls);

        } else if (Configs.compare("DynamicCommands") == 0) {
            CHECK_TRUE(!cfgIdx["Fnum"].empty(), "DynamicConfigs need Fnum");
            CHECK_TRUE(cfgIdx["Fnum"].isInt(), "Configs::Fnum is int");

            lCtrls->fnum = cfgIdx["Fnum"].asInt();
            config.dynamicCommands.push_back(lCtrls);
        }

        printf("Enter End parse \n");
    }

    if (Configs.compare("StaticControls") == 0) {
        if (count < MandatoryCtrls.size()) {
            printf("Mandatory controls not found!!!\n");
            return -EINVAL;
        }
    }

    return 0;
}

static int readFileToString(const std::string& path, std::string* content) {
    if (!content) {
        return -EINVAL;
    }
    if (std::ifstream fs{path, std::ios::in | std::ios::binary | std::ios::ate /* seek to end */}) {
        constexpr size_t kCacheSize = 1024;
        char cache[kCacheSize];
        auto fileSize = fs.tellg();
        content->reserve((size_t)fileSize);
        fs.seekg(0);

        do {
            fs.read(cache, kCacheSize);
            if (fs.gcount() > 0) {
                content->append(cache, (size_t)fs.gcount());
            }
        } while (!fs.eof() && fs.gcount() == kCacheSize);
    } else {
        printf("Failed to open file (%s) for reading", path.c_str());
        return -EINVAL;
    }

    return 0;
}

int parseJsonConfigs(
    const std::string& path, std::string& ExecutionMode,
    std::unordered_map<std::string, ConfigureStruct>& mapTestCasesConfig) {
    Json::Value allConfigs;
    Json::CharReaderBuilder builder;

    int ret = 0;
    std::string jsonString;
    std::string errorMessage;

    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

    ret = readFileToString(path, &jsonString);
    if (ret) {
        printf("Error: Read configure file failed - %s\n", path.c_str());
        return -EINVAL;
    }

    ret = reader->parse(&*jsonString.begin(), &*jsonString.end(), &allConfigs, &errorMessage);
    if (!ret) {
        printf("Error: Parse configuration failed - %s\n", errorMessage.c_str());
        return -EINVAL;
    }

    if (allConfigs.empty()) {
        printf("Error: Check config not empty\n");
        return -EINVAL;
    }

    if (!allConfigs["ExecutionMode"].empty()) {
        ExecutionMode = allConfigs["ExecutionMode"].asString();
    }
    const Json::Value testcases = allConfigs["TestCases"];

    for (auto const& testcase : testcases) {
        ConfigureStruct config = {};
        auto testConfig = testcase["TestConfigs"];

        CHECK_MANDATORY(testConfig, Domain, String);
        CHECK_MANDATORY(testConfig, InputPath, String);
        CHECK_MANDATORY(testConfig, NumFrames, Int);
        if (config.NumFrames <= 0) {
            printf("Non-postive numFrame. Will be set to infinite.\n");
            config.NumFrames = INT_MAX;
        }
        CHECK_MANDATORY(testConfig, CodecName, String);
        CHECK_MANDATORY(testConfig, PixelFormat, String);

        CHECK_MANDATORY(testConfig, Width, Int);
        CHECK_MANDATORY(testConfig, Height, Int);

        CHECK_OPTIONAL(testConfig, Outputpath, String, "");
        CHECK_OPTIONAL(testConfig, DumpInputPath, String, "");

        if (config.Domain.compare("Encoder") == 0) {
            CHECK_MANDATORY(testConfig, OperatingRate, Int);
            CHECK_MANDATORY(testConfig, FrameRate, Int);
            CHECK_OPTIONAL(testConfig, InputBufferCount, Int, 32);
            CHECK_OPTIONAL(testConfig, OutputBufferCount, Int, 32);
        } else {
            CHECK_OPTIONAL(testConfig, InputBufferCount, Int, 16);
            CHECK_OPTIONAL(testConfig, OutputBufferCount, Int, 16);
        }

        ret = getConfigs(testConfig, config, "StaticControls");
        if (ret) {
            printf("Static configs are mandatory for Encoder!\n");
            return ret;
        }

        ret = getConfigs(testConfig, config, "DynamicControls");
        if (ret) {
            printf("Dynamic configs are not mandatory!\n");
            return 0;
        }

        ret = getConfigs(testConfig, config, "DynamicCommands");
        if (ret) {
            printf("Dynamic commands are not mandatory!\n");
            return 0;
        }

        Json::FastWriter writer;
        std::string testCaseName = writer.write(testcase["Name"]);
        mapTestCasesConfig[testCaseName] = config;
    }

    return ret;
}
