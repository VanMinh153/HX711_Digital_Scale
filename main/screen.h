#ifndef screen_h
#define screen_h

#include "config.h"
#include <stdint.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_SSD1306.h>

extern uint8_t Mode;
extern float readTemperature();

class Screen
{
public:
  virtual void begin() = 0;
  virtual void clear() = 0;
  virtual void noBacklight() = 0;
  virtual void backlight() = 0;
  virtual void printWeight(float w) = 0;
  virtual void printTemperature(float t) = 0;
  virtual void printTitle(String title) = 0;
  virtual void printContent(String content) = 0;
};

class LCD_I2C : public LiquidCrystal_I2C, public Screen
{
public:
  LCD_I2C(uint8_t lcd_Addr, uint8_t lcd_cols, uint8_t lcd_rows);
  void begin() override;
  void clear() override;
  void noBacklight() override;
  void backlight() override;

  void printWeight(float w) override;
  void printTemperature(float t) override;
  void printTitle(String title) override;
  void printContent(String content) override;
};

class OLED_SSD1306 : Adafruit_SSD1306, public Screen
{
public:
  OLED_SSD1306(uint8_t width, uint8_t height);
  void begin() override;
  void clear() override;
  void noBacklight() override;
  void backlight() override;

  void printWeight(float w) override;
  void printTemperature(float t) override;
  void printTitle(String title) override;
  void printContent(String content) override;
};

class TEST_Screen : public Screen
{
public:
  TEST_Screen();
  LCD_I2C lcd;
  OLED_SSD1306 oled;
  void begin() override;
  void clear() override;
  void noBacklight() override;
  void backlight() override;

  void printWeight(float w) override;
  void printTemperature(float t) override;
  void printTitle(String title) override;
  void printContent(String content) override;
};


#endif