/**
 * @brief       Library for HX711
 * @author     Nguyen Van Minh - SOICT-HUST
 */

#ifndef __HX711_HEDSPI__
#define __HX711_HEDSPI__

#include "Arduino.h"

#define HX711_FAIL 0x7FFFFF

// Gain 32 is for B+-, while others are for A+-
enum hx711_gain_t
{
  CHAN_A_GAIN_128 = 25,
  CHAN_A_GAIN_64 = 27,
  CHAN_B_GAIN_32 = 26
};

float toWeight(int data, hx711_gain_t gain = CHAN_A_GAIN_128);
//__________________________________________________________________________________________________
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

protected:
  uint8_t _dataPin;
  uint8_t _clockPin;
};

//__________________________________________________________________________________________________
class HX711 : public HX711Mini
{
public:
  HX711(uint8_t dataPin, uint8_t clockPin, hx711_gain_t gain = CHAN_A_GAIN_128);
  void setGain(hx711_gain_t gain_val);
  void setPowerDown(bool powerDown_val);
  void setTareA(int tareA_val);
  void setTareB(int tareB_val);
  void setScaleA(float scale_val);
  void setScaleB(float scale_val);

  void tareA();
  void tareB();
  float toWeight(int data, hx711_gain_t gain = CHAN_A_GAIN_128);
  float getWeight(hx711_gain_t gain = CHAN_A_GAIN_128);
  int readData(hx711_gain_t gain);
  int readData();

private:
  hx711_gain_t _gain;
  bool _powerDown = 0;
  int _tareA = 0;
  int _tareB = 0;
  float _scaleA = 1;
  float _scaleB = 1;
};
//__________________________________________________________________________________________________
// for channel A only
class HX711List : public HX711
{
public:
  HX711Mini *_hx711;
  uint8_t _size;

  HX711List(uint8_t hx711_num,
            uint8_t *dataPin, uint8_t *clockPin,
            hx711_gain_t gain = CHAN_A_GAIN_128);

  void begin();
  bool isReady();
  void powerDown();
  void powerUp();

  int readData(uint8_t hx711_num);
  int readDataAsync();
  int readDataSync();

private:
  hx711_gain_t _gain;
  bool _powerDown = 0;
  int _tareA = 0;
  int _tareB = 0;
  float _scaleA = 1;
  float _scaleB = 1;
  uint8_t _errRead = 0;
};

#endif