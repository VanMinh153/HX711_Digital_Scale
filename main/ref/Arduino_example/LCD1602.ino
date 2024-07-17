
#include <LiquidCrystal_I2C.h>

// #define SDA 2
// #define SCL 3

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // Wire.begin(SDA, SCL);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("Hello!");
  delay(2000);
}

void loop() {
}