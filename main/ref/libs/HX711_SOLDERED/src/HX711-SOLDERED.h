/**
 **************************************************
 *
 * @file        HX711.h
 * @brief       Arduino library for hx711 breakout
 *
 *
 *
 * @authors     @ soldered.com @Robert Peric
 ***************************************************/
#ifndef __HX711_SOLDERED__
#define __HX711_SOLDERED__

#include "Arduino.h"
#include "libs/Generic-easyC/easyC.h"


// Gain 32 is for B+-, while others are for A+-
#define GAIN_128 25
#define GAIN_64  27
#define GAIN_32  26

class HX711 : public EasyC
{
  public:
    HX711(int dout, int pd_sck);
    HX711();

    void setToZerro();
    void setScale(float scale);
    void setGain(uint8_t gain);
    uint32_t getData();
    uint32_t readDataNative();
    float getWeight();

  protected:
    void initializeNative();

  private:
    int _dout;
    int _pd_sck;
    uint8_t _gain = GAIN_128;
    uint32_t _zerroConstant = 0;
    float _scaleConstant = 1;
};

#endif
