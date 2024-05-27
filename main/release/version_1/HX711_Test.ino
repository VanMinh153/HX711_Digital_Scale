/*
Test result on Arduino UNO: (Avarage time for 40 loops)
getData_H(GAIN_128) ~ 99
getData_H(GAIN_64) ~ 49
getData_L(GAIN_128) ~ 99
getData_L(GAIN_64) ~ 99
*/

#include <Arduino.h>

#define GAIN_128 25
#define GAIN_64 27
#define GAIN_32 26

#define DOUT 6
#define PD_SCK 7
#define SCALE 420

// Test for weight = 39kg
#define D128 16380
#define D64 8190

int32_t getData_H(byte Gain);
int32_t getData_L(byte Gain);
void test_H(byte Gain, byte Loop);
void test_L(byte Gain, byte Loop);


void setup()
{
  Serial.begin(57600);
  pinMode(PD_SCK, OUTPUT);
  pinMode(DOUT, INPUT);
  delay(1000);
}


int32_t data;
unsigned long wait = 0;
unsigned long wait_avg = 0;
void loop()
{
  test_H(GAIN_128, 40);
  test_H(GAIN_64, 40);

  getData_L(GAIN_128); // must to getData_L(GAIN_128) twice before get correct data
  test_L(GAIN_128, 40);
  test_L(GAIN_64, 40);
  delay(100000);
}

//-----------------------------------------------------------------------------------------------
void test_H(byte Gain, byte Loop)
{
  data = getData_H(Gain); // first getData_H() is to set up Gain for HX711
  wait_avg = 0;
  for (byte i = 0; i < Loop; i++)
  {
    data = getData_H(Gain);
    wait_avg += wait;
    if ((data == D128 && Gain == GAIN_128) || (data == D64 && Gain == GAIN_64))
      Serial.print('_');
    else if (data == D128 || data == D64)
      Serial.print('*');
    else
      Serial.print('!');
  }
  Serial.println(String(" > " + String(wait_avg / Loop)));
}
//
void test_L(byte Gain, byte Loop)
{
  data = getData_L(Gain); // first getData_L() is to set up Gain for HX711
  wait_avg = 0;
  for (byte i = 0; i < Loop; i++)
  {
    data = getData_L(Gain);
    wait_avg += wait;
    if ((data == D128 && Gain == GAIN_128) || (data == D64 && Gain == GAIN_64))
      Serial.print('_');
    else if (data == D128 || data == D64)
      Serial.print('*');
    else
      Serial.print('!');
  }
  Serial.println(String(" > " + String(wait_avg / Loop)));
}
//
/**
 * @brief     Get data from HX711 and set up PD_SCK = HIGH after
*/
int32_t getData_H(byte Gain)
{
  const byte response_time = 1;
  digitalWrite(PD_SCK, LOW);

  unsigned long timer = millis();
  while (digitalRead(DOUT) == HIGH && millis() - timer < 102)
    delayMicroseconds(100);
  wait = millis() - timer;

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
/**
 * @brief     Get data from HX711 and set up PD_SCK = LOW after
*/
int32_t getData_L(byte Gain)
{
  const byte response_time = 1;
  unsigned long timer = millis();
  while (digitalRead(DOUT) == HIGH && millis() - timer < 102)
    delayMicroseconds(100);
  wait = millis() - timer;

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
  digitalWrite(PD_SCK, LOW);
  delayMicroseconds(response_time);

  byte i = Gain - 25;
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