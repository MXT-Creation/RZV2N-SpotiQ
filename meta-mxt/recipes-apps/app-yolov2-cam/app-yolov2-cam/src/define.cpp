#include "define.h"

position_type position_11;
position_type position_12 ;
position_type position_13 ;
position_type position_14 ;
position_type position_15 ;
position_type position_16 ;

double posDegres_11 = 0;
double posDegres_12 = 0;
double posDegres_13 = 0;
double posDegres_14 = 0;
double posDegres_15 = 0;
double posDegres_16 = 0;
//I2C bus pointer to be initialized
const char *i2c_bus = "/dev/i2c-3";
int i2c_file;
// For the four corners in pixel coordinates (H, W):
 std::atomic<double> gTopLeft_H(253),    gTopLeft_W(876);
 std::atomic<double> gTopRight_H(253),   gTopRight_W(1171);
 std::atomic<double> gBottomLeft_H(542), gBottomLeft_W(860);
 std::atomic<double> gBottomRight_H(542),gBottomRight_W(1215);
// For calibration state:
 std::atomic<bool> gIsCalibrating(false);
 std::atomic<int> gCalibrationStep(0);