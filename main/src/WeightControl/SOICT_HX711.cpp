/**
 * @brief      Library for HX711
 * @author     Nguyen Van Minh - SOICT - HUST
 */

#include "SOICT_HX711.h"

// #define powerUp()

// HX711Mini class
HX711Mini::HX711Mini() {}

HX711Mini::HX711Mini(uint8_t dataPin, uint8_t clockPin)
    : dataPin(dataPin), clockPin(clockPin) {}

void HX711Mini::begin()
{
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, INPUT);
  digitalWrite(clockPin, LOW);
}

inline void HX711Mini::powerDown()
{
  digitalWrite(clockPin, HIGH);
  // time to power down mode: min 60us
  delayMicroseconds(65);
}

inline void HX711Mini::powerUp()
{
  digitalWrite(clockPin, LOW);
}

inline bool HX711Mini::isReady()
{
  return digitalRead(dataPin) == LOW;
}

// Read data from HX711 and set gain after read
int HX711Mini::readData(hx711_gain_t gain_val)
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
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(1);
    data = (data << 1) | digitalRead(dataPin);
    digitalWrite(clockPin, LOW);
    delayMicroseconds(1);
  }

  for (int i = 0; i < gain_val - 24; i++)
  {
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(1);
    digitalWrite(clockPin, LOW);
    delayMicroseconds(1);
  }

  // data pin will be HIGH after read process
  if (digitalRead(dataPin) == LOW)
    return HX711_FAIL;

  // Overflow and Underflow
  if (data == 0x7FFFFF || data == 0x800000)
    return HX711_FAIL;

  if ((data & BIT23) > 0)
    data |= 0xFF000000;

  return data;
}

//----------------------------------------------------------------------------------------------------------------------
// HX711 class
HX711::HX711()
    : HX711Mini() {}

HX711::HX711(uint8_t dataPin, uint8_t clockPin, hx711_gain_t gain_val)
    : HX711Mini(dataPin, clockPin), gain(gain_val) {}

void HX711::setPowerDown(bool powerDown_val)
{
  _powerDown = powerDown_val;
  if (_powerDown == 1)
    powerDown();
  else
    powerUp();
}

void HX711::setGain(hx711_gain_t gain_val)
{
  gain = gain_val;
  setting_gain = gain;
  readData(gain);
}

void HX711::setTare(int tare_val, hx711_gain_t gain_val)
{
  if (gain == CHAN_A_GAIN_128)
    _tareA_128 = tare_val;
  else if (gain == CHAN_A_GAIN_64)
    _tareA_128 = tare_val * 2;
  else
    _tareB = tare_val;
}

void HX711::setScale(float scale_val, hx711_gain_t gain_val)
{
  if (gain == CHAN_A_GAIN_128)
    scaleA_128 = scale_val;
  else if (gain == CHAN_A_GAIN_64)
    scaleA_128 = scale_val * 2;
  else
    scaleB = scale_val;
}

void HX711::tare(hx711_gain_t gain_val)
{
  if (gain == CHAN_A_GAIN_128 || gain == CHAN_A_GAIN_64)
    _tareA_128 = readData(CHAN_A_GAIN_128);
  else
    _tareB = readData(CHAN_B_GAIN_32);
}

float HX711::getWeight(hx711_gain_t gain_val)
{
  if (gain_val == CHAN_A_GAIN_128)
    return (float)(readData(CHAN_A_GAIN_128) - _tareA_128) / scaleA_128;
  else if (gain_val == CHAN_A_GAIN_64)
    return (float)(readData(CHAN_A_GAIN_64) - _tareA_128 / 2) / (scaleA_128 / 2);
  else
    return (float)(readData(CHAN_B_GAIN_32) - _tareB) / scaleB;
}

int HX711::readData()
{
  return readData(gain);
}

// Read data from HX711 and set gain after read
int HX711::readData(hx711_gain_t gain_val)
{
  if (_powerDown == 1)
    powerUp();

  int data;
  if (gain_val == setting_gain)
    data = HX711Mini::readData(gain_val);
  else
  {
    HX711Mini::readData(gain_val);
    data = HX711Mini::readData(gain_val);
    setting_gain = gain_val;
  }

#if defined(DEBUG_MODE)
  // Serial.print("`DATA`= " + String(data) + " | ");
  // if (data == HX711_FAIL)
  //   Serial.print("`READ_FAIL`; ");
#endif

  if (_powerDown == 1)
    powerDown();

  return data;
}

//----------------------------------------------------------------------------------------------------------------_____
// HX711List class
HX711List::HX711List(uint8_t size_val, uint8_t *dataPin_val, uint8_t *clockPin_val, hx711_gain_t gain_val)
    : Size(size_val)
{
  gain = gain_val;
  hx711 = new HX711Mini[Size];

  for (int i = 0; i < Size; i++)
    hx711[i] = HX711(dataPin_val[i], clockPin_val[i], gain_val);

  bool flag = 0;
  clockPin = clockPin_val[0];
  for (int i = 1; i < Size; i++)
  {
    if (clockPin_val[i] != clockPin)
    {
      flag = 1;
      break;
    }
  }
  if (flag == 0)
    SyncMode = 1;
  else
    SyncMode = 0;
}

void HX711List::begin()
{
  for (int i = 0; i < Size; i++)
    hx711[i].begin();
}

bool HX711List::isReady()
{
  for (int i = 0; i < Size; i++)
    if (hx711[i].isReady() == false)
      return false;
  return true;
}

void HX711List::powerDown()
{
  for (int i = 0; i < Size; i++)
    hx711[i].powerDown();
}

void HX711List::powerUp()
{
  for (int i = 0; i < Size; i++)
    hx711[i].powerUp();
}

void HX711List::setGain(hx711_gain_t gain_val)
{
  gain = gain_val;
  readData(gain);
}

int HX711List::readData(uint8_t hx711_num)
{
  return hx711[hx711_num].readData(gain);
}

int HX711List::readData()
{
  const uint8_t try_again = 3;
  int data = 0;

  if (SyncMode == 1)
  {
    int try_count = 0;
    while (try_count < try_again)
    {
      try_count++;
      data = readDataSync(gain);
      if (errRead == 0)
        break;
    }
  }
  else
    data = readDataAsync(gain);

  return data;
}

#if !defined(DEBUG_MODE)
int HX711List::readDataAsync(hx711_gain_t gain_val)
{
  const uint8_t total_read_allow = Size * 2;
  errRead = 0;

  if (_powerDown == 1)
    powerUp();

  int data = 0;
  int data_temp = 0;
  int try_count = 0;
  for (int i = 0; i < Size; i++)
  {
    while (try_count < total_read_allow)
    {
      try_count++;
      data_temp = hx711[i].readData(gain_val);

      if (data_temp != HX711_FAIL && data_temp < DataUnitMax)
        break;
    }

    if (data_temp == HX711_FAIL)
      errRead++;
    else
      data += data_temp;
  }

  if (_powerDown == 1)
    powerDown();

  // data = data / (Size - errRead) * Size;
  return data;
}

int HX711List::readDataSync(hx711_gain_t gain_val)
{
  const uint8_t max_wait = 105; // max wait time (105ms)
  unsigned long timer = millis();
  errRead = 0;
  while (isReady() == false && millis() - timer < max_wait)
    delay(1);

  // time wait after ready signal: min 0.1us
  delayMicroseconds(1);
  if (isReady() == false)
    return HX711_FAIL;

  int data[Size] = {0};
  int data_sum = 0;
  for (int i = 0; i < 24; i++)
  { // response time: max 0.1us
    // clock pin high time: min 0.2us, typ 1us, max 50us
    // clock pin low time: min 0.2us, typ 1us
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(1);
    for (int j = 0; j < Size; j++)
    {
      data[j] = (data[j] << 1) | digitalRead(hx711[j].dataPin);
    }
    digitalWrite(clockPin, LOW);
    delayMicroseconds(1);
  }

  for (int i = 0; i < gain_val - 24; i++)
  {
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(1);
    digitalWrite(clockPin, LOW);
    delayMicroseconds(1);
  }
  // data pin will be HIGH after read process
  for (int i = 0; i < Size; i++)
    if (digitalRead(hx711[i].dataPin) == LOW)
      data[i] = HX711_FAIL;

  // Overflow and Underflow
  for (int i = 0; i < Size; i++)
  {
    if (data[i] == 0x7FFFFF || data[i] == 0x800000 || data[i] == HX711_FAIL || data[i] > DataUnitMax)
      errRead++;
    else
    {
      if ((data[i] & BIT23) > 0)
        data[i] |= 0xFF000000;
      data_sum += data[i];
    }
  }

  // data_sum = data_sum / (Size - errRead) * Size;
  return data_sum;
}
#else
int HX711List::readDataAsync(hx711_gain_t gain_val)
{
  Serial.println();
  const uint8_t total_read_allow = Size * 2;
  errRead = 0;

  if (_powerDown == 1)
    powerUp();

  int data = 0;
  int data_temp = 0;
  int try_count = 0;
  for (int i = 0; i < Size; i++)
  {
    while (try_count < total_read_allow)
    {
      try_count++;
      data_temp = hx711[i].readData(gain_val);

      if (data_temp != HX711_FAIL && data_temp < DataUnitMax)
        break;
    }

    if (data_temp == HX711_FAIL)
    {
      errRead++;
      Serial.print("`FAIL`" + String(i + 1) + " | ");
    }
    else
    {
      data += data_temp;
      Serial.print("`READ`" + String(i + 1) + " = " + String(data_temp / 420) + " | ");
    }
  }

  if (_powerDown == 1)
    powerDown();

  Serial.print("`DATA`=" + String(data / 420) + "; \r\n");

  return data;
}

int HX711List::readDataSync(hx711_gain_t gain_val)
{
  Serial.println();
  const uint8_t max_wait = 105; // max wait time (105ms)
  unsigned long timer = millis();
  errRead = 0;
  while (isReady() == false && millis() - timer < max_wait)
    delay(1);

  // time wait after ready signal: min 0.1us
  delayMicroseconds(1);
  if (isReady() == false)
    return HX711_FAIL;

  int data[Size] = {0};
  int data_sum = 0;
  for (int i = 0; i < 24; i++)
  { // response time: max 0.1us
    // clock pin high time: min 0.2us, typ 1us, max 50us
    // clock pin low time: min 0.2us, typ 1us
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(1);
    for (int j = 0; j < Size; j++)
    {
      data[j] = (data[j] << 1) | digitalRead(hx711[j].dataPin);
    }
    digitalWrite(clockPin, LOW);
    delayMicroseconds(1);
  }

  for (int i = 0; i < gain_val - 24; i++)
  {
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(1);
    digitalWrite(clockPin, LOW);
    delayMicroseconds(1);
  }
  // data pin will be HIGH after read process
  for (int i = 0; i < Size; i++)
    if (digitalRead(hx711[i].dataPin) == LOW)
      data[i] = HX711_FAIL;

  // Overflow and Underflow
  for (int i = 0; i < Size; i++)
  {
    if (data[i] == 0x7FFFFF || data[i] == 0x800000 || data[i] == HX711_FAIL || data[i] > DataUnitMax)
    {
      errRead++;
      Serial.print("`FAIL`" + String(i + 1) + " | ");
    }
    else
    {
      if ((data[i] & BIT23) > 0)
        data[i] |= 0xFF000000;
      data_sum += data[i];
      Serial.print("`READ`" + String(i + 1) + " = " + String(data[i] / 420) + " | ");
    }
  }

  Serial.print("`DATA`= " + String(data_sum / 420) + "; \r\n");

  // data_sum = data_sum / (Size - errRead) * Size;
  return data_sum;
}
#endif