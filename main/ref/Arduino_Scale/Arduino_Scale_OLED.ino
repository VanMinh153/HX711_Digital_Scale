#include "HX711-SOLDERED.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define I2C_ADDRESS 0x3C

#define DOUT 6
#define PD_SCLK 7

#define TAREE 4
#define UP 9
#define DOWN 8

#define SCALE 420
#define MAX_LOAD 150
#define AVG 5

HX711 sensor(DOUT, PD_SCLK);
Adafruit_SSD1306 oled(128, 64, &Wire, -1);

void setup()
{
  Serial.begin(57600);
  pinMode(TARE, INPUT_PULLUP);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);

  sensor.begin();
  sensor.setToZerro();
  sensor.setGain(GAIN_128);
  sensor.setScale(SCALE);

  oled.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS);
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(24, 0);
  oled.println("Digital Scale");

  oled.setCursor(28, 20);
  oled.println("SOICT - HUST");
  oled.display();
}

int  scale = SCALE;
float w = 0;

void loop()
{
  w = sensor.getAverageWeight(10, 20);

  oled_(w);

  if (digitalRead(TARE) == LOW)
  {
    sensor.setToZerro();
    oled_("Taring...");
  }

  while(digitalRead(UP) == LOW)
  {
    scale -= 2;
    sensor.setScale(scale);
    oled_(sensor.getWeight());
    delay(50);
  }
  
  while(digitalRead(DOWN) == LOW)
  {
    scale += 2;
    sensor.setScale(scale);
    oled_(sensor.getWeight());
    delay(50);
  }
  delay(50);
}

void oled_(float w) {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(24, 0);
  oled.println("Digital Scale");

  oled.setCursor(28, 24);
  oled.setTextSize(2);
  oled.print(w);
  oled.setTextSize(1);
  oled.println(" kg");
  oled.display();
}


void oled_(const char *msg) {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(24, 0);
  oled.println("Digital Scale");

  oled.setCursor(15, 20);
  oled.setTextSize(2);
  oled.print(msg);
  oled.display();
}