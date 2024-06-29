#line 1 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\screen.h"
#ifndef screen_h
#define screen_h

// #include <sys/_stdint.h>
#include <stdint.h>
#include "config.h"
#include <LiquidCrystal_I2C.h>
#include <Adafruit_SSD1306.h>

extern uint8_t Mode;


#if defined(LCD)
HW_LCD screen(LCD_I2C_ADDRESS, 16, 2);
// LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, 16, 2);
#endif

#if defined(OLED)
OLED screen(128, 64);
// Adafruit_SSD1306 oled(128, 64);
#endif

class Screen
{
public:
  virtual void printWeight(float w) = 0;

};

class HW_LCD : public LiquidCrystal_I2C, public Screen
{
public:
  HW_LCD(uint8_t lcd_Addr, uint8_t lcd_cols, uint8_t lcd_rows);

protected:

};

class OLED : Adafruit_SSD1306, public Screen
{
public:
  OLED(uint8_t w, uint8_t h);

protected:

};


#endif