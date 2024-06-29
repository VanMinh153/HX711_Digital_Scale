#include "screen.h"

// class LCD
LCD_I2C::LCD_I2C(uint8_t lcd_Addr, uint8_t lcd_cols, uint8_t lcd_rows)
    : LiquidCrystal_I2C(lcd_Addr, lcd_cols, lcd_rows) {}

void LCD_I2C::printWeight(float w)
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

void LCD_I2C::printTitle(char *title)
{
  setCursor(1, 0);
  print(title);
}

void LCD_I2C::printContent(char *content)
{
  setCursor(2, 0);
  print(content);
}

//----------------------------------------------------------------------------------------------------------------------
OLED_SSD1306::OLED_SSD1306(uint8_t w, uint8_t h)
    : Adafruit_SSD1306(128, 64, &Wire, -1) {}

void OLED_SSD1306::printWeight(float w)
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

void OLED_SSD1306::printTitle(char *title)
{
  clearDisplay();
  setCursor(24, 0);
  println(title);
  display();
}

void OLED_SSD1306::printContent(char *content)
{
  setCursor(2, 0);
  print(content);
}