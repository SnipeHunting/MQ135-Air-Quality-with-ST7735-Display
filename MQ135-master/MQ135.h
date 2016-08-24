/**************************************************************************/
/*!
@file     MQ135.h
@author   G.Krocker (Mad Frog Labs)
@license  GNU GPLv3

First version of an Arduino Library for the MQ135 gas sensor
TODO: Review the correction factor calculation. This currently relies on
the datasheet but the information there seems to be wrong.

@section  HISTORY

v1.0 - First release
*/
/**************************************************************************/
#ifndef MQ135_H
#define MQ135_H
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

/// The load resistance on the board
#define RLOAD 10.0
/// Calibration resistance at atmospheric CO2 level
//#define RZERO 76.63
#define RZERO  6125 //4973 5672 4668.26 5292.8 6619.89 9937.62  8031.71 7223.11
/// Parameters for calculating ppm of CO2 from sensor resistance

#define PARA 116.6020682 // for CO2 ---MQ-135
#define PARB 2.769034857  // for CO2 ---MQ-135

//#define PARA 780.25597 // for LPG ---MQ-2
//#define PARB 1.634397  // for LPC ---MQ-2
//#define PARA 115.3653279 // for CO ---MQ-2
//#define PARB 2.772544  // for CO ---MQ-2
//#define PARA 1841.304171 // for Smoke ---MQ-2
//#define PARB 2.665252  // for Smoke ---MQ-2


/// Parameters to model temperature and humidity dependence
#define CORA 0.00035
#define CORB 0.02718
#define CORC 1.39538
#define CORD 0.0018

/// Atmospheric CO2 level for calibration purposes
#define ATMOCO2 400 //397.13

class MQ135 {
 private:
  uint8_t _pin;

 public:
  MQ135(uint8_t pin);
  float getCorrectionFactor(float t, float h);
  float getResistance();
  float getCorrectedResistance(float t, float h);
  float getPPM();
  float getCorrectedPPM(float t, float h);
  float getRZero();
  float getCorrectedRZero(float t, float h);
};
#endif
