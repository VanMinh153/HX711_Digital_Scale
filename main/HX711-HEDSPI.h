/**
 * @brief       Library for HX711
 * @author     Nguyen Van Minh - SOICT-HUST
 */

#ifndef __HX711_HEDSPI__
#define __HX711_HEDSPI__

#include "Arduino.h"

// Gain 32 is for B+-, while others are for A+-
#define GAIN_128 25
#define GAIN_64 27
#define GAIN_32 26

class HX711 {
public:
  HX711(byte DOUT, byte PD_SCK, byte Gain = GAIN_128, float Scale = 1.0f);
  void init();


  void setGain(byte Gain);
  void setScale(float Scale);
  void setZero(int32_t Zero);
  int32_t setZero();
  int32_t getData();
  int32_t getData_H(byte Gain, uint16_t check_freq = 100);
  int32_t getData_L(byte Gain, uint16_t check_freq = 100);
  float getWeight();

private:
  byte DOUT;
  byte PD_SCK;
  byte Gain = GAIN_128;
  int32_t Zero = 0;
  float Scale = 1;
};

class HX711x4_Async {
public:
  HX711x4_Async();
  HX711x4_Async(byte DOUT1, byte PD_SCK1, byte DOUT2, byte PD_SCK2, byte DOUT3, byte PD_SCK3, byte DOUT4, byte PD_SCK4, byte Gain = GAIN_128, float Scale = 1.0f);
  void init(byte acd_4, byte DOUT, byte PD_SCK, byte Gain = GAIN_128, float Scale = 1.0f);
  void init();

  void setGain(byte Gain);
  void setScale(float Scale);
  void setZero(int32_t Zero);
  HX711 hx711[4];

private:
  byte Gain = GAIN_128;
};

#endif
