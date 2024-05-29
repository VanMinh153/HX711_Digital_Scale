#line 1 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\release\\version_1_lcd\\HX711-HEDSPI.h"
/**
 * @brief       Library for HX711
 *
 * @author     Nguyen Van Minh - SOICT-HUST
 * @ref        HX711-SOLDERED - @soldered.com @Robert Peric
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
  HX711(uint8_t DOUT, uint8_t PD_SCK, uint8_t Gain = GAIN_128, float Scale = 1.0f);
  void init();

  void setGain(uint8_t Gain);
  void setScale(float Scale);
  void setZero(int32_t Zero);
  int32_t setZero();
  int32_t getData();
  float getWeight();

private:
  uint8_t DOUT;
  uint8_t PD_SCK;
  uint8_t Gain = GAIN_128;
  int32_t Zero = 0;
  float Scale = 1;
};

#endif
