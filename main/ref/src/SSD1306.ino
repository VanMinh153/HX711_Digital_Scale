
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define I2C_ADDRESS 0x3C
Adafruit_SSD1306 oled(128, 64, &Wire, OLED_RESET);

void setup() {
  oled.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS);
  
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.println("Hello, world!");
  oled.display();
}

void loop() {
}