#include "HX711-HEDSPI.h"
#include <LiquidCrystal_I2C.h>

#define DOUT 6
#define PD_SCLK 7
#define SDA 2
#define SCL 3

#define TAREE 4
#define MODE 5
#define DOWN 8
#define UP 9
#define RECORD 10

#define SCALE 420
// #define SCALE 1030
#define MAX_LOAD 200
#define ERR_RANGE 0.1
#define TIMEOUT 5000
#define FLICKER_DELAY 500
#define KG_MODE 0
#define LB_MODE 1
#define KG_TO_LB 2.204623

HX711 sensor(DOUT, PD_SCLK);
LiquidCrystal_I2C lcd(0x27, 16, 2);

int scale = SCALE;
float w = 0;
float w_data[10];
int p = 0;
float w_avg = 0;
unsigned long timer = millis();
unsigned long refTime2 = millis();
uint8_t mode = KG_MODE;
float record[3];
int q = 0;
volatile char k = 0;

#ifdef ESP32
  void IRAM_ATTR recordInterrupt() {
    k++;
    if (k == 255)
      k = 0;
    lcd.setCursor(1, 0);
    lcd.print("Record Weight");

    lcd.print(record[(q + k)%3]);
    lcd.print(" kg    ");
    lcd.setCursor(0, 0);
    lcd.print(" Digital Scale");
    delay(2000);
  }
#elif defined(ARDUINO_AVR_UNO)
  void recordInterrupt() {
    k++;
    if (k == 4) {
      k = 0;
      lcd.clear();
      return;
    }
    lcd.setBacklight(0);
    lcd.setCursor(1, 0);
    lcd.print("Record Weight");
    lcd.setCursor(1, 1);
    lcd.print(record[(q - k + 4)%3]);
    lcd.print(" kg    ");
    delay(2000);
  }
#endif

void setup() {
  Serial.begin(57600);
  pinMode(TARE, INPUT_PULLUP);
  pinMode(MODE, INPUT_PULLUP);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(RECORD, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(MODE), recordInterrupt, FALLING);
  #ifdef ESP32
    Wire.begin(SDA, SCL);
  #endif

  sensor.begin();
  sensor.setGain(GAIN_128);
  sensor.setScale(SCALE);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(" Digital Scale");
  lcd.setCursor(0, 1);
  lcd.print(" SOICT - HUST");
  delay(1000);
  lcd.setCursor(1, 1);
  lcd.print("            ");
}


void loop() {
  lcd.setCursor(0, 0);
  lcd.print(" Digital Scale");
  
  do {
    w = sensor.getWeight(3);
  } while (abs(w) > MAX_LOAD);

  w_avg += (w - w_data[p])/10;
  p++;
  if (p == 10)
    p = 0;
  w_data[p] = w;

// feature: display weighting results to 2 decimal places
// allow display negative results
  lcd_(w);

// feature: Auto turn off the screen backlight
// if the weighing result does not change by more than (ERR_RANGE)kg in 3s
  if (abs(w - w_avg) < ERR_RANGE) {
    if (millis() - timer > TIMEOUT) {
      lcd.noBacklight();
      delay(FLICKER_DELAY);
      lcd.backlight();
      delay(FLICKER_DELAY);
      lcd.noBacklight();
      delay(FLICKER_DELAY);
      lcd.backlight();
      delay(1200);
      lcd.noBacklight();
      lcd.clear();
      while (abs(w - sensor.getWeight()) < ERR_RANGE) {
        delay(500);
      }
      lcd.backlight();
      timer = millis();
    }
  } else {
    timer = millis();
  }
  
// feature: Save and View the results of the last 3 weightings
  if (w_avg > 1 && abs(w - w_avg) < ERR_RANGE && abs(w_avg - record[q]) > ERR_RANGE) {
    int flag = 0;
    for (int i = 0; i < 10; i++) {
      if (abs(w_data[i] - w) > ERR_RANGE) {
        flag = 1;
        break;
      }
      if (flag == 0) {
        q++;
        if (q == 3)
          q = 0;
        record[q] = w;
      }
    }
  }

// feature: Adjust the scale to 0 kg
  if (digitalRead(TARE) == LOW) {
    sensor.setToZerro();
    lcd.setCursor(0, 1);
    lcd.print(" Taring...       ");
    delay(100);
    timer = millis();
  }

// feature: Change weight unit from kilogram to pound
  if (digitalRead(MODE) == LOW) {
    if (mode == KG_MODE) {
      mode = LB_MODE;
    } else {
      mode = KG_MODE;
    }
    lcd_(w);
    delay(100);
    timer = millis();
  }

// feature: Adjust weighting result up
// by decreasing weight coefficient (scale)
  while (digitalRead(UP) == LOW) {
    scale -= 1;
    sensor.setScale(scale);
    lcd_(sensor.getWeight(3, 2));
    timer = millis();
  }

// feature: Adjust weighting result down
// by increasing weight coefficient (scale)
  while (digitalRead(DOWN) == LOW) {
    scale += 1;
    sensor.setScale(scale);
    lcd_(sensor.getWeight(3, 2));
    timer = millis();
  }

  delay(50);
}

void lcd_(float w) {
  lcd.setCursor(1, 1);
  if (mode == LB_MODE) {
    lcd.print(w*KG_TO_LB);
    lcd.print(" lb    ");
  } else {
    lcd.print(w);
    lcd.print(" kg    ");
  }
}
