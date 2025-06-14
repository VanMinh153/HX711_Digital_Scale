#include "screen.h"

// LCD_I2C: LCD display implementation
LCD_I2C::LCD_I2C(uint8_t lcd_Addr, uint8_t lcd_cols, uint8_t lcd_rows)
    : LiquidCrystal_I2C(lcd_Addr, lcd_cols, lcd_rows) {}

void LCD_I2C::begin()
{
  // Initialize LCD
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

// Print weight to LCD, with unit conversion
void LCD_I2C::printWeight(float w)
{
  if (w < 0 && w > -ABSOLUTE_ERROR)
    w = 0;
  setCursor(1, 1);
  print("               "); // Clear line
  setCursor(1, 1);
  if (Mode == MODE_US)
  {
    print(w * KG_TO_LB);
    print(" lb");
  }
  else
  {
    print(w);
    print(" kg");
  }
#if defined(HW_TEMPERATURE)
  printTemperature(readTemperature());
#endif
}

// Print temperature to LCD
void LCD_I2C::printTemperature(float t)
{
  setCursor(12, 1);
  if (Mode == MODE_US)
  {
    print(round(t * 9 / 5 + 32), 0);
    print(String((char) 223) + "F");
  }
  else
  {
    print(round(t), 0);
    print(String((char) 223) + "C");
  }
}

// Print title to LCD (clears display)
void LCD_I2C::printTitle(String title)
{
  LiquidCrystal_I2C::clear();
  setCursor(1, 0);
  print(title);
}

// Print content to LCD
void LCD_I2C::printContent(String content)
{
  setCursor(1, 1);
  print(content);
}

// Print record to LCD
void LCD_I2C::printRecord(String name, String weight)
{
  clear();
  setCursor(0, 0); // Dòng trên
  print(name);
  setCursor(0, 1); // Dòng dưới
  print(weight);
}

//----------------------------------------------------------------------------------------------------------------------
// OLED_SSD1306: OLED display implementation
OLED_SSD1306::OLED_SSD1306(uint8_t w, uint8_t h)
    : Adafruit_SSD1306(128, 64, &Wire, -1) {}

void OLED_SSD1306::begin()
{
  // Initialize OLED
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
  dim(true); // Dim display
}

void OLED_SSD1306::backlight()
{
  dim(false); // Brighten display
}

// Print weight to OLED, with unit conversion
void OLED_SSD1306::printWeight(float w)
{
  if (w < 0 && w > -ABSOLUTE_ERROR)
    w = 0;
  setTextSize(2);
  setCursor(24, 24);
  if (Mode == MODE_US)
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
#if defined(HW_TEMPERATURE)
  printTemperature(readTemperature());
#endif
  display();
}

// Print temperature to OLED
void OLED_SSD1306::printTemperature(float t)
{
  setTextSize(1);
  setCursor(66, 44);
  if (Mode == MODE_US)
  {
    print(round(t * 9 / 5 + 32), 0);
    setTextSize(1);
    print(String((char) 247) + "F");
  }
  else
  {
    print(round(t), 0);
    setTextSize(1);
    print(String((char) 247) + "C");
  }
  display();
}

// Print title to OLED (clears display)
void OLED_SSD1306::printTitle(String title)
{
  clearDisplay();
  setTextSize(1);
  setCursor(24, 4);
  println(title);
  display();
}

// Print content to OLED
void OLED_SSD1306::printContent(String content)
{
  setCursor(12, 24);
  setTextSize(2);
  println(content);
  display();
}

// Print record to OLED
void OLED_SSD1306::printRecord(String name, String weight)
{
  clearDisplay();
  setTextSize(1);
  setCursor(0, 0); // Dòng trên
  println(name);
  setTextSize(2);
  setCursor(0, 24); // Dòng dưới
  println(weight);
  display();
}

//----------------------------------------------------------------------------------------------------------------------
// TEST_Screen: Combines LCD and OLED for testing
TEST_Screen::TEST_Screen()
:lcd(LCD_I2C_ADDRESS, 16, 2), oled(128, 64) {}

void TEST_Screen::begin()
{
  lcd.begin();
  oled.begin();
}

void TEST_Screen::clear()
{
  lcd.clear();
  oled.clear();
}

void TEST_Screen::noBacklight()
{
  lcd.noBacklight();
  oled.noBacklight();
}

void TEST_Screen::backlight()
{
  lcd.backlight();
  oled.backlight();
}

void TEST_Screen::printWeight(float w)
{
  lcd.printWeight(w);
  oled.printWeight(w);
}

void TEST_Screen::printTemperature(float t)
{
  lcd.printTemperature(t);
  oled.printTemperature(t);
}

void TEST_Screen::printTitle(String title)
{
  lcd.printTitle(title);
  oled.printTitle(title);
}

void TEST_Screen::printContent(String content)
{
  lcd.printContent(content);
  oled.printContent(content);
}

// Print record to test screen
void TEST_Screen::printRecord(String name, String weight)
{
  lcd.printRecord(name, weight);
  oled.printRecord(name, weight);
}
