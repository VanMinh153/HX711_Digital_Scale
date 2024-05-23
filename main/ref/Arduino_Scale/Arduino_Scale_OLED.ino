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
Adafruit_SSD1306 display(128, 64, &Wire, -1);

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

  display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(24, 0);
  display.println("Digital Scale");

  display.setCursor(28, 20);
  display.println("SOICT - HUST");
  display.display();
}

int  scale = SCALE;
float w = 0;

void loop()
{
  w = sensor.getAverageWeight(10, 20);

  display_(w);

  if (digitalRead(TARE) == LOW)
  {
    sensor.setToZerro();
    display_("Taring...");
  }

  while(digitalRead(UP) == LOW)
  {
    scale -= 2;
    sensor.setScale(scale);
    display_(sensor.getWeight());
    delay(50);
  }
  
  while(digitalRead(DOWN) == LOW)
  {
    scale += 2;
    sensor.setScale(scale);
    display_(sensor.getWeight());
    delay(50);
  }
  delay(50);
}

void display_(float w) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(24, 0);
  display.println("Digital Scale");

  display.setCursor(28, 24);
  display.setTextSize(2);
  display.print(w);
  display.setTextSize(1);
  display.println(" kg");
  display.display();
}


void display_(const char *msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(24, 0);
  display.println("Digital Scale");

  display.setCursor(15, 20);
  display.setTextSize(2);
  display.print(msg);
  display.display();
}