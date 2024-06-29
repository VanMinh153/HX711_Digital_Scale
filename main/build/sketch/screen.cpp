#line 1 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\screen.cpp"
#include "screen.h"

// #if defined(HW_LCD)
// extern LiquidCrystal_I2C lcd;
// #endif

// #if defined(OLED)
// extern Adafruit_SSD1306 oled;
// #endif

// class LCD
HW_LCD::HW_LCD(uint8_t lcd_Addr, uint8_t lcd_cols, uint8_t lcd_rows)
    : LiquidCrystal_I2C(lcd_Addr, lcd_cols, lcd_rows) {}

void HW_LCD::printWeight(float w)
{
  if (w < 0 && w > -ABSOLUTE_ERROR)
    return;
  setCursor(1, 1);
  if (Mode == LB_MODE)
  {
    print(w * KG_TO_LB);
    print(" lb    ");
  }
  else
  {
    print(w);
    print(" kg    ");
  }
}

//----------------------------------------------------------------------------------------------------------------------
OLED::OLED(uint8_t w, uint8_t h)
    : Adafruit_SSD1306(128, 64, &Wire, -1) {}

void OLED::printWeight(float w)
{
  if (w < 0 && w > -ABSOLUTE_ERROR)
    return;
  setTextSize(2);
  setCursor(28, 24);
  if (Mode == LB_MODE)
  {
    print(w * KG_TO_LB);
    setTextSize(1);
    print(" lb    ");
  }
  else
  {
    print(w);
    setTextSize(1);
    print(" kg    ");
  }
  display();
}
//
void oled_M(float w)
{
  if (w < 0 && w > -ABSOLUTE_ERROR)
    return;
  clearDisplay();
  setCursor(24, 0);
  println(MAIN_TITLE);
  setTextSize(2);
  setCursor(28, 24);
  if (Mode == LB_MODE)
  {
    print(w * KG_TO_LB);
    setTextSize(1);
    print(" lb    ");
  }
  else
  {
    print(w);
    setTextSize(1);
    print(" kg    ");
  }
  display();
}