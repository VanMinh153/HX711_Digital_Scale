#line 1 "C:\\Users\\Moderator\\Documents\\Documents\\GR1 Scale\\main\\HX711-HEDSPI.cpp"
/**
 * @brief       Library for HX711
 * @author     Nguyen Van Minh - SOICT-HUST
 */

#include "HX711-HEDSPI.h"

// HX711 class
HX711::HX711(byte dout, byte pd_sck, byte gain, float scale)
{
  DOUT = dout;
  PD_SCK = pd_sck;
  Gain = gain;
  Scale = scale;
}

void HX711::init()
{
  pinMode(PD_SCK, OUTPUT);
  pinMode(DOUT, INPUT);
}

void HX711::setGain(byte gain)
{
  if (gain >= 25 && gain <= 27)
    Gain = gain;
}

void HX711::setScale(float scale) { Scale = scale; }

void HX711::setZero(int32_t zero) { Zero = zero; }

int32_t HX711::setZero()
{
  Zero = getData();
  return Zero;
}

float HX711::getWeight()
{
  return (long)(getData() - Zero) / Scale;
}

/**
 * @brief     Get data from HX711 and set up PD_SCK = HIGH after
 */
int32_t HX711::getData_H()
{
  return getData_H(Gain);
}

int32_t HX711::getData_H(byte gain, uint16_t check_freq)
{
  const byte response_time = 1;
  digitalWrite(PD_SCK, LOW);

  unsigned long timer = millis();
  while (digitalRead(DOUT) == HIGH && millis() - timer < 102)
    delayMicroseconds(check_freq);

  int32_t data = 0;
  for (byte i = 0; i < 24; i++)
  {
    digitalWrite(PD_SCK, HIGH);
    delayMicroseconds(response_time);
    data = ((data << 1) | digitalRead(DOUT));
    digitalWrite(PD_SCK, LOW);
    delayMicroseconds(response_time);
  }

  digitalWrite(PD_SCK, HIGH);
  delayMicroseconds(response_time);
  if (digitalRead(DOUT) == LOW)
    return 0x7fffff;

  byte i = gain - 25;
  while (i > 0)
  {
    i--;
    digitalWrite(PD_SCK, LOW);
    delayMicroseconds(response_time);
    digitalWrite(PD_SCK, HIGH);
    delayMicroseconds(response_time);
  }
  delayMicroseconds(65);

  if (bitRead(data, 23) == 1)
    data |= 0xFF000000;

  return data;
}
/**
 * @brief     Get data from HX711 and set up PD_SCK = LOW after
 */
int32_t HX711::getData_L(byte gain, uint16_t check_freq)
{
  const byte response_time = 1;
  unsigned long timer = millis();
  while (digitalRead(DOUT) == HIGH && millis() - timer < 102)
    delayMicroseconds(check_freq);

  int32_t data = 0;
  for (byte i = 0; i < 24; i++)
  {
    digitalWrite(PD_SCK, HIGH);
    delayMicroseconds(response_time);
    data = ((data << 1) | digitalRead(DOUT));
    digitalWrite(PD_SCK, LOW);
    delayMicroseconds(response_time);
  }

  digitalWrite(PD_SCK, HIGH);
  delayMicroseconds(response_time);
  if (digitalRead(DOUT) == LOW)
  {
    digitalWrite(PD_SCK, LOW);
    return 0x7FFFFFFF;
  }
  digitalWrite(PD_SCK, LOW);
  delayMicroseconds(response_time);

  byte i = gain - 25;
  while (i > 0)
  {
    i--;
    digitalWrite(PD_SCK, HIGH);
    delayMicroseconds(response_time);
    digitalWrite(PD_SCK, LOW);
    delayMicroseconds(response_time);
  }

  if (bitRead(data, 23) == 1)
    data |= 0xFF000000;

  return data;
}
//_____________________________________________________________________________________________________________________
// HX711x4_Async class
HX711x4_Async::HX711x4_Async()
{
}

HX711x4_Async::HX711x4_Async(byte dout1, byte pd_sck1, byte dout2, byte pd_sck2, byte dout3, byte pd_sck3, byte dout4, byte pd_sck4, byte gain, float scale)
{
  hx711[0] = HX711(dout1, pd_sck1, gain, scale);
  hx711[1] = HX711(dout2, pd_sck2, gain, scale);
  hx711[2] = HX711(dout3, pd_sck3, gain, scale);
  hx711[3] = HX711(dout4, pd_sck4, gain, scale);
}

HX711x4_Async::init(byte adc_4, byte dout, byte pd_sck, byte gain, float scale)
{
  switch (adc_4)
  {
  case 1:
    hx711[0] = HX711(dout, pd_sck, gain, scale);
    break;
  case 2:
    hx711[1] = HX711(dout, pd_sck, gain, scale);
    break;
  case 3:
    hx711[2] = HX711(dout, pd_sck, gain, scale);
    break;
  case 4:
    hx711[3] = HX711(dout, pd_sck, gain, scale);
    break;
  }
}

void HX711x4_Async::init()
{
  for (byte i = 0; i < 4; i++)
    hx711[i].init();
}

void HX711x4_Async::setGain(byte gain)
{
  if (gain < 25 && gain > 27)
    return;
  for (byte i = 0; i < 4; i++)
    hx711[i].setGain(gain);
}