/***********************************************************************************************************************
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : define.h
* Version      : 1.00
* Description  : RZ/V2H DRP-AI Sample Application for Lightnet YOLOv2 with MIPI/USB Camera
***********************************************************************************************************************/

#ifndef DEFINE_MACRO_H
#define DEFINE_MACRO_H

/*****************************************
* includes
******************************************/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <vector>
#include <map>
#include <fstream>
#include <math.h>
#include <iomanip>
#include <cstring>
#include <float.h>
#include <atomic>
#include <semaphore.h>
#include <numeric>

/*****************************************
* Macro for YOLOv2
******************************************/
/* Input Camera support */
/* n = 0: USB Camera, n = 1: eCAM22 */
#define INPUT_CAM_TYPE 1

/* Output Camera Size */
#define CAM_INPUT_FHD
#define IMAGE_OUTPUT_FHD
#define MIPI_CAM_RES "1920x1080"
//#define CAM_INPUT_VGA
/* Display AI frame rate */
#define DISP_AI_FRAME_RATE

/*****************************************
* YOLOv2 Model Configuration
******************************************/
/* Directory name of DRP-AI Object files */
const static std::string drpai_prefix0 = "yolov2_cam";

/* Anchor box information (Updated to match training configuration) */
//const static double anchors[] = 
//{
//    1.3221, 1.73145,
//    3.19275, 4.00944,
//    5.05587, 8.09892,
//    9.47112, 4.84053,
//    11.2364, 10.0071
//};
const static double anchors[] = 
{
	0.57273, 0.677385,
	1.87446, 2.06253,
	3.33843, 5.47434,
	7.88282, 3.52778,
	9.77052, 9.16828
};
/* Number of class to be detected */
#define NUM_CLASS                   (1)
/* Number of grids in the image */
#define NUM_GRID_X  (13)
#define NUM_GRID_Y  (13)

/* Number for [region] layer num parameter */
#define NUM_BB  (5)

/* Detection Thresholds */
#define TH_PROB                     (0.5f)
#define TH_NMS                      (0.5f)
/* Size of input image to the model */
#define MODEL_IN_W                  (416)
#define MODEL_IN_H                  (416)
#define INF_OUT_SIZE                ((NUM_CLASS + 5)* NUM_BB * NUM_GRID_X * NUM_GRID_Y)


/*****************************************
* DRP-AI Configuration
******************************************/
/* Maximum DRP-AI Timeout threshold */
#define DRPAI_TIMEOUT (5)

/* DRP Clock Configurations */
static constexpr uint32_t kDefaultDrpClockDivider{3};
static constexpr uint32_t kDefaultAiMacClockDivider{5};

/*****************************************
* Camera Settings
******************************************/
#ifdef CAM_INPUT_VGA
    #define CAM_IMAGE_WIDTH  (640)
    #define CAM_IMAGE_HEIGHT (480)
#else
    #define CAM_IMAGE_WIDTH  (1920)
    #define CAM_IMAGE_HEIGHT (1080)
#endif

#define CAM_IMAGE_CHANNEL_YUY2 (2) // Camera input format
#define CAM_IMAGE_SIZE (CAM_IMAGE_WIDTH * CAM_IMAGE_HEIGHT * CAM_IMAGE_CHANNEL_YUY2)

/*Camera:: Capture Information */
#if INPUT_CAM_TYPE == 1
#define CAP_BUF_NUM                 (6)
#define INPUT_CAM_NAME              "MIPI Camera"
#else /* INPUT_CAM_TYPE */
#define CAP_BUF_NUM                 (3)
#define INPUT_CAM_NAME              "USB Camera"
#endif /* INPUT_CAM_TYPE */

/*DRP-AI Input image information*/
#define DRPAI_IN_WIDTH              (CAM_IMAGE_WIDTH)
#define DRPAI_IN_HEIGHT             (CAM_IMAGE_HEIGHT)
#define DRPAI_IN_CHANNEL_YUY2       (CAM_IMAGE_CHANNEL_YUY2)

/*Wayland:: Wayland Information */
#ifdef IMAGE_OUTPUT_HD
#define IMAGE_OUTPUT_WIDTH          (1280)
#define IMAGE_OUTPUT_HEIGHT         (720)
#else /* IMAGE_OUTPUT_FHD */
#define IMAGE_OUTPUT_WIDTH          (1920)
#define IMAGE_OUTPUT_HEIGHT         (1080)
#endif


#define DRPAI_OUT_WIDTH             (IMAGE_OUTPUT_WIDTH)
#define DRPAI_OUT_HEIGHT            (IMAGE_OUTPUT_HEIGHT)

#define IMAGE_CHANNEL_BGR (3)
#define WL_BUF_NUM        (2)

/*****************************************
* Memory Configuration
******************************************/
#define IMG_AREA_ORG_ADDRESS  (0xD0000000)  /* Do not modify */
#define IMG_AREA_CNV_ADDRESS  (0x58000000)  /* CMA area */
#define IMG_AREA_SIZE         (0x20000000)  /* CMA area size */

/*****************************************
* Image Processing Constants
******************************************/
#define CHAR_SCALE_LARGE        (0.8)
#define CHAR_SCALE_SMALL        (0.7)
#define CHAR_THICKNESS          (2)
#define LINE_HEIGHT             (30)  /* pixels */
#define LINE_HEIGHT_OFFSET      (20)  /* pixels */
#define TEXT_WIDTH_OFFSET       (10)  /* pixels */
#define BOX_LINE_SIZE           (3)   /* pixels */
#define BOX_HEIGHT_OFFSET       (30)  /* pixels */
#define BOX_TEXT_HEIGHT_OFFSET  (8)   /* pixels */
#define CHAR_SCALE_FONT         (0.8)

#define WHITE_DATA (0xFFFFFFu)
#define BLACK_DATA (0x000000u)

/*****************************************
* Timer Related Settings
******************************************/
#define CAPTURE_TIMEOUT         (20)  /* seconds */
#define AI_THREAD_TIMEOUT       (20)  /* seconds */
#define DISPLAY_THREAD_TIMEOUT  (20)  /* seconds */
#define KEY_THREAD_TIMEOUT      (5)   /* seconds */
#define TIME_COEF               (1)

/*****************************************
* DRP-AI Buffer Sizes
******************************************/
#define BUF_SIZE (1024)

/*****************************************
* Helper Macros
******************************************/
#define SIZE_OF_ARRAY(array) (sizeof(array)/sizeof(array[0]))

/*****************************************
* Index Access for DRP-AI Paths
******************************************/
#define INDEX_D  (0)
#define INDEX_C  (1)
#define INDEX_P  (2)
#define INDEX_A  (3)
#define INDEX_W  (4)
#define INDEX_AC (5)
#define INDEX_AP (6)
#define INDEX_APC (7)

/*****************************************
* Image Drawing Constants
******************************************/
#define FONTDATA_WIDTH  (6)
#define FONTDATA_HEIGHT (8)


/*****************************************
* I2C Commands
******************************************/
//postion in 2 bytes for i2c message
union position_type
{
uint16_t positon_16Bits;
uint8_t   position_8Bits[2];
};
extern position_type position_11;
extern position_type position_12 ;
extern position_type position_13 ;
extern position_type position_14 ;
extern position_type position_15 ;
extern position_type position_16 ;

extern double posDegres_11;
extern double posDegres_12;
extern double posDegres_13;
extern double posDegres_14;
extern double posDegres_15;
extern double posDegres_16;

//I2C bus pointer to be initialized
extern const char *i2c_bus;
extern int i2c_file;


#define MOTOR6 0x16
#define MOTOR5 0x15
#define MOTOR4 0x14
#define MOTOR3 0x13
#define MOTOR2 0x12
#define MOTOR1 0x11
// Define motor lengths
#define L0 (12)
#define LENGHT1 (8.5) // Length of motor 2 arm (cm)
#define LENGHT2 (8.5)  // Length of motor 3 arm (cm)
#define LENGHT3 (19) // Length of motor 4 arm (cm)
#define BGRA_CHANNEL                (4)
// Robot movement in progress
extern std::atomic<bool> robot_is_moving;
// For calibration state:
extern std::atomic<bool> gIsCalibrating;
extern std::atomic<int> gCalibrationStep;
// For the four corners in pixel coordinates (H, W):
extern std::atomic<double> gTopLeft_H,    gTopLeft_W;
extern std::atomic<double> gTopRight_H,   gTopRight_W;
extern std::atomic<double> gBottomLeft_H, gBottomLeft_W;
extern std::atomic<double> gBottomRight_H,gBottomRight_W;

const static std::string cameraCalibrationFile  = "cameraCalibration.json";

#endif /* DEFINE_MACRO_H */