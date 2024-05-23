#include "HX711-HEDSPI.h"
#include <LiquidCrystal_I2C.h>

#define DOUT 6
#define PD_SCK 7
#define TAREE 4
#define MODE 5
#define DOWN 8
#define UP 9
#define RECORD 3
#define DEBOUNT_TIME 200

#define SCALE 420
// #define SCALE 1030
#define MAX_LOAD 200
#define ERR_RANGE 0.2
#define TIMEOUT 10000
#define FLICKER_DELAY 500
#define KG_MODE 0
#define LB_MODE 1
#define KG_TO_LB 2.204623

HX711 sensor(DOUT, PD_SCK, GAIN_128, SCALE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

int scale = SCALE;
uint8_t mode = KG_MODE;
float w = 0;
float w_data[5];
int p = 0;
float w_avg = 0;
unsigned long timer = millis();

float record[3];
int q = 0;
volatile char k = 0;
volatile bool status = false;
unsigned long buttonTime = millis();

void recordInterrupt()
{
  if (millis() - buttonTime < DEBOUNT_TIME)
  {
    return;
  }
  buttonTime = millis();
  status = true;
}

void setup()
{
  Serial.begin(57600);
  pinMode(TARE, INPUT_PULLUP);
  pinMode(MODE, INPUT_PULLUP);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(RECORD, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RECORD), recordInterrupt, RISING);

  sensor.init();
  
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

uint32_t Zero = 0;
float Scale = 420;
float getWeight(uint8_t repeat) {
  uint32_t w = 0;
  for (uint8_t i = 0; i < repeat; i++) {
    w += sensor.getData();
  }

  w /= repeat;
  return (long)(w - Zero) / Scale;
}

float getWeight_(uint8_t repeat) {
  uint32_t w = 0;
  uint32_t w_avg = 0;
  uint8_t n = 0;
  for (uint8_t i = 0; i < repeat; i++) {
    w += sensor.getData();
    if (abs(w) < MAX_LOAD) {
      w_avg += w;
      n++;
    }
  }

  w_avg /= n;
  return (long)(w_avg - Zero) / Scale;
}
void insertionSort(int arr[], int n) {
  for (int i = 1; i < n; i++) {
    int key = arr[i];
    int j = i - 1;
    while (j >= 0 && arr[j] > key) {
      arr[j + 1] = arr[j];
      j = j - 1;
    }
    arr[j + 1] = key;
  }
}
float getWeight_2(uint8_t repeat) {
  uint32_t w[repeat];
  uint32_t w_avg = 0;
  uint8_t n = 0;
  for (uint8_t i = 0; i < repeat; i++) {
    w[i] += sensor.getData();
  }
  sqrt(3);
  constrain(3, 2, 5);
  w_avg /= n;
  return (long)(w_avg - Zero) / Scale;
}

void loop()
{
  lcd.setCursor(0, 0);
  lcd.print(" Digital Scale");

  delay(1000);
  Serial.println("\r\ngetWeight()*3");
  for (int i = 0; i < 20; i++)
  {
    sensor.getWeight();
    sensor.getWeight();
    sensor.getWeight();
  }
  delay(1000);
  Serial.println("\r\ngetWeight(3) Typical");
  for (int i = 0; i < 20; i++)
  {
    uint32_t w = 0;
    for (uint8_t i = 0; i < 3; i++) {
      w += sensor.getData();
    }
    w /= 3;
    w = (long)(w - Zero) / Scale;
  }
  delay(1000);
  Serial.println("\r\ngetWeight_(3) function");
  for (int i = 0; i < 20; i++)
  {
    getWeight_(3);
  }
  Serial.println("\r\nEND!!!");


  do
  {
    w = sensor.getWeight(3);
  } while (abs(w) > MAX_LOAD);

  w_avg += (w - w_data[p]) / 5;
  w_data[p] = w;
  p++;
  if (p == 5)
    p = 0;
  Serial.println(w);

  // display weighting results
  lcd_(w);

  // feature: Auto turn off the screen backlight
  // if the weighing result does not change by more than (ERR_RANGE)kg in 3s
  if (abs(w - w_avg) < ERR_RANGE)
  {
    if (millis() - timer > TIMEOUT)
    {
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
      while (abs(w - sensor.getWeight()) < ERR_RANGE)
      {
        delay(500);
      }
      lcd.backlight();
      timer = millis();
    }
  }
  else
  {
    timer = millis();
  }

  // feature: Save and View the results of the last 3 weightings
  if (w_avg > 1 && abs(w - w_avg) < ERR_RANGE && abs(w_avg - record[q]) > ERR_RANGE)
  {
    int flag = 0;
    for (int i = 0; i < 5; i++)
    {
      if (abs(w_data[i] - w) > ERR_RANGE)
      {
        flag = 1;
        break;
      }
      if (flag == 0)
      {
        q++;
        if (q == 3)
          q = 0;
        record[q] = w;
      }
    }
  }

  // feature: Adjust the scale to 0 kg
  if (digitalRead(TARE) == LOW)
  {
    sensor.setZero();
    lcd.setCursor(0, 1);
    lcd.print(" Taring...       ");
    delay(100);
    timer = millis();
  }

  // feature: Change weight unit from kilogram to pound
  if (digitalRead(MODE) == LOW)
  {
    if (mode == KG_MODE)
    {
      mode = LB_MODE;
    }
    else
    {
      mode = KG_MODE;
    }
    lcd_(w);
    delay(100);
    timer = millis();
  }

  // feature: Adjust weighting result up
  // by decreasing weight coefficient (scale)
  while (digitalRead(UP) == LOW)
  {
    scale -= 1;
    sensor.setScale(scale);
    lcd_(sensor.getWeight(3));
    timer = millis();
  }

  // feature: Adjust weighting result down
  // by increasing weight coefficient (scale)
  while (digitalRead(DOWN) == LOW)
  {
    scale += 1;
    sensor.setScale(scale);
    lcd_(sensor.getWeight(3));
    timer = millis();
  }

  delay(20);
}

// display weighting results
void lcd_(float w)
{
  lcd.setCursor(1, 1);
  if (mode == LB_MODE)
  {
    lcd.print(w * KG_TO_LB);
    lcd.print(" lb    ");
  }
  else
  {
    lcd.print(w);
    lcd.print(" kg    ");
  }
}
