/*
 **************************************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 **************************************************************************************************
*/

#include <unistd.h>

#include "V4l2Codec.h"

std::unordered_map<std::string, unsigned int> gV4l2KeyCIDMap = {
    //Codec Based
    {"AVC_Level",                    V4L2_CID_MPEG_VIDEO_H264_LEVEL},
    {"AVC_Profile",                  V4L2_CID_MPEG_VIDEO_H264_PROFILE},
    {"AVC_MinQP",                    V4L2_CID_MPEG_VIDEO_H264_MIN_QP},
    {"AVC_MaxQP",                    V4L2_CID_MPEG_VIDEO_H264_MAX_QP},
    {"AVC_IQP",                      V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP},
    {"AVC_PQP",                      V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP},
    {"AVC_BQP",                      V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP},
    {"AVC_MaxIQP",                   V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MAX_QP},
    {"AVC_MaxPQP",                   V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MAX_QP},
    {"AVC_MaxBQP",                   V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MAX_QP},
    {"AVC_MinIQP",                   V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MIN_QP},
    {"AVC_MinPQP",                   V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MIN_QP},
    {"AVC_MinBQP",                   V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MIN_QP},
    {"AVC_EntropyCoding",            V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE},
    {"AVC_Transform8x8",             V4L2_CID_MPEG_VIDEO_H264_8X8_TRANSFORM},
    {"AVC_LoopFilterMode",           V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE},
    {"AVC_LFBetaOffset",             V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_BETA},
    {"AVC_LFAlphaOffset",            V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_ALPHA},
    {"AVC_HierarchicalCoding",       V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING},
    {"AVC_HierarchicalCodingType",   V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_TYPE},
    {"AVC_HierarchicalLayerCount",   V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER},
    {"AVC_HierCodingL0BR",           V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L0_BR},
    {"AVC_HierCodingL1BR",           V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L1_BR},
    {"AVC_HierCodingL2BR",           V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L2_BR},
    {"AVC_HierCodingL3BR",           V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L3_BR},
    {"AVC_HierCodingL4BR",           V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L4_BR},
    {"AVC_HierCodingL5BR",           V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L5_BR},
    {"AVC_HierCodingL6BR",           V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L6_BR},

    {"HEVC_Tier",                    V4L2_CID_MPEG_VIDEO_HEVC_TIER},
    {"HEVC_Level",                   V4L2_CID_MPEG_VIDEO_HEVC_LEVEL},
    {"HEVC_Profile",                 V4L2_CID_MPEG_VIDEO_HEVC_PROFILE},
    {"HEVC_MinQP",                   V4L2_CID_MPEG_VIDEO_HEVC_MIN_QP},
    {"HEVC_MaxQP",                   V4L2_CID_MPEG_VIDEO_HEVC_MAX_QP},
    {"HEVC_IQP",                     V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_QP},
    {"HEVC_PQP",                     V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_QP},
    {"HEVC_BQP",                     V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_QP},
    {"HEVC_MaxIQP",                  V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MAX_QP},
    {"HEVC_MaxPQP",                  V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MAX_QP},
    {"HEVC_MaxBQP",                  V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MAX_QP},
    {"HEVC_MinIQP",                  V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MIN_QP},
    {"HEVC_MinPQP",                  V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MIN_QP},
    {"HEVC_MinBQP",                  V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MIN_QP},
    {"HEVC_LoopFilterMode",          V4L2_CID_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE},
    {"HEVC_LFTCOffset",              V4L2_CID_MPEG_VIDEO_HEVC_LF_TC_OFFSET_DIV2},
    {"HEVC_HierCodingL0BR",          V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L0_BR},
    {"HEVC_HierCodingL1BR",          V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L1_BR},
    {"HEVC_HierCodingL2BR",          V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L2_BR},
    {"HEVC_HierCodingL3BR",          V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L3_BR},
    {"HEVC_HierCodingL4BR",          V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L4_BR},
    {"HEVC_HierCodingL5BR",          V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L5_BR,},
    {"HEVC_HierCodingL6BR",          V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L6_BR,},
    {"HEVC_HierarchicalCodingType",  V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_TYPE},
    {"HEVC_HierarchicalLayerCount",  V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_LAYER},

    //Non-Codec Based
    {"VFlip",                        V4L2_CID_VFLIP},
    {"HFlip",                        V4L2_CID_HFLIP},
    {"Rotate",                       V4L2_CID_ROTATE},
    {"BitRate",                      V4L2_CID_MPEG_VIDEO_BITRATE},
    {"BFrames",                      V4L2_CID_MPEG_VIDEO_B_FRAMES},
    {"GOPSize",                      V4L2_CID_MPEG_VIDEO_GOP_SIZE},
    {"LTRCount",                     V4L2_CID_MPEG_VIDEO_LTR_COUNT},
    {"VBVDelay",                     V4L2_CID_MPEG_VIDEO_VBV_DELAY},
    {"PrefixHeaderMode",             V4L2_CID_MPEG_VIDEO_HEADER_MODE},
    {"PeakBitrate",                  V4L2_CID_MPEG_VIDEO_BITRATE_PEAK},
    {"BitRateMode",                  V4L2_CID_MPEG_VIDEO_BITRATE_MODE},
    {"UseLTR",                       V4L2_CID_MPEG_VIDEO_USE_LTR_FRAMES},
    {"FrameRC",                      V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE},
    {"LTRIndex",                     V4L2_CID_MPEG_VIDEO_FRAME_LTR_INDEX},
    {"InjectKeyFrame",               V4L2_CID_MPEG_VIDEO_FORCE_KEY_FRAME},
    {"CQ",                           V4L2_CID_MPEG_VIDEO_CONSTANT_QUALITY},
    {"MultiSliceMode",               V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE},
    {"MaxMacroBlocksPerSlice",       V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB},
    {"IntraRefreshPeriod",           V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD},
    {"MaxBytesPerSlice",             V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_BYTES},
    {"PrependPsToIDR",               V4L2_CID_MPEG_VIDEO_PREPEND_SPSPPS_TO_IDR},
    {"BasePriorityID",               V4L2_CID_MPEG_VIDEO_BASELAYER_PRIORITY_ID},
    {"IntraRefreshType",             V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD_TYPE},
};

