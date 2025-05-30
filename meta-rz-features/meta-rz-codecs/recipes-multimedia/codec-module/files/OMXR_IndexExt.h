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
 * OMXR Extension header for index
 *
 * This file contains common definitions to access Renesas extension definations.
 *
 * \file OMXR_IndexExt.h
 */

#ifndef OMXR_Index_Ext_h
#define OMXR_Index_Ext_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <OMX_IndexExt.h>
#include <OMXR_Extension_h264.h>
#include <OMXR_Extension_h265.h>

enum {
    OMX_IndexParamVideoHevc = OMXR_MC_IndexParamVideoHevc,
    OMX_IndexConfigVideoHEVCIntraPeriod = OMXR_MC_IndexConfigVideoHEVCIntraPeriod,
};

#ifdef __cplusplus
}
#endif

#endif
/* File EOF */
