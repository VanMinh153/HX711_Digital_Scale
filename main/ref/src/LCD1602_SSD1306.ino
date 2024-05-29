#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LiquidCrystal_I2C.h>

#define SDA 2
#define SCL 3

#define I2C_ADDRESS 0x3C
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Wire.begin(SDA, SCL);

  oled.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS);
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(20, 0);
  oled.println("Hello!");
  oled.display();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("Hello!");
  delay(2000);
}

void loop() {
}