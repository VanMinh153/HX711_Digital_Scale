#line 1 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\release\\version_1.0.1\\code\\HX711-HEDSPI.h"
/**
 * @brief       Library for HX711
 * @author     Nguyen Van Minh - SOICT-HUST
 * @ref        HX711-SOLDERED library for arduino
 */

#ifndef __HX711_HEDSPI__
#define __HX711_HEDSPI__

#include "Arduino.h"

// Gain 32 is for B+-, while others are for A+-
#define GAIN_128 25
#define GAIN_64 27
#define GAIN_32 26
#define getData() getData_H(Gain)

class HX711 {
public:
  HX711(byte DOUT, byte PD_SCK, byte Gain = GAIN_128, float Scale = 1.0f);
  void init();

  void setGain(byte Gain);
  void setScale(float Scale);
  void setZero(int32_t Zero);
  int32_t setZero();
  int32_t readDataHigh(byte Gain = GAIN_128, uint16_t check_freq = 100);
  int32_t getData_L(byte Gain = GAIN_128, uint16_t check_freq = 100);
  float getWeight();

private:
  byte DOUT;
  byte PD_SCK;
  byte Gain = GAIN_128;
  int32_t Zero = 0;
  float Scale = 1;
};

#endif
