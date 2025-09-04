/*
 * Copyright(C) 2024 Renesas Electronics Corporation. All Rights Reserved.
 * RENESAS ELECTRONICS CONFIDENTIAL AND PROPRIETARY
 * This program must be used solely for the purpose for which
 * it was furnished by Renesas Electronics Corporation.
 * No part of this program may be reproduced or disclosed to
 * others, in any form, without the prior written permission
 * of Renesas Electronics Corporation.
 */
/**
 * OMXR Extension header for video common
 *
 * This file contains definitions to access video items in Renesas extension definations.
 *
 * \file OMXR_IndexExt.h
 */

#ifndef OMXR_Video_Ext_h
#define OMXR_Video_Ext_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <OMX_VideoExt.h>
#include <OMXR_Extension_h264.h>
#include <OMXR_Extension_h265.h>

enum {
    OMX_VIDEO_CodingHEVC = OMXR_MC_VIDEO_CodingHEVC,
};

typedef enum OMX_VIDEO_HEVCPROFILETYPE {
    OMX_VIDEO_HEVCProfileUnknown      = OMXR_MC_VIDEO_HEVCProfileUnknown,
    OMX_VIDEO_HEVCProfileMain         = OMXR_MC_VIDEO_HEVCProfileMain,    /**< Main profile */
    OMX_VIDEO_HEVCProfileMain10       = OMXR_MC_VIDEO_HEVCProfileMain10,    /**< Main 10 profile */
    OMX_VIDEO_HEVCProfileMainStillPic = OMXR_MC_VIDEO_HEVCProfileMainStillPic,    /**< Main Still Picture profile */
} OMX_VIDEO_HEVCPROFILETYPE;

typedef enum OMX_VIDEO_HEVCLEVELTYPE {
    OMX_VIDEO_HEVCLevelUnknown    = OMXR_MC_VIDEO_HEVCLevelUnknown   ,
    OMX_VIDEO_HEVCMainTierLevel1  = OMXR_MC_VIDEO_HEVCMainTierLevel1 ,              /**< Level 1 */
    OMX_VIDEO_HEVCHighTierLevel1  = OMXR_MC_VIDEO_HEVCHighTierLevel1 ,
    OMX_VIDEO_HEVCMainTierLevel2  = OMXR_MC_VIDEO_HEVCMainTierLevel2 ,              /**< Level 2 */
    OMX_VIDEO_HEVCHighTierLevel2  = OMXR_MC_VIDEO_HEVCHighTierLevel2 ,
    OMX_VIDEO_HEVCMainTierLevel21 = OMXR_MC_VIDEO_HEVCMainTierLevel21,             /**< Level 2.1 */
    OMX_VIDEO_HEVCHighTierLevel21 = OMXR_MC_VIDEO_HEVCHighTierLevel21,
    OMX_VIDEO_HEVCMainTierLevel3  = OMXR_MC_VIDEO_HEVCMainTierLevel3 ,             /**< Level 3 */
    OMX_VIDEO_HEVCHighTierLevel3  = OMXR_MC_VIDEO_HEVCHighTierLevel3 ,
    OMX_VIDEO_HEVCMainTierLevel31 = OMXR_MC_VIDEO_HEVCMainTierLevel31,            /**< Level 3.1 */
    OMX_VIDEO_HEVCHighTierLevel31 = OMXR_MC_VIDEO_HEVCHighTierLevel31,
    OMX_VIDEO_HEVCMainTierLevel4  = OMXR_MC_VIDEO_HEVCMainTierLevel4 ,            /**< Level 4 */
    OMX_VIDEO_HEVCHighTierLevel4  = OMXR_MC_VIDEO_HEVCHighTierLevel4 ,
    OMX_VIDEO_HEVCMainTierLevel41 = OMXR_MC_VIDEO_HEVCMainTierLevel41,           /**< Level 4.1 */
    OMX_VIDEO_HEVCHighTierLevel41 = OMXR_MC_VIDEO_HEVCHighTierLevel41,
    OMX_VIDEO_HEVCMainTierLevel5  = OMXR_MC_VIDEO_HEVCMainTierLevel5 ,           /**< Level 5 */
    OMX_VIDEO_HEVCHighTierLevel5  = OMXR_MC_VIDEO_HEVCHighTierLevel5 ,
    OMX_VIDEO_HEVCMainTierLevel51 = OMXR_MC_VIDEO_HEVCMainTierLevel51,
    OMX_VIDEO_HEVCHighTierLevel51 = OMXR_MC_VIDEO_HEVCHighTierLevel51,
    OMX_VIDEO_HEVCMainTierLevel52 = OMXR_MC_VIDEO_HEVCMainTierLevel52,
    OMX_VIDEO_HEVCHighTierLevel52 = OMXR_MC_VIDEO_HEVCHighTierLevel52,
    OMX_VIDEO_HEVCMainTierLevel6  = OMXR_MC_VIDEO_HEVCMainTierLevel6 ,
    OMX_VIDEO_HEVCHighTierLevel6  = OMXR_MC_VIDEO_HEVCHighTierLevel6 ,
    OMX_VIDEO_HEVCMainTierLevel61 = OMXR_MC_VIDEO_HEVCMainTierLevel61,
    OMX_VIDEO_HEVCHighTierLevel61 = OMXR_MC_VIDEO_HEVCHighTierLevel61,
    OMX_VIDEO_HEVCMainTierLevel62 = OMXR_MC_VIDEO_HEVCMainTierLevel62,
    OMX_VIDEO_HEVCHighTierLevel62 = OMXR_MC_VIDEO_HEVCHighTierLevel62,
} OMX_VIDEO_HEVCLEVELTYPE;

typedef enum OMX_VIDEO_HEVLOOPFILTERTYPE {
    OMX_VIDEO_HEVLoopFilterEnable = OMXR_MC_VIDEO_HEVLoopFilterEnable,
    OMX_VIDEO_HEVLoopFilterDisable = OMXR_MC_VIDEO_HEVLoopFilterDisable,
    OMX_VIDEO_HEVLoopFilterDisableSliceBoundary = OMXR_MC_VIDEO_HEVLoopFilterDisableSliceBoundary,
} OMX_VIDEO_HEVLOOPFILTERTYPE;

typedef struct OMXR_MC_VIDEO_PARAM_HEVCTYPE OMX_VIDEO_PARAM_HEVCTYPE;
typedef struct OMXR_MC_VIDEO_CONFIG_HEVCINTRAPERIOD OMX_VIDEO_CONFIG_HEVCINTRAPERIOD;

#ifdef __cplusplus
}
#endif

#endif
/* File EOF */
