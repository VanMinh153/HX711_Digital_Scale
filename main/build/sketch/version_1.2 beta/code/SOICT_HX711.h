#line 1 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\version_1.2 beta\\code\\SOICT_HX711.h"
/**
 * @brief      Library for HX711
 * @author     Nguyen Van Minh - SOICT-HUST
 */

#ifndef SOICT_HX711_H
#define SOICT_HX711_H

#include <Arduino.h>
// #include "config.h"
#define HX711_FAIL 0xFFFF

// Gain 32 is for B+-, while others are for A+-
enum hx711_gain_t
{
  CHAN_A_GAIN_128 = 25,
  CHAN_A_GAIN_64 = 27,
  CHAN_B_GAIN_32 = 26
};
#define HX711_DEFAULT_GAIN CHAN_A_GAIN_128
//--------------------------------------------------------------------------------------------------
class HX711Mini
{
public:
  HX711Mini();
  HX711Mini(uint8_t dataPin, uint8_t clockPin);
  void begin();
  void powerDown();
  void powerUp();
  bool isReady();
  int readData(hx711_gain_t gain);

  uint8_t dataPin;
  uint8_t clockPin;
};

//--------------------------------------------------------------------------------------------------
class HX711 : public HX711Mini
{
public:
  HX711();
  HX711(uint8_t dataPin, uint8_t clockPin, hx711_gain_t gain = HX711_DEFAULT_GAIN);
  void setGain(hx711_gain_t gain_val);
  void setPowerDown(bool powerDown_val);

  void setTare(int tare_val, hx711_gain_t gain = HX711_DEFAULT_GAIN);
  void setScale(float scale_val, hx711_gain_t gain = HX711_DEFAULT_GAIN);
  void tare(hx711_gain_t gain = HX711_DEFAULT_GAIN);
  float getWeight(hx711_gain_t gain = HX711_DEFAULT_GAIN);
  int readData(hx711_gain_t gain);
  int readData();

protected:
  hx711_gain_t gain = HX711_DEFAULT_GAIN;
  hx711_gain_t setting_gain = HX711_DEFAULT_GAIN;
  bool _powerDown = 0;
  int _tareA_128 = 0;
  int _tareB = 0;
  float scaleA_128 = 1;
  float scaleB = 1;
};
//--------------------------------------------------------------------------------------------------
// for channel A only
class HX711List
{
public:
  HX711List(uint8_t hx711_num,
            uint8_t *dataPin, uint8_t *clockPin,
            hx711_gain_t gain = CHAN_A_GAIN_128);
  void begin();
  bool isReady();
  void powerDown();
  void powerUp();
  void setGain(hx711_gain_t gain_val);

  int readData(uint8_t hx711_num);
  int readData();
  int readDataAsync(hx711_gain_t gain_val);
  int readDataSync(hx711_gain_t gain_val);

  uint8_t errRead = 0;
  int DataUnitMax; // for channel A gain 128

protected:
  HX711Mini *hx711;
  uint8_t Size;
  hx711_gain_t gain = CHAN_A_GAIN_128;
  bool _powerDown = 0;
  bool SyncMode;
  uint8_t clockPin; // for sync mode only
};

#endif
