#ifndef screen_h
#define screen_h

#include "config.h"
#include <stdint.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_SSD1306.h>

extern uint8_t Mode;
extern float readTemperature();

// Abstract base class for all screen types
class Screen
{
public:
  virtual void begin() = 0;           // Initialize screen
  virtual void clear() = 0;           // Clear display
  virtual void noBacklight() = 0;     // Turn off backlight/dim
  virtual void backlight() = 0;       // Turn on backlight/brighten
  virtual void printWeight(float w) = 0;      // Show weight
  virtual void printTemperature(float t) = 0; // Show temperature
  virtual void printTitle(String title) = 0;  // Show title
  virtual void printContent(String content) = 0; // Show content
};

// LCD implementation
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

// OLED implementation
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

// Test screen: combines LCD and OLED
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