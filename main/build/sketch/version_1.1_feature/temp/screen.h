#line 1 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\version_1.1_feature\\temp\\screen.h"
#ifndef screen_h
#define screen_h

// #include <sys/_stdint.h>
#include <stdint.h>
#include "config.h"
#include <LiquidCrystal_I2C.h>
#include <Adafruit_SSD1306.h>

extern uint8_t Mode;

class Screen
{
public:
  virtual void begin() = 0;
  virtual void clear() = 0;
  virtual void noBacklight() = 0;
  virtual void backlight() = 0;
  virtual void printWeight(float w) = 0;
  virtual void printTitle(char *title) = 0;
  virtual void printContent(char *content) = 0;
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
  void printTitle(char *title) override;
  void printContent(char *content) override;
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
  void printTitle(char *title) override;
  void printContent(char *content) override;
  // void printContent(char *content, uint8_t textSize);
};


#endif