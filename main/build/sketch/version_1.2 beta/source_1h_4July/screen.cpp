#line 1 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\version_1.2 beta\\source_1h_4July\\screen.cpp"
#include "screen.h"

// class LCD
LCD_I2C::LCD_I2C(uint8_t lcd_Addr, uint8_t lcd_cols, uint8_t lcd_rows)
    : LiquidCrystal_I2C(lcd_Addr, lcd_cols, lcd_rows) {}

void LCD_I2C::begin()
{
  init();
  backlight();
  noCursor();
}

void LCD_I2C::clear()
{
  LiquidCrystal_I2C::clear();
}

void LCD_I2C::noBacklight()
{
  LiquidCrystal_I2C::noBacklight();
}

void LCD_I2C::backlight()
{
  LiquidCrystal_I2C::backlight();
}

void LCD_I2C::printWeight(float w)
{
  if (w < 0 && w > -ABSOLUTE_ERROR)
    return;
  setCursor(1, 1);
  print("               ");
  setCursor(1, 1);
  if (Mode == LB_MODE)
  {
    print(w * KG_TO_LB);
    print(" lb");
  }
  else
  {
    print(w);
    print(" kg");
  }
}

void LCD_I2C::printTitle(char *title)
{
  LiquidCrystal_I2C::clear();
  setCursor(1, 0);
  print(title);
}

void LCD_I2C::printContent(char *content)
{
  setCursor(1, 1);
  print(content);
}

//----------------------------------------------------------------------------------------------------------------------
OLED_SSD1306::OLED_SSD1306(uint8_t w, uint8_t h)
    : Adafruit_SSD1306(128, 64, &Wire, -1) {}

void OLED_SSD1306::begin()
{
  Adafruit_SSD1306::begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
  clearDisplay();
  setTextColor(SSD1306_WHITE);
}

void OLED_SSD1306::clear()
{
  clearDisplay();
  display();
}

void OLED_SSD1306::noBacklight()
{
  dim(true);
}

void OLED_SSD1306::backlight()
{
  dim(false);
}

void OLED_SSD1306::printWeight(float w)
{
  if (w < 0 && w > -ABSOLUTE_ERROR)
    return;
  setTextSize(2);
  setCursor(24, 24);
  if (Mode == LB_MODE)
  {
    print(w * KG_TO_LB);
    setTextSize(1);
    print(" lb");
  }
  else
  {
    print(w);
    setTextSize(1);
    print(" kg");
  }
  display();
}

void OLED_SSD1306::printTitle(char *title)
{
  clearDisplay();
  setTextSize(1);
  setCursor(24, 4);
  println(title);
  display();
}

void OLED_SSD1306::printContent(char *content)
{
  setCursor(12, 24);
  setTextSize(2);
  println(content);
  display();
}