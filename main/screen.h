#ifndef screen_h
#define screen_h

// #include <sys/_stdint.h>
#include <stdint.h>
#include "config.h"
#include <LiquidCrystal_I2C.h>
#include <Adafruit_SSD1306.h>

extern uint8_t Mode;


// #if defined(HW_LCD)
// LCD_I2C screen(LCD_I2C_ADDRESS, 16, 2);
// #endif

// #if defined(HW_OLED)
// OLED_SSD1306 screen(128, 64);
// #endif

class Screen
{
public:
  virtual void printWeight(float w) = 0;
  virtual void printTitle(char *title = MAIN_TITLE) = 0;
  virtual void printContent(char *content) = 0;
};

class LCD_I2C : public LiquidCrystal_I2C, public Screen
{
public:
  LCD_I2C(uint8_t lcd_Addr, uint8_t lcd_cols, uint8_t lcd_rows);
  void printWeight(float w) override;
  void printTitle(char *title) override;
  void printContent(char *content) override;
};

class OLED_SSD1306 : Adafruit_SSD1306, public Screen
{
public:
  OLED_SSD1306(uint8_t width, uint8_t height);
  void printWeight(float w) override;
  void printTitle(char *title) override;
  void printContent(char *content) override;
};


#endif