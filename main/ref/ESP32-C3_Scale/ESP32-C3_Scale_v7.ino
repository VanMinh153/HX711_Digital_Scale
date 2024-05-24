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
#define ABSOLUTE_ERROR 0.1f
#define AUTO_SLEEP_TIME 3600
#define RECORD_TIME 720
#define FLICKER_DELAY 540
#define SLEEP_DELAY 2400
#define SHOW_RECORD_DELAY 2400 // not merge with show interrupt result delay
#define SHOW_INTERRUPT_RESULT_DELAY 1800
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
uint32_t Absolute_error = (uint32_t)(Scale * ABSOLUTE_ERROR);
int32_t _d = 0;
float _w = 0;
float _w_prev = 0;
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
bool _sleep = false;
bool _detect = false;

//-----------------------------------------------------------------------------------------------
void IRAM_ATTR recordInterrupt();
void IRAM_ATTR tareInterrupt();
void IRAM_ATTR modeInterrupt();
void IRAM_ATTR upInterrupt();
void IRAM_ATTR downInterrupt();
void lcd_(float _w);
void sort_(int32_t arr[], uint8_t n, int32_t avg);
int32_t getData(uint8_t K = 50, int32_t *sensor_error = NULL, uint8_t N = 5);
float getWeight(uint8_t K = 50, int32_t *sensor_error = NULL, uint8_t N = 5);
float toWeight(int32_t data);
void sleep_();
bool delay_(uint32_t timeout);
bool delay2_(uint32_t timeout);

//-----------------------------------------------------------------------------------------------
void setup()
{
  Serial.begin(57600);
  pinMode(TARE, INPUT_PULLUP);
  pinMode(MODE, INPUT_PULLUP);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(RECORD, INPUT_PULLUP);
  attachInterrupt(TARE, tareInterrupt, RISING);
  attachInterrupt(MODE, modeInterrupt, RISING);
  attachInterrupt(UP, upInterrupt, RISING);
  attachInterrupt(DOWN, downInterrupt, RISING);
  attachInterrupt(RECORD, recordInterrupt, RISING);
  Wire.begin(SDA, SCL);

  sensor.init();

  lcd.init();
  lcd.backlight();
  lcd.noCursor();
  lcd.setCursor(1, 0);
  lcd.print("Digital Scale");
  lcd.setCursor(1, 1);
  lcd.print("SOICT - HUST");
  delay(1000);
  lcd.clear();
}
//-----------------------------------------------------------------------------------------------
void loop()
{
  lcd.setCursor(1, 0);
  lcd.print("Digital Scale");

  _d = getData();
  _w = toWeight(_d);
  lcd_(_w);
  if (_detect == true)
    _detect = false;

  // feature: Auto turn off the screen backlight
  // if the weighing result does not change by more than (ABSOLUTE_ERROR)kg in 3s
  if (abs(_w - _w_prev) < ABSOLUTE_ERROR)
  {
    while (millis() - timer > AUTO_SLEEP_TIME)
    {
      if (_w == 0)
      {
        _sleep = true;
        sleep_();
        break;
      }
      for (uint8_t i = 0; i < 2; i++)
      {
        lcd.noBacklight();
        delay2_(FLICKER_DELAY);
        if (_detect == true)
        {
          lcd.backlight();
          break;
        }
        lcd.backlight();
        delay2_(FLICKER_DELAY);
        if (_detect == true)
          break;
      }
      if (_detect == true)
        break;
      delay2_(SLEEP_DELAY);
      if (_detect == true)
        break;
      _sleep = true;
      sleep_();
      break;
    }
  }
  else
    timer = millis();

  // feature: Save the results of the last RECORD_NUM weightings
  if (millis() - timer > RECORD_TIME && abs(_w - record[q]) > ABSOLUTE_ERROR && abs(_w) > ABSOLUTE_ERROR)
  {
    q++;
    if (q == RECORD_NUM)
      q = 0;
    record[q] = _w;
    Serial.println(String("Record: " + String(_w)));
  }
  _w_prev = _w;

  // -----------------------------------------------------------------
  // Interrupt handling
  while (_interrupt == true)
  {
    bool show_result = true;
    lcd.clear();
    // feature: View the results of the last RECORD_NUM weightings
    if (_record == true)
    {
      lcd.setCursor(1, 0);
      lcd.print("Record Weight:");
      show_result = false;
    }
    else
    {
      lcd.setCursor(1, 0);
      lcd.print("Digital Scale");
    }

    k = 0;
    while (_record == true)
    {
      _record = false;
      k++;
      if (k == RECORD_NUM + 1)
      {
        k = 0;
        break;
      }

      lcd_(record[(q - k + RECORD_NUM + 1) % RECORD_NUM]);
      _interrupt = false;
      if (delay_(SHOW_RECORD_DELAY) == false)
        continue;
    }

    // feature: Adjust the scale to 0 kg
    if (_tare == true)
    {
      _tare = false;
      lcd.setCursor(1, 1);
      lcd.print("Taring...       ");
      for (uint8_t i = 0; i < 5; i++)
      {
        Zero = getData(50, &sensor_error);
        if (sensor_error < (int)(Scale * (ABSOLUTE_ERROR / 5)))
          break;
      }
      sensor.setZero(Zero);
      lcd_(getWeight(10));
    }

    // feature: Change weight unit from kilogram to pound
    if (_mode == true)
    {
      _mode = false;
      Mode = (Mode == KG_MODE) ? LB_MODE : KG_MODE;
      lcd_(_w);
    }

    // feature: Adjust weighting result up
    float w;
    while (_up == true)
    {
      _up = false;
      Scale -= 0.5;
      Absolute_error = (uint32_t)(Scale * ABSOLUTE_ERROR);
      sensor.setScale(Scale);
      if (_sleep == true)
      {
        w = _d / Scale;
        lcd_(w);
      }
      lcd_(getWeight(10));

      if (digitalRead(UP) == LOW)
      {
        Scale -= 5;
        continue;
      }
    }

    // feature: Adjust weighting result down
    while (_down == true)
    {
      _down = false;
      Scale += 0.5;
      Absolute_error = (uint32_t)(Scale * ABSOLUTE_ERROR);
      sensor.setScale(Scale);
      if (_sleep == true)
      {
        w = _d / Scale;
        lcd_(w);
      }
      lcd_(getWeight(10));
      if (digitalRead(DOWN) == LOW)
      {
        Scale += 5;
        continue;
      }
    }
    _interrupt = false;
    if (show_result == true)
      delay_(SHOW_INTERRUPT_RESULT_DELAY);
    lcd.clear();

    if (_sleep == true)
      sleep_();
  }
  delay(50);
}

//-----------------------------------------------------------------------------------------------

void lcd_(float _w)
{
  if (_w < 0 && _w > -ABSOLUTE_ERROR)
    return;
  lcd.setCursor(1, 1);
  if (Mode == LB_MODE)
  {
    lcd.print(_w * KG_TO_LB);
    lcd.print(" lb    ");
  }
  else
  {
    lcd.print(_w);
    lcd.print(" kg    ");
  }
}

void sleep_()
{
  lcd.clear();
  lcd.noBacklight();
  uint8_t count = 0;
  uint8_t flag = 0;
  while (_interrupt == false)
  {
    float d = getData(10);

    if (flag == 1)
      if (abs(d) > Absolute_error)
      {
        _detect = true;
        break;
      }
      else
        continue;

    if (abs(d) < Absolute_error)
    {
      flag = 1;
      continue;
    }

    if (abs(_d - d) > Absolute_error)
    {
      count++;
      if (count == 3)
      {
        _detect = true;
        break;
      }
    }
    else
      count = 0;
    delay(50);
  }
  lcd.backlight();
  if (_detect == true)
    timer = millis();
}

// Delay function with the ability to detect the interruption signal and weight changes
bool delay2_(uint32_t timeout)
{
  _detect = false;
  int t = millis() + timeout;
  uint8_t count = 0;
  uint8_t flag = 0;
  while (_interrupt == false && millis() < t)
  {
    delay(50);

    float d = getData(10);

    if (flag == 1)
      if (abs(d) > Absolute_error)
      {
        _detect = true;
        break;
      }
      else
        continue;

    if (abs(d) < Absolute_error)
    {
      flag = 1;
      continue;
    }

    if (abs(_d - d) > Absolute_error)
    {
      count++;
      if (count == 3)
      {
        _detect = true;
        break;
      }
    }
    else
      count = 0;
  }
  if (_detect == true)
    timer = millis();
  return (millis() >= t) ? true : false;
}

bool delay_(uint32_t timeout)
{
  int t = millis() + timeout;
  while (_interrupt == false && millis() < t)
  {
    delay(50);
  }
  return (millis() >= t) ? true : false;
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
  int32_t _w[N];
  int32_t w_avg = 0;
  int32_t w_worst = 0;
  int32_t d = 0;
  uint8_t k = 0;

  for (uint8_t i = 0; i < N; i++)
  {
    d = sensor.getData();
    if (abs(d) < Scale * MAX_LOAD)
    {
      _w[i] = d;
      w_avg += d;
    }
    else
    {
      i--;
    }
  }

  w_avg /= N;
  sort_(_w, N, w_avg);
  w_worst = abs(_w[N - 1] - w_avg);
  while (k < K && w_worst > (int)(Scale * Absolute_error))
  {
    d = sensor.getData();
    if (abs(d - w_avg) < w_worst)
    {
      w_avg += (d - _w[N - 1]) / N;
      _w[N - 1] = d;
      sort_(_w, N, w_avg);
      w_worst = abs(_w[N - 1] - w_avg);
    }
    k++;
  }

  if (sensor_error != NULL)
  {
    *sensor_error = w_worst;
  }
  return w_avg;
}

int32_t getData_(uint8_t K, int32_t *sensor_error, uint8_t N)
{
  int32_t _w[N];
  int32_t w_avg = 0;
  int32_t w_worst = 0;
  int32_t d = 0;

  for (uint8_t i = 0; i < N; i++)
  {
    d = sensor.getData();
    if (abs(d) < Scale * MAX_LOAD)
    {
      _w[i] = d;
      w_avg += d;
    }
    else
    {
      i--;
    }
  }

  w_avg /= N;
  sort_(_w, N, w_avg);
  w_worst = abs(_w[N - 1] - w_avg);

  int32_t d_prev = 0;
  int32_t distance = w_worst;
  uint8_t count = 0;
  uint8_t k = 0;
  while (k < K && w_worst > (int)(Scale * Absolute_error))
  {
    d = sensor.getData();
    if (abs(d - w_avg) < w_worst)
    {
      w_avg += (d - _w[N - 1]) / N;
      _w[N - 1] = d;
      sort_(_w, N, w_avg);
      w_worst = abs(_w[N - 1] - w_avg);
    }
    k++;
    if (d - d_prev > distance)
      count++;
    else
      count = 0;
    d_prev = d;
  }

  if (sensor_error != NULL)
  {
    *sensor_error = w_worst;
  }
  return w_avg;
}

float toWeight(int32_t data)
{
  return (long)(data - Zero) / Scale;
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