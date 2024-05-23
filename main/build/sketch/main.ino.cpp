#include <Arduino.h>
#line 1 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
#include "HX711-HEDSPI.h"
#include <LiquidCrystal_I2C.h>

#define DOUT 6
#define PD_SCK 7
#define SDA 2
#define SCL 3
#define TARE 4
#define MODE 5
#define DOWN 8
#define UP 9
#define RECORD 18

#define SCALE 420
// #define SCALE 1030
#define MAX_LOAD 200
#define ABSOLUTE_ERROR 0.2f
#define AUTO_SLEEP_TIME 4800
#define RECORD_TIME 720
#define FLICKER_DELAY 600
#define SLEEP_DELAY 3000
#define SHOW_RECORD_DELAY 3000
#define DEBOUNCE_TIME 200
#define RECORD_NUM 3
#define KG_MODE 0
#define LB_MODE 1
#define KG_TO_LB 2.204623f

HX711 sensor(DOUT, PD_SCK, GAIN_128, SCALE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

float Scale = SCALE;
int32_t Zero = 0;
uint8_t Mode = KG_MODE;
float w = 0;
float w_prev = 0;

int32_t sensor_error = 0;
unsigned long timer = millis();

float record[RECORD_NUM];
uint8_t q = 0;
uint8_t k = 0;
volatile bool _record = false;
volatile bool _tare = false;
volatile bool _mode = false;
volatile bool _up = false;
volatile bool _down = false;
volatile bool _interrupt = false;
unsigned long recordPrev = millis();
unsigned long tarePrev = millis();
unsigned long modePrev = millis();
unsigned long upPrev = millis();
unsigned long downPrev = millis();

// uint8_t i = 0;
// uint8_t flag = 0;
bool _sleep = false;

//-----------------------------------------------------------------------------------------------
void IRAM_ATTR recordInterrupt();
void IRAM_ATTR tareInterrupt();
void IRAM_ATTR modeInterrupt();
void IRAM_ATTR upInterrupt();
void IRAM_ATTR downInterrupt();
void lcd_(float w);
void sort_(int32_t arr[], uint8_t n, int32_t avg);
int32_t getData(uint8_t K = 50, int32_t *sensor_error = NULL, uint8_t N = 5);
float getWeight(uint8_t K = 50, int32_t *sensor_error = NULL, uint8_t N = 5);
void sleep_();

//-----------------------------------------------------------------------------------------------
#line 73 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
void setup();
#line 101 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
void loop();
#line 308 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
bool delay_(uint32_t timeout);
#line 334 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
int32_t getData(uint8_t K, int32_t *sensor_error, uint8_t N);
#line 378 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
float getWeight(uint8_t K, int32_t *sensor_error, uint8_t N);
#line 73 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
void setup()
{
  Serial.begin(57600);
  // Serial.println("Starting");
  pinMode(TARE, INPUT_PULLUP);
  pinMode(MODE, INPUT_PULLUP);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(RECORD, INPUT_PULLUP);
  attachInterrupt(RECORD, recordInterrupt, RISING);
  attachInterrupt(TARE, tareInterrupt, RISING);
  attachInterrupt(MODE, modeInterrupt, RISING);
  attachInterrupt(UP, upInterrupt, RISING);
  attachInterrupt(DOWN, downInterrupt, RISING);
  Wire.begin(SDA, SCL);

  sensor.init();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(" Digital Scale");
  lcd.setCursor(0, 1);
  lcd.print(" SOICT - HUST");
  delay(1000);
  lcd.clear();
}
//-----------------------------------------------------------------------------------------------
void loop()
{
  lcd.setCursor(1, 0);
  lcd.print("Digital Scale");

  w = getWeight();

  // display weighting results
  lcd_(w);

  // feature: Auto turn off the screen backlight
  // if the weighing result does not change by more than (ABSOLUTE_ERROR)kg in 3s
  if (abs(w - w_prev) < ABSOLUTE_ERROR)
  {
    if (millis() - timer > AUTO_SLEEP_TIME)
    {
      if (w == 0)
      {
        sleep_();
      }
      else
      {
        lcd.noBacklight();
        delay_(FLICKER_DELAY);
        lcd.backlight();
        delay_(FLICKER_DELAY);
        lcd.noBacklight();
        delay_(FLICKER_DELAY);
        lcd.backlight();
        delay_(SLEEP_DELAY);
        _sleep = true;
        sleep_();
        if (_sleep == false)
        {
          lcd.backlight();
          timer = millis();
        }
      }
    }
  }
  else
    timer = millis();

  // feature: Save the results of the last RECORD_NUM weightings
  if (millis() - timer > RECORD_TIME && abs(w - record[q]) > ABSOLUTE_ERROR && abs(w) > ABSOLUTE_ERROR)
  {
    q++;
    if (q == RECORD_NUM)
      q = 0;
    record[q] = w;
    Serial.println(String("Record: " + String(w)));
  }

  if (_interrupt == true)
  {
    // feature: View the results of the last RECORD_NUM weightings
    while (_record == true)
    {
      k++;
      Serial.println(k);
      if (k == RECORD_NUM + 1)
        k = 0;

      _record = false;
      _interrupt = false;
      if (k != 0)
      {
        lcd.clear();
        lcd.backlight();
        lcd.setCursor(1, 0);
        lcd.print("Record Weight: ");
        lcd_(record[(q - k + RECORD_NUM + 1) % RECORD_NUM]);
        if (delay_(SHOW_RECORD_DELAY) == false)
          continue;
      }

      lcd.clear();
      timer = millis();
      if (_sleep == true)
        sleep_();
    }

    // feature: Adjust the scale to 0 kg
    if (_tare == true)
    {
      lcd.setCursor(0, 0);
      lcd.print(" Digital Scale");
      lcd.setCursor(0, 1);
      lcd.print(" Taring...       ");
      for (uint8_t i = 0; i < 5; i++)
      {
        Zero = getData(50, &sensor_error);
        if (sensor_error < (int)(Scale * (ABSOLUTE_ERROR / 10)))
          break;
      }
      sensor.setZero(Zero);
      _tare = false;
      _interrupt = false;
      delay_(100);
      timer = millis();
    }

    // feature: Change weight unit from kilogram to pound
    if (_mode == true)
    {
      Mode = (Mode == KG_MODE) ? LB_MODE : KG_MODE;
      lcd_(w);
      _mode = false;
      _interrupt = false;
    }

    // feature: Adjust weighting result up
    while (_up == true)
    {
      Scale -= 0.5;
      sensor.setScale(Scale);
      lcd_(getWeight(10));
      if (digitalRead(UP) == LOW)
      {
        Scale -= 5;
        continue;
      }
      _up = false;
      _interrupt = false;
      timer = millis();
    }

    // feature: Adjust weighting result down
    while (_down == true)
    {
      Scale += 0.5;
      sensor.setScale(Scale);
      lcd_(getWeight(10));
      if (digitalRead(DOWN) == LOW)
      {
        Scale += 5;
        continue;
      }
      _down = false;
      _interrupt = false;
      timer = millis();
    }
  }
  w_prev = w;
  delay(20);
}

//-----------------------------------------------------------------------------------------------

void lcd_(float w)
{
  if (w < 0 && w > -ABSOLUTE_ERROR)
    return;
  lcd.setCursor(1, 1);
  if (Mode == LB_MODE)
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

void sleep_()
{
  lcd.clear();
  lcd.noBacklight();
  uint8_t i = 0;
  uint8_t flag = 0;
  while (_interrupt == false)
  {
    float temp = getWeight(10);

    if (flag == 1)
      if (abs(temp) > ABSOLUTE_ERROR)
      {
        _sleep = false;
        break;
      }
      else
        continue;

    if (abs(temp) < ABSOLUTE_ERROR)
    {
      flag = 1;
      continue;
    }

    if (abs(w - temp) > ABSOLUTE_ERROR)
    {
      i++;
      if (i == 3)
      {
        _sleep = false;
        break;
      }
    }
    else
      i = 0;

    delay(50);
  }
  lcd.backlight();
}
bool delay_(uint32_t timeout)
{
  int i = timeout / 20;
  while (_interrupt == false && i > 0 && abs(w - getWeight(10)) < ABSOLUTE_ERROR)
  {
    i--;
    delay(20);
  }
  return (i == 0) ? true : false;
}

void sort_(int32_t arr[], uint8_t n, int32_t avg)
{
  for (int i = 1; i < n; i++)
  {
    int key = arr[i];
    int j = i - 1;
    while (j >= 0 && abs(arr[j] - avg) > abs(key - avg))
    {
      arr[j + 1] = arr[j];
      j--;
    }
    arr[j + 1] = key;
  }
}

int32_t getData(uint8_t K, int32_t *sensor_error, uint8_t N)
{
  int32_t w[N];
  int32_t w_avg = 0;
  int32_t w_worst = 0;
  int32_t temp = 0;
  uint8_t k = 0;

  for (uint8_t i = 0; i < N; i++)
  {
    temp = sensor.getData();
    if (abs(temp) < Scale * MAX_LOAD)
    {
      w[i] = temp;
      w_avg += temp;
    }
    else
    {
      i--;
    }
  }

  w_avg /= N;
  sort_(w, N, w_avg);
  w_worst = abs(w[N - 1] - w_avg);
  while (k < K && w_worst > (int)(Scale * (ABSOLUTE_ERROR / 2)))
  {
    temp = sensor.getData();
    if (abs(temp - w_avg) < w_worst)
    {
      w_avg += (temp - w[N - 1]) / N;
      w[N - 1] = temp;
      sort_(w, N, w_avg);
      w_worst = abs(w[N - 1] - w_avg);
    }
    k++;
  }

  if (sensor_error != NULL)
  {
    *sensor_error = w_worst;
  }
  return w_avg;
}
float getWeight(uint8_t K, int32_t *sensor_error, uint8_t N)
{
  return (long)(getData(K, sensor_error, N) - Zero) / Scale;
}
//-----------------------------------------------------------------------------------------------
void IRAM_ATTR recordInterrupt()
{
  if (millis() - recordPrev < DEBOUNCE_TIME)
    return;
  recordPrev = millis();
  _record = true;
  _interrupt = true;
}

void IRAM_ATTR tareInterrupt()
{
  if (millis() - tarePrev < DEBOUNCE_TIME)
    return;
  tarePrev = millis();
  _tare = true;
  _interrupt = true;
}

void IRAM_ATTR modeInterrupt()
{
  if (millis() - modePrev < DEBOUNCE_TIME)
    return;
  modePrev = millis();
  _mode = true;
  _interrupt = true;
}

void IRAM_ATTR upInterrupt()
{
  if (millis() - upPrev < DEBOUNCE_TIME)
    return;
  upPrev = millis();
  _up = true;
  _interrupt = true;
}

void IRAM_ATTR downInterrupt()
{
  if (millis() - downPrev < DEBOUNCE_TIME)
    return;
  downPrev = millis();
  _down = true;
  _interrupt = true;
}
