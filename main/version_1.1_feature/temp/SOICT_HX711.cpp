/**
 * @brief      Library for HX711
 * @author     Nguyen Van Minh - SOICT - HUST
 */

#include "SOICT_HX711.h"

// #define powerUp()

// HX711Mini class
HX711Mini::HX711Mini() {}

HX711Mini::HX711Mini(uint8_t dataPin, uint8_t clockPin)
    : _dataPin(dataPin), _clockPin(clockPin) {}

void HX711Mini::begin()
{
  pinMode(_clockPin, OUTPUT);
  pinMode(_dataPin, INPUT);
  digitalWrite(_clockPin, LOW);
}

inline void HX711Mini::powerDown()
{
  digitalWrite(_clockPin, HIGH);
  // time to power down mode: min 60us
  delayMicroseconds(65);
}

inline void HX711Mini::powerUp()
{
  digitalWrite(_clockPin, LOW);
}

inline bool HX711Mini::isReady()
{
  return digitalRead(_dataPin) == LOW;
}

inline int HX711Mini::readData(hx711_gain_t gain)
{
  const uint8_t max_wait = 105; // max wait time (105ms)
  unsigned long timer = millis();
  while (isReady() == false && millis() - timer < max_wait)
    delay(1);

  // time wait after ready signal: min 0.1us
  delayMicroseconds(1);
  if (isReady() == false)
    return HX711_FAIL;

  int data = 0;
  for (int i = 0; i < 24; i++)
  { // response time: max 0.1us
    // clock pin high time: min 0.2us, typ 1us, max 50us
    // clock pin low time: min 0.2us, typ 1us
    digitalWrite(_clockPin, HIGH);
    delayMicroseconds(1);
    data = (data << 1) | digitalRead(_dataPin);
    digitalWrite(_clockPin, LOW);
    delayMicroseconds(1);
  }

  for (int i = 0; i < gain - 24; i++)
  {
    digitalWrite(_clockPin, HIGH);
    delayMicroseconds(1);
    digitalWrite(_clockPin, LOW);
    delayMicroseconds(1);
  }
  // data pin will be HIGH after read process
  if (digitalRead(_dataPin) == LOW)
    return HX711_FAIL;

  // Overflow and Underflow
  if (data == 0x7FFFFF || data == 0x800000)
    return HX711_FAIL;

  if ((data&BIT23) > 0)
    data |= 0xFF000000;
  
  return data;
}

//----------------------------------------------------------------------------------------------------------------------
// HX711 class
HX711::HX711()
    : HX711Mini() {}

HX711::HX711(uint8_t dataPin, uint8_t clockPin, hx711_gain_t gain)
    : HX711Mini(dataPin, clockPin), _gain(gain) {}

void HX711::setPowerDown(bool powerDown_val)
{
  _powerDown = powerDown_val;
  if (_powerDown == 1)
    powerDown();
  else
    powerUp();
}

void HX711::setGain(hx711_gain_t gain)
{
  _gain = gain;
}

void HX711::setTareA(int tareA_val)
{
  _tareA = tareA_val;
}

void HX711::setTareB(int tareB_val)
{
  _tareB = tareB_val;
}

void HX711::setScaleA(float scale)
{
  _scaleA = scale;
}

void HX711::setScaleB(float scale)
{
  _scaleB = scale;
}

void HX711::tareA()
{
  _tareA = _gain == CHAN_A_GAIN_128 ? readData(CHAN_A_GAIN_128) : readData(CHAN_A_GAIN_64);
}

void HX711::tareB()
{
  _tareB = readData(CHAN_B_GAIN_32);
}

float HX711::toWeight(int data, hx711_gain_t gain)
{
  if (gain == CHAN_A_GAIN_128)
    return (float)(data - _tareA) / _scaleA;
  else if (gain == CHAN_A_GAIN_64)
    return (float)(data - _tareA) / (_scaleA / 2);
  else
    return (float)(data - _tareB) / _scaleB;
}

float HX711::getWeight(hx711_gain_t gain)
{
  if (gain == CHAN_A_GAIN_128)
    return (float)(readData(CHAN_A_GAIN_128) - _tareA) / _scaleA;
  else if (gain == CHAN_A_GAIN_64)
    return (float)(readData(CHAN_A_GAIN_64) - _tareA) / (_scaleA / 2);
  else
    return (float)(readData(CHAN_B_GAIN_32) - _tareB) / _scaleB;
}

int HX711::readData()
{
  return readData(_gain);
}

int HX711::readData(hx711_gain_t gain)
{
  if (_powerDown == 1)
    powerUp();

  int data = HX711Mini::readData(gain);
  
  // FIXBUG
  if (data == HX711_FAIL)
    Serial.println("HX711_Failed : ");

  if (_powerDown == 1)
    powerDown();

  return data;
}

//----------------------------------------------------------------------------------------------------------------_____
// HX711List class
HX711List::HX711List(uint8_t hx711_num, uint8_t *dataPin, uint8_t *clockPin, hx711_gain_t gain)
    : HX711(), _size(hx711_num)
{
  _gain = gain;
  _hx711 = new HX711Mini[_size];
  for (int i = 0; i < _size; i++)
    _hx711[i] = HX711(dataPin[i], clockPin[i], gain);
}

void HX711List::begin()
{
  for (int i = 0; i < _size; i++)
    _hx711[i].begin();
  readData();
}

bool HX711List::isReady()
{
  for (int i = 0; i < _size; i++)
    if (_hx711[i].isReady() == false)
      return false;
  return true;
}

void HX711List::powerDown()
{
  for (int i = 0; i < _size; i++)
    _hx711[i].powerDown();
}

void HX711List::powerUp()
{
  for (int i = 0; i < _size; i++)
    _hx711[i].powerUp();
}

int HX711List::readData(uint8_t hx711_num)
{
  return _hx711[hx711_num].readData(_gain);
}

int HX711List::readData()
{
  if ()
}

int HX711List::readDataAsync()
{
  if (_powerDown == 1)
    powerUp();

  int data = 0;
  for (int i = 0; i < _size; i++)
    data += _hx711[i].readData(_gain);

  if (_powerDown == 1)
    powerDown();
  return data;
}

// int HX711List::readDataSync()
// {
//   const uint8_t max_wait = 105; // max wait time (105ms)

//   if (_powerDown == 1)
//     powerUp();

//   unsigned long timer = millis();
//   while (isReady() == false && millis() - timer < max_wait)
//     delay(1);

//   // time wait after ready signal: min 0.1us
//   delayMicroseconds(1);
//   if (isReady() == false)
//     return HX711_FAIL;

//   int data = 0;
//   for (int i = 0; i < 24; i++)
//   { // response time: max 0.1us
//     // clock pin high time: min 0.2us, typ 1us, max 50us
//     // clock pin low time: min 0.2us, typ 1us
//     digitalWrite(_clockPin, HIGH);
//     delayMicroseconds(1);
//     data = (data << 1) | digitalRead(_dataPin);
//     digitalWrite(_clockPin, LOW);
//     delayMicroseconds(1);
//   }

//   for (int i = 0; i < gain - 24; i++)
//   {
//     digitalWrite(_clockPin, HIGH);
//     delayMicroseconds(1);
//     digitalWrite(_clockPin, LOW);
//     delayMicroseconds(1);
//   }
//   // data pin will be HIGH after read process
//   if (digitalRead(_dataPin) == LOW)
//     return HX711_FAIL;

//   if (data & BIT23 > 0)
//     data |= 0xFF000000;

//   if (_powerDown == 1)
//     powerDown();

//   return data;
// }