std::unordered_map<std::string, int> gProfileMap = {
    //AVC Codec Profiles
    {"AVC_BASELINE",                 V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE },
    {"AVC_MAIN",                     V4L2_MPEG_VIDEO_H264_PROFILE_MAIN },
    {"AVC_HIGH",                     V4L2_MPEG_VIDEO_H264_PROFILE_HIGH },

    //HEVC Codec Profiles
    {"HEVC_MAIN",                    V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN },
    {"HEVC_MAIN10",                  V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN_10 },
    {"HEVC_MAIN_SP",                 V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN_STILL_PICTURE },
};

std::unordered_map<std::string, int> gLevelMap = {
    //AVC Codec Levels
    { "AVC_1.0",                     V4L2_MPEG_VIDEO_H264_LEVEL_1_0},
    { "AVC_1B",                      V4L2_MPEG_VIDEO_H264_LEVEL_1B},
    { "AVC_1.1",                     V4L2_MPEG_VIDEO_H264_LEVEL_1_1},
    { "AVC_1.2",                     V4L2_MPEG_VIDEO_H264_LEVEL_1_2},
    { "AVC_1.3",                     V4L2_MPEG_VIDEO_H264_LEVEL_1_3},
    { "AVC_2.0",                     V4L2_MPEG_VIDEO_H264_LEVEL_2_0},
    { "AVC_2.1",                     V4L2_MPEG_VIDEO_H264_LEVEL_2_1},
    { "AVC_2.2",                     V4L2_MPEG_VIDEO_H264_LEVEL_2_2},
    { "AVC_3.0",                     V4L2_MPEG_VIDEO_H264_LEVEL_3_0},
    { "AVC_3.1",                     V4L2_MPEG_VIDEO_H264_LEVEL_3_1},
    { "AVC_3.2",                     V4L2_MPEG_VIDEO_H264_LEVEL_3_2},
    { "AVC_4.0",                     V4L2_MPEG_VIDEO_H264_LEVEL_4_0},
    { "AVC_4.1",                     V4L2_MPEG_VIDEO_H264_LEVEL_4_1},
    { "AVC_4.2",                     V4L2_MPEG_VIDEO_H264_LEVEL_4_2},
    { "AVC_5.0",                     V4L2_MPEG_VIDEO_H264_LEVEL_5_0},
    { "AVC_5.1",                     V4L2_MPEG_VIDEO_H264_LEVEL_5_1},
    { "AVC_5.2",                     V4L2_MPEG_VIDEO_H264_LEVEL_5_2},
    { "AVC_6.0",                     V4L2_MPEG_VIDEO_H264_LEVEL_6_0},
    { "AVC_6.1",                     V4L2_MPEG_VIDEO_H264_LEVEL_6_1},
    { "AVC_6.2",                     V4L2_MPEG_VIDEO_H264_LEVEL_6_2},
     

    //HEVC Codec Levels
    { "HEVC_1.0",                    V4L2_MPEG_VIDEO_HEVC_LEVEL_1},
    { "HEVC_2.0",                    V4L2_MPEG_VIDEO_HEVC_LEVEL_2},
    { "HEVC_2.1",                    V4L2_MPEG_VIDEO_HEVC_LEVEL_2_1},
    { "HEVC_3.0",                    V4L2_MPEG_VIDEO_HEVC_LEVEL_3},
    { "HEVC_3.1",                    V4L2_MPEG_VIDEO_HEVC_LEVEL_3_1},
    { "HEVC_4.0",                    V4L2_MPEG_VIDEO_HEVC_LEVEL_4},
    { "HEVC_4.1",                    V4L2_MPEG_VIDEO_HEVC_LEVEL_4_1},
    { "HEVC_5.0",                    V4L2_MPEG_VIDEO_HEVC_LEVEL_5},
    { "HEVC_5.1",                    V4L2_MPEG_VIDEO_HEVC_LEVEL_5_1},
    { "HEVC_5.2",                    V4L2_MPEG_VIDEO_HEVC_LEVEL_5_2},
    { "HEVC_6.0",                    V4L2_MPEG_VIDEO_HEVC_LEVEL_6},
    { "HEVC_6.1",                    V4L2_MPEG_VIDEO_HEVC_LEVEL_6_1},
    { "HEVC_6.2",                    V4L2_MPEG_VIDEO_HEVC_LEVEL_6_2},
};

std::unordered_map<std::string, int> gLoopFilterModeMap = {
    //AVC Codec modes
    {"AVC_ENABLED",                  V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_ENABLED },
    {"AVC_DISABLED",                 V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_DISABLED },
    {"AVC_DISABLED_AT_SLICE_BOUNDARY", V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_DISABLED_AT_SLICE_BOUNDARY },

    //HEVC Codec modes
    {"HEVC_ENABLED",                  V4L2_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE_ENABLED },
    {"HEVC_DISABLED",                 V4L2_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE_DISABLED },
    {"HEVC_DISABLED_AT_SLICE_BOUNDARY", V4L2_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE_DISABLED_AT_SLICE_BOUNDARY },
};

std::unordered_map<std::string, int> gTierMap = {
    {"HEVC_MAIN",                    V4L2_MPEG_VIDEO_HEVC_TIER_MAIN},
    {"HEVC_HIGH",                    V4L2_MPEG_VIDEO_HEVC_TIER_HIGH},
};

std::unordered_map<std::string, int> gBitrateModeMap = {
    {"VBR",                          V4L2_MPEG_VIDEO_BITRATE_MODE_VBR},
    {"CBR",                          V4L2_MPEG_VIDEO_BITRATE_MODE_CBR},
    {"CQ",                           V4L2_MPEG_VIDEO_BITRATE_MODE_CQ},
    {"MBR",                          V4L2_MPEG_VIDEO_BITRATE_MODE_MBR},
};

std::unordered_map<std::string, int> gHeaderModeMap = {
    {"JOINED",                      V4L2_MPEG_VIDEO_HEADER_MODE_JOINED_WITH_1ST_FRAME},
    {"SEPARATE",                    V4L2_MPEG_VIDEO_HEADER_MODE_SEPARATE},
};

std::unordered_map<std::string, int> gHierCodingTypeMap = {
    {"AVC_HierP",                   V4L2_MPEG_VIDEO_H264_HIERARCHICAL_CODING_P },
    {"AVC_HierB",                   V4L2_MPEG_VIDEO_H264_HIERARCHICAL_CODING_B },

    {"HEVC_HierP",                  V4L2_MPEG_VIDEO_HEVC_HIERARCHICAL_CODING_P },
    {"HEVC_HierB",                  V4L2_MPEG_VIDEO_HEVC_HIERARCHICAL_CODING_B },
};

std::unordered_map<std::string, int> gMultiSliceModeMap = {
    {"SINGLE",                      V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_SINGLE },
    {"MAX_MB",                      V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_MAX_MB },
    {"MAX_BYTES",                   V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_MAX_BYTES },
};

std::unordered_map<std::string, int> gIntraRefreshTypeMap = {
    {"CYCLIC",                      V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD_TYPE_CYCLIC },
    {"RANDOM",                      V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD_TYPE_RANDOM },
};

std::unordered_map<std::string, int> gEntropyModeMap = {
    {"AVC_CAVLC",                   V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC },
    {"AVC_CABAC",                   V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CABAC },
};

std::unordered_map<std::string, int> gMemoryTypeMap = {
    {"",                            0},
    {"MMAP",                        V4L2_MEMORY_MMAP},
    {"DMA_BUF",                     V4L2_MEMORY_DMABUF},
};

V4l2Codec::V4l2Codec(unsigned int codec, unsigned int pixel,
                     std::string sessionId)
    : mCodecFmt(codec), mPixelFmt(pixel), mSessionId(sessionId) {
    mV4l2Driver = std::make_shared<V4l2Driver>(mSessionId);
}

V4l2Codec::~V4l2Codec() {
    if (mOutputDumpFile) {
        fclose(mOutputDumpFile);
        mOutputDumpFile = nullptr;
    }
    if (mInputDumpFile) {
        fclose(mInputDumpFile);
        mInputDumpFile = nullptr;
    }
}

std::string V4l2Codec::id() {
    return mSessionId;
}

int V4l2Codec::setStaticControls() {
    int ret = 0;
    while (!mStaticControls.empty()) {
        auto ctrl = mStaticControls.front();
        LOGI("%s: id: %#x, value: %d\n", __FUNCTION__, ctrl->id, ctrl->value);
        ret = setControl(ctrl->id, ctrl->value);
        if (ret) {
            return ret;
        }
        mStaticControls.pop_front();
    }
    return ret;
}

int V4l2Codec::setDynamicCommands(unsigned int currFrame) {
    int ret = 0;
    auto itr = mDynamicCommands.begin();

    while (itr != mDynamicCommands.end()) {
        auto cmd = *itr;

        if (cmd->fnum != currFrame) {
            itr++;
            continue;
        }
        LOGI("%s: id: %s, value: %s fnum: %u\n", __FUNCTION__, cmd->id.c_str(),
            cmd->value.c_str(), cmd->fnum);

        if (cmd->id.compare("PauseUSec") == 0) {
            usleep(stoi(cmd->value));

        } else if (cmd->id.compare("IDRSeek") == 0) {
            mIDRSeek[cmd->fnum] = stoi(cmd->value);

        } else if (cmd->id.compare("RandomSeek") == 0) {
            mRandomSeek[cmd->fnum] = stoi(cmd->value);
        }

        itr = mDynamicCommands.erase(itr);
    }

    return ret;
}

int V4l2Codec::setDynamicControls(unsigned int currFrame) {
    int ret = 0;
    auto itr = mDynamicControls.begin();

    while (itr != mDynamicControls.end()) {
        auto ctrl = *itr;

        if (ctrl->fnum != currFrame) {
            itr++;
            continue;
        }
        LOGI("%s: id: %#x, value: %d fnum: %d\n", __FUNCTION__, ctrl->id,
            ctrl->value, ctrl->fnum);

        ret = setControl(ctrl->id, ctrl->value);
        if (ret) {
            return ret;
        }

        itr = mDynamicControls.erase(itr);
    }

    return ret;
}

bool V4l2Codec::fillCfgCtrls(std::shared_ptr<EventConfig> ctrl,
                             unsigned int* id, int* value) {
    std::string idStr;

    if (ctrl->Id.find("_") != std::string::npos) {
        int pos = ctrl->Id.find("_") + 1;
        idStr = ctrl->Id.substr(pos, ctrl->Id.length());
    } else {
        idStr = ctrl->Id;
    }

    *id = gV4l2KeyCIDMap[ctrl->Id];

    if (ctrl->vtype == STRING) {
        if (idStr.compare("Profile") == 0) {
            *value = gProfileMap[ctrl->valueStr];
        } else if (idStr.compare("Tier") == 0) {
            *value = gTierMap[ctrl->valueStr];
        } else if (idStr.compare("Level") == 0) {
            *value = gLevelMap[ctrl->valueStr];
        } else if (idStr.compare("BitRateMode") == 0) {
            // non-codec based
            *value = gBitrateModeMap[ctrl->valueStr];
        } else if (idStr.compare("EntropyCoding") == 0) {
            *value = gEntropyModeMap[ctrl->valueStr];
        } else if (idStr.compare("MultiSliceMode") == 0) {
            *value = gMultiSliceModeMap[ctrl->valueStr];
        } else if (idStr.compare("LoopFilterMode") == 0) {
            *value = gLoopFilterModeMap[ctrl->valueStr];
        } else if (idStr.compare("IntraRefreshType") == 0) {
            *value = gIntraRefreshTypeMap[ctrl->valueStr];
        } else if (idStr.compare("PrefixHeaderMode") == 0) {
            *value = gHeaderModeMap[ctrl->valueStr];
        } else if (idStr.compare("HierarchicalCodingType") == 0) {
            *value = gHierCodingTypeMap[ctrl->valueStr];
        }
    } else {
        *value = ctrl->valueInt;
    }

    return true;
}

int V4l2Codec::populateStaticConfigs(
    std::list<std::shared_ptr<EventConfig>> eventConfig) {
    uint32_t level = 0, tier = 0;
    bool sawLevel = false, sawTier = false;

    while (!eventConfig.empty()) {
        auto ctrl = eventConfig.front();
        auto sControl = std::make_shared<StaticV4L2CtrlInfo>();

        LOGV("%s: %s %s %d %d %s\n", __func__, ctrl->Id.c_str(),
            ctrl->valueStr.c_str(), ctrl->valueInt, ctrl->fnum,
            ctrl->Id.c_str());

        bool ret = fillCfgCtrls(ctrl, &sControl->id, &sControl->value);

        if (ret) {
            mStaticControls.push_back(sControl);
        }

        std::string idStr;

        if (ctrl->Id.find("_") != std::string::npos) {
            int pos = ctrl->Id.find("_") + 1;
            idStr = ctrl->Id.substr(pos, ctrl->Id.length());
        } else {
            idStr = ctrl->Id;
        }

        if (idStr.compare("Level") == 0) {
            level = sControl->value;
            sawLevel = true;

        } else if (idStr.compare("Tier") == 0) {
            tier = sControl->value;
            sawTier = true;

        } else if (idStr.compare("FrameRC") == 0) {
            if (sControl->value) {
                auto control = std::make_shared<StaticV4L2CtrlInfo>();

                control->id = V4L2_CID_MPEG_VIDEO_FRAME_SKIP_MODE;
                control->value = V4L2_MPEG_VIDEO_FRAME_SKIP_MODE_DISABLED;

                mStaticControls.push_back(control);
            }
        }

        eventConfig.pop_front();
    }

    if (mCodecFmt == V4L2_PIX_FMT_H264 and sawLevel) {
        queryControlsAVC(level);
    } else if (mCodecFmt == V4L2_PIX_FMT_HEVC and sawLevel and sawTier) {
        queryControlsHEVC(level, tier);
    }

    return 0;
}

int V4l2Codec::populateDynamicConfigs(
    std::list<std::shared_ptr<EventConfig>> eventConfig) {
    while (!eventConfig.empty()) {
        auto ctrl = eventConfig.front();
        auto dControl = std::make_shared<DynamicV4L2CtrlInfo>();

        LOGV("%s: %s %s %d %d %s\n", __func__, ctrl->Id.c_str(),
            ctrl->valueStr.c_str(), ctrl->valueInt, ctrl->fnum,
            ctrl->Id.c_str());

        bool ret = fillCfgCtrls(ctrl, &dControl->id, &dControl->value);

        dControl->fnum = ctrl->fnum;
        if (ret) {
            mDynamicControls.push_back(dControl);
        }
        eventConfig.pop_front();
    }
    return 0;
}

int V4l2Codec::populateDynamicCommands(
    std::list<std::shared_ptr<EventConfig>> eventConfig) {
    while (!eventConfig.empty()) {
        auto dynCmd = eventConfig.front();
        eventConfig.pop_front();

        auto dynCmdInfo = std::make_shared<DynamicCommandInfo>();

        LOGV("%s: %s %s %d %d %s\n", __func__, dynCmd->Id.c_str(),
            dynCmd->valueStr.c_str(), dynCmd->valueInt, dynCmd->fnum,
            dynCmd->Id.c_str());

        dynCmdInfo->id = dynCmd->Id;
        dynCmdInfo->value = dynCmd->valueStr;
        dynCmdInfo->fnum = dynCmd->fnum;

        mDynamicCommands.push_back(dynCmdInfo);
    }
    return 0;
}

int V4l2Codec::setControl(unsigned int ctrlId, int value) {
    int ret = 0;
    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = ctrlId;
    control.value = value;
    ret = mV4l2Driver->setControl(&control);
    if (ret) {
        return ret;
    }

    return 0;
}

int V4l2Codec::setInputSizeOverWrite(int size) {
    LOGW("Client Input Size OverWrite %d\n", size);
    mInputSizeOverWrite = size;
    return 0;
}

int V4l2Codec::setInputActualCount(int count) {
    LOGW("Client set input actual count %d\n", count);
    mActualInputCount = count;
    return 0;
}

int V4l2Codec::setOutputActualCount(int count) {
    LOGW("Client set output actual count %d\n", count);
    mActualOutputCount = count;
    return 0;
}

int V4l2Codec::startInput() {
    int ret = 0;

    ret = mV4l2Driver->streamOn(INPUT_MPLANE);
    if (ret) {
        return ret;
    }
    setInputPortStarted(true);

    return 0;
}

int V4l2Codec::startOutput() {
    int ret = 0;

    ret = mV4l2Driver->streamOn(OUTPUT_MPLANE);
    if (ret) {
        return ret;
    }
    setOutputPortStarted(true);

    return 0;
}

int V4l2Codec::stopInput() {
    int ret = 0;

    ret = mV4l2Driver->streamOff(INPUT_MPLANE);
    if (ret) {
        return ret;
    }
    {
        std::unique_lock<std::mutex> lock(mInputBufLock);
        while (!mPendingInputBufs.empty()) {
            auto buf = mPendingInputBufs.front();
            mInputBufs.push_back(buf);
            mPendingInputBufs.pop_front();
        }
    }
    setInputPortStarted(false);
    return ret;
}

int V4l2Codec::stopOutput() {
    int ret = 0;

    ret = mV4l2Driver->streamOff(OUTPUT_MPLANE);
    if (ret) {
        return ret;
    }

    {
        std::unique_lock<std::mutex> lock(mOutputBufLock);
        while (!mPendingOutputBufs.empty()) {
            auto buf = mPendingOutputBufs.front();
            mOutputBufs.push_back(buf);
            mPendingOutputBufs.pop_front();
        }
    }
    setOutputPortStarted(false);
    return ret;
}

int V4l2Codec::setOutputBufferData(std::shared_ptr<v4l2_buffer> buf) {
    auto itr = mOutputBuffersPool.find(buf->index);
    if (itr == mOutputBuffersPool.end()) {
        LOGE("Error: no DMA buffer found for buffer index: %d\n", buf->index);
        return -EINVAL;
    }
    auto& buffer = itr->second;
    if (mMemoryType == V4L2_MEMORY_DMABUF) {
        auto dmaBuf = std::dynamic_pointer_cast<DMABuffer>(buffer);
        buf->m.planes[0].bytesused = getOutputSize();
        buf->m.planes[0].data_offset = 0;
        buf->m.planes[0].length = getOutputSize();
        buf->m.planes[0].m.fd = dmaBuf->mFd;
    }
    return 0;
}

int V4l2Codec::setDump(std::string inputFile, std::string outputFile) {
    static std::unordered_map<std::string, uint32_t> sNames;

    auto genUniqueFileName = [&](std::string filename) -> std::string {
        if (sNames[filename] != 0) {
            if (access (filename.c_str(), F_OK) == -1 ) {
                /*file does not exist. Reset cache;*/
                sNames[filename] = 0;
            }
        }

        /* append instance count to filename. */
        std::string modified = filename;
        if (sNames[filename] != 0) {
            size_t extPos = filename.find_last_of('.');
            if (extPos == std::string::npos) {
                LOGE("Invalid file name. No extension specified");
                return "";
            }
            modified.insert(extPos, std::to_string(sNames[filename])).insert(extPos, 1, '_');
        }
        sNames[filename]++;
        return modified;
    };


    mInputDumpFile = fopen(genUniqueFileName(inputFile).c_str(), "wb");
    if (!mInputDumpFile) {
        LOGE("Error: failed to open input dump file.\n");
    }

    mOutputDumpFile = fopen(genUniqueFileName(outputFile).c_str(), "wb");
    if (!mOutputDumpFile) {
        LOGE("Error: failed to open output file.\n");
    }
    return 0;
}

int V4l2Codec::setMemoryType(std::string memoryType) {
    mMemoryType = gMemoryTypeMap[memoryType];
    mV4l2Driver->setMemoryType(mMemoryType);
    LOGD("Set MemoryType: %u.\n", mMemoryType);
    return 0;
}

int V4l2Codec::allocateBuffers(port_type port) {
    int bufCount = 0, bufSize = 0, ret = 0;
    std::shared_ptr<v4l2_buffer> buf;
    if (port != INPUT_PORT && port != OUTPUT_PORT) {
        return -EINVAL;
    }

    if (port == INPUT_PORT) {
        bufCount = mActualInputCount;
        bufSize = getInputSize();
        if (mDomain == V4L2_CODEC_TYPE_DECODER) {
            if (bufSize < mInputSizeOverWrite) {
                bufSize = mInputSizeOverWrite;
            }
        }
    } else {
        bufCount = mActualOutputCount;
        bufSize = getOutputSize();
    }

    for (int i = 0; i < bufCount; i++) {
        buf = allocateBuffer(i, port, bufSize);
        if (buf == nullptr) {
            return -EINVAL;
        }
        {
            if (port == INPUT_PORT) {
                std::unique_lock<std::mutex> lock(mInputBufLock);
                mInputBufs.push_back(buf);
            } else {
                std::unique_lock<std::mutex> lock(mOutputBufLock);
                mOutputBufs.push_back(buf);
            }
        }
    }

    return ret;
}

