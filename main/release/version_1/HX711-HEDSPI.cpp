/**
 * @brief       Library for HX711
 *
 * @author     Nguyen Van Minh - SOICT-HUST
 * @ref        HX711-SOLDERED - @soldered.com @Robert Peric
 */

#include "HX711-HEDSPI.h"

HX711::HX711(uint8_t dout, uint8_t pd_sck, uint8_t gain, float scale)
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

void HX711::setGain(uint8_t gain)
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

int32_t HX711::getData()
{
  const byte response_time = 1;
  digitalWrite(PD_SCK, LOW);
  delayMicroseconds(response_time);

  unsigned long timer = millis();
  while (digitalRead(DOUT) == HIGH && millis() - timer < 105)
    delay(10);

  int32_t data = 0;
  for (uint8_t i = 0; i < 24; i++)
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
    return 0x7FFFFFFF;

  byte i = Gain - 25;
  while (i > 0)
  {
    i--;
    digitalWrite(PD_SCK, LOW);
    delayMicroseconds(response_time);
    digitalWrite(PD_SCK, HIGH);
    delayMicroseconds(response_time);
  }

  if (bitRead(data, 23) == 1)
    data |= 0xFF000000;

  return data;
}