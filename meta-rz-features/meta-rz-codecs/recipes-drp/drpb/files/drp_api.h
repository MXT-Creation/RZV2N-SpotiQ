/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
* this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2022 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : drp_api.h
* Version      : -
* Description  : RZ/V2MA DRP library API header
***********************************************************************************************************************/

#ifndef DRP_LIB_API_H
#define DRP_LIB_API_H

#include <stdint.h>
/*****************************************
*  MACROS/DEFINES
******************************************/
#define CNV_MODE_YUV_PLANER         0
#define CNV_MODE_NV12               1
#define CNV_MODE_RGB888             2
#define CNV_MODE_BGR888             3

/* for YUV to NV color format (YUV2NV) conversion */
/* Input / Output color format */
#define YUV_IN_FORMAT_YUY2  (0)     // [Input] YUY2
#define YUV_IN_FORMAT_YVYU  (1)     // [Input] YVYU
#define YUV_IN_FORMAT_UYVY  (2)     // [Input] UYVY
#define YUV_IN_FORMAT_VYUY  (3)     // [Input] VYUY
#define YUV_OUT_FORMAT_NV12 (0)     // [Output] NV12
#define YUV_OUT_FORMAT_NV21 (1)     // [Output] NV21



/*****************************************
*  TYPE DEFINITION
******************************************/
/* API I/F for all DRPB */
typedef struct {
    int32_t  drp_b_driver_fd;

    /* DRPB Address Info */
    uint64_t DRP_BinAdr;
    uint64_t DrpT2LCircuitAdr;
    uint64_t DrpYUV2NVCircuitAdr;
    uint64_t DrpCropCircuitAdr;
    uint64_t DrpCopyCircuitAdr;
    uint64_t DrpDiscAdr;
    uint64_t DrpT2LParamAdr;
    uint64_t DrpYUV2NVParamAdr;
    uint64_t DrpCropParamAdr;
    uint64_t DrpCopyParamAdr;

    /* DRPB Binary Size Info */
    uint64_t DrpT2LBinSize;
    uint64_t DrpYUV2NVBinSize;
    uint64_t DrpCropBinSize;
    uint64_t DrpCopyBinSize;
} st_drp_info_t;

/* API I/F for Tile-to-linear conversion (vcd_drp_copy_XY2L) */
typedef struct {
    int xy_on_field;            //Whether the input picture is interlaced (0:Progressive, 1:Interlaced)
                                //NOTE:Interlaced is unevaluated as of 11/14 (Maybe it doesn't work correctly)
    int x_pic_size;             //Input picture size(width).Specify a multple of 64(= half of 1 tile)
    int y_pic_size;             //Input picture size(height) Specify a multple of 32(= 1 tile)
    int log2stride;             //Stride of input picture.Specify a power of 2 (For FHD, set 11(= 2 to the power o 11 (=2048)))

    int crop_top;               //Number of lines to crop (top). even number
    int crop_bot;               //Number of lines to crop (bottom). even number
    int crop_left;              //Number of pixels to crop (left side). 0 or multiples of 64
    int crop_right;             //Number of pixels to crop (right side). even number
    unsigned long y_top_addr;   //Starting address of input Y picture(TOP). If the input image is progressive, use only this.
    unsigned long y_bot_addr;   //Starting address of input Y picture(BOTTOM).
    unsigned long uv_top_addr;  //Starting address of input UV picture(TOP). If the input image is progressive , use only this.
    unsigned long uv_bot_addr;  //Starting address of input UV picture(BOTTOM).
    unsigned long l_addr[3];    //Starting address of output picture
                                    //YUV Planer : Y to l_addr[0], U to l_addr[1], V to l_addr[2]
                                    //NV12       : Y to l_addr[0], U/V to l_addr[1]
                                    //RGB or BGR : RGB or BGR to l_addr[0]
    int mode;                   //Output format (0:YUV Planer, 1:NV12, 2:RGB, 3:BGR)
} st_drp_t2l_t;

/* API I/F for YUVtoNV conversion (vcd_drp_yuv_to_nv) */
typedef struct{
    unsigned long src_adr;      //Starting address of input YUV picture (physical address)
    unsigned long dst_adr;      //Starting address of output NV picture (physical address)
    unsigned short img_size_w;  //Picture size(width). even number
    unsigned short img_size_h;  //Picture size(height). even number
    unsigned short src_format;  //Input picture format:0:YUY2, 1:YVYU, 2:UYVY, 3:VYUY
    unsigned short dst_format;  //Output picture format:0:NV12, 1:NV21
} st_drp_y2n_t;

/* API I/F for DRP_Crop */
typedef struct{
    unsigned long src_adr;      //Starting address of input picture (physical address)
    unsigned long dst_adr;      //Starting address of output picture (physical address)
    unsigned short src_width;   //Picture size(width). conditions 32 or more and multiples of 16
    unsigned short src_height;  //Picture size(height). conditions 16 or more and multiples of 8
    unsigned short src_format;  //Picture format (0:NV12 or NV21, 1:YUV Planer)
    unsigned short crop_left;   //Number of pixels to crop (left side). even number
    unsigned short crop_right;  //Number of pixels to crop (right side). even number
    unsigned short crop_top;    //Number of lines to crop (top). even number
    unsigned short crop_bottom; //Number of lines to crop (bottom). even number
} st_drp_crop_t;

/* API I/F for DRP_Copy */
typedef struct{
    unsigned long src_adr;      //Starting address of input picture (physical address)
    unsigned long dst_adr;      //Starting address of output picture (physical address)
    unsigned long copy_size;    //Copy size (byte). multiple of 32
} st_drp_copy_t;

/***************************************************************************/
/*    Function Prototypes                                                  */
/***************************************************************************/

/**************************************************************************
* Function Name : DRP_Config_init
* Description   : Enable tile-to-linear conversion, NAL search circuit,
*                 and NV12 conversion circuit.
*                 Run it only one time after startup
* Arguments     : drp_info - drpb infomation
* Return value  : 0:OK  -1:NG
**************************************************************************/
int32_t DRP_Config_init(st_drp_info_t * drp_info);

/**************************************************************************
* Function Name : DRP_Config_deinit
* Description   : Close DRP driver
* Arguments     : drp_info - drpb infomation
* Return value  : void
**************************************************************************/
void DRP_Config_deinit(st_drp_info_t * drp_info);

/**************************************************************************
* Function Name : vcd_drp_copy_XY2L
* Description   : DRP execution process for Tile-to-linear conversion.
*                 Write the parameters every time. If the parameter is fixed, 
*                 removing the process can speed it up a bit.
* Arguments     : p_xy  - parameters to pass to the DRP
*                 drp_info - drpb infomation
* Return value  : 0:OK  -1:NG  -2:NG(semaphore wait error)
**************************************************************************/
int32_t vcd_drp_copy_XY2L(st_drp_t2l_t * p_xy, st_drp_info_t * drp_info);

/**************************************************************************
* Function Name : vcd_drp_yuv_to_nv
* Description   : DRP execution process for YUVtoNV conversion..
*                 Write the parameters every time. If the parameter is fixed, 
*                 removing the process can speed it up a bit.
* Arguments     : p_y2n  - parameters to pass to the DRP
*                 drp_info - drpb infomation
* Return value  : 0:OK  -1:NG  -2:NG(semaphore wait error)
**************************************************************************/
int32_t vcd_drp_yuv_to_nv(st_drp_y2n_t * p_y2n, st_drp_info_t * drp_info);

/*****************************************
* Function Name : DRP_Crop
* Description   : DRP execution processing for Crop
* Arguments     : p_crop  - parameters to pass to the DRP
*                 drp_info - drpb infomation
* Return value  : 0:OK  -1:NG
******************************************/
int32_t DRP_Crop(st_drp_crop_t * p_crop, st_drp_info_t * drp_info);
/*****************************************
* Function Name : DRP_Copy
* Description   : DRP execution processing for copy
* Arguments     : p_copy  - parameters to pass to the DRP
*                 drp_info - drpb infomation
* Return value  : 0:OK  -1:NG
******************************************/
int32_t DRP_Copy(st_drp_copy_t * p_copy, st_drp_info_t * drp_info);

#endif /* DRP_LIB_API_H */