int V4l2Codec::freeBuffers(port_type port) {
    int ret = 0;
    struct v4l2_requestbuffers reqBufs;
    auto& bufs = port == OUTPUT_PORT ? mOutputBufs : mInputBufs;
    auto& pendingBuf =
        port == OUTPUT_PORT ? mPendingOutputBufs : mPendingInputBufs;
    auto& bufPool =
        port == OUTPUT_PORT ? mOutputBuffersPool : mInputBuffersPool;

    auto handleFreeBuffers = [&]() {
        while (!bufs.empty()) {
            auto buf = bufs.front();
            if (buf->m.planes) {
                free(buf->m.planes);
                buf->m.planes = nullptr;
            }
            bufs.pop_front();
        }
        while (!pendingBuf.empty()) {
            auto buf = pendingBuf.front();
            if (buf->m.planes) {
                free(buf->m.planes);
                buf->m.planes = nullptr;
            }
            pendingBuf.pop_front();
        }
        bufPool.clear();
    };

    if (port == OUTPUT_PORT) {
        std::unique_lock<std::mutex> lock(mOutputBufLock);
        LOGD("Freeing %d + %d output buffers\n", bufs.size(), pendingBuf.size());
        handleFreeBuffers();
    } else {
        std::unique_lock<std::mutex> lock(mInputBufLock);
        LOGD("Freeing %d + %d input buffers\n", bufs.size(), pendingBuf.size());
        handleFreeBuffers();
    }

    memset(&reqBufs, 0, sizeof(reqBufs));
    reqBufs.type = port == OUTPUT_PORT ? OUTPUT_MPLANE : INPUT_MPLANE;
    reqBufs.memory = mMemoryType;
    reqBufs.count = 0;
    ret = mV4l2Driver->reqBufs(&reqBufs);
    if (ret) {
        return ret;
    }
    return 0;
}

std::shared_ptr<v4l2_buffer> V4l2Codec::allocateBuffer(int index, port_type port, int bufSize) {
    std::shared_ptr<v4l2_buffer> buf = std::make_shared<v4l2_buffer>();
    struct v4l2_plane* plane;

    memset(buf.get(), 0, sizeof(struct v4l2_buffer));
    plane = (struct v4l2_plane*)malloc(sizeof(struct v4l2_plane) * VIDEO_MAX_PLANES);
    if (plane == nullptr) {
        return nullptr;
    }
    memset(plane, 0, sizeof(struct v4l2_plane) * VIDEO_MAX_PLANES);

    buf->type = port == INPUT_PORT ? INPUT_MPLANE : OUTPUT_MPLANE;
    buf->memory = mMemoryType;
    buf->index = index;
    buf->length = 1;
    buf->m.planes = plane;
    buf->flags = 0;
    memset(&buf->timestamp, 0, sizeof(buf->timestamp));

    if (mMemoryType == V4L2_MEMORY_DMABUF) {
        std::shared_ptr<DMABuffer> dmaBuf = nullptr;
        {
            int ret, bufFd = -1;
            ret = mV4l2Driver->AllocDMABuffer(bufSize, &bufFd);
            if (ret) {
                return nullptr;
            }
            dmaBuf = std::make_shared<DMABuffer>(bufSize, bufFd);
            close(bufFd);
        }
        if (port == INPUT_PORT) {
            mInputBuffersPool[index] = dmaBuf;
        } else {
            mOutputBuffersPool[index] = dmaBuf;
        }
    } else if (mMemoryType == V4L2_MEMORY_MMAP) {
        std::shared_ptr<MMAPBuffer> mmapBuf = std::make_shared<MMAPBuffer>();
        int ret = mV4l2Driver->AllocMMAPBuffer(mmapBuf, buf);
        if (ret)
        {
            LOGE("Allocate MMAP buffer failed.\n");
            return nullptr;
        }
        if (port == INPUT_PORT) {
            mInputBuffersPool[index] = mmapBuf;
        } else {
            mOutputBuffersPool[index] = mmapBuf;
        }
    }

    return buf;
}

int V4l2Codec::queueBuffer(std::shared_ptr<v4l2_buffer> buffer) {
    return mV4l2Driver->queueBuf(buffer.get());
}
