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
#define SHOW_RECORD_DELAY 2400
#define SHOW_INTERRUPT_RESULT_DELAY 1800
#define DEBOUNCE_TIME 200
#define RECORD_NUM 3
#define KG_MODE 0
#define LB_MODE 1
#define KG_TO_LB 2.204623f
#define MAIN_TITLE "Digital Scale"

HX711 sensor(DOUT, PD_SCK, GAIN_128, SCALE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

float Scale = SCALE;
int32_t Zero = 0;
byte Mode = KG_MODE;
uint32_t Absolute_error = (uint32_t)(Scale * ABSOLUTE_ERROR);
int32_t _d = 0;
float _w = 0;
float prev_w = 0;
int32_t sensor_error = 0;
unsigned long timer = millis();
float record_w[RECORD_NUM];
byte q = 0;
byte k = 0;
volatile byte tare = 0;
volatile byte mode = 0;
volatile byte up = 0;
volatile byte down = 0;
volatile byte record = 0;
volatile byte interrupt = 0;
unsigned long prev_record = millis();
unsigned long prev_tare = millis();
unsigned long prev_mode = millis();
unsigned long prev_up = millis();
unsigned long prev_down = millis();
byte _sleep = 0;
byte detect = 0;

//----------------------------------------------------------------------------------------------------------------------
void IRAM_ATTR recordInterrupt();
void IRAM_ATTR tareInterrupt();
void IRAM_ATTR modeInterrupt();
void IRAM_ATTR upInterrupt();
void IRAM_ATTR downInterrupt();
void lcd_(float _w);
void sort_(int32_t arr[], byte n, int32_t avg);
int32_t getData(byte K = 50, int32_t *sensor_error = NULL, byte N = 5);
float getWeight(byte K = 50, int32_t *sensor_error = NULL, byte N = 5);
float toWeight(int32_t data);
void sleep_();
byte delay_(uint32_t timeout);
byte delay2_(uint32_t timeout);

//----------------------------------------------------------------------------------------------------------------------
void setup()
{
  Serial.begin(57600);
  pinMode(TARE, INPUT_PULLUP);
  pinMode(MODE, INPUT_PULLUP);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(RECORD, INPUT_PULLUP);
  attachInterrupt(TARE, tareInterrupt, FALLING);
  attachInterrupt(MODE, modeInterrupt, FALLING);
  attachInterrupt(UP, upInterrupt, FALLING);
  attachInterrupt(DOWN, downInterrupt, FALLING);
  attachInterrupt(RECORD, recordInterrupt, FALLING);
  Wire.begin(SDA, SCL);

  sensor.init();

  lcd.init();
  lcd.backlight();
  lcd.noCursor();
  lcd.setCursor(1, 0);
  lcd.print(MAIN_TITLE);
  lcd.setCursor(1, 1);
  lcd.print("SOICT - HUST");
  delay(1000);
  lcd.setCursor(1, 1);
  lcd.print("            ");
}
//----------------------------------------------------------------------------------------------------------------------
void loop()
{
  byte _interrupt = 0;
   if (_sleep = true || _interrupt == 1)
  {
    _sleep = false;
    lcd.setCursor(1, 0);
    lcd.print(MAIN_TITLE);
  }
  _d = getData();
  _w = toWeight(_d);
  lcd_(_w);

  // feature: Auto turn off the screen backlight
  // if the weighing result does not change by more than (ABSOLUTE_ERROR)kg in 3s
  if (detect == true)
    detect = false;
  if (abs(_w - prev_w) < ABSOLUTE_ERROR)
  {
    while (millis() - timer > AUTO_SLEEP_TIME)
    {
      if (_w == 0)
      {
        _sleep = true;
        sleep_();
        break;
      }
      for (byte i = 0; i < 2; i++)
      {
        lcd.noBacklight();
        if (delay2_(FLICKER_DELAY) != 0)
        {
          lcd.backlight();
          break;
        }

        lcd.backlight();
        if (delay2_(FLICKER_DELAY) != 0)
          break;
      }
      if (delay2_(FLICKER_DELAY) != 0)
        break;
      _sleep = true;
      sleep_();
      break;
    }
  }
  else
    timer = millis();

  // feature: Save the results of the last RECORD_NUM weightings
  if (millis() - timer > RECORD_TIME && abs(_w - record_w[q]) > ABSOLUTE_ERROR && abs(_w) > ABSOLUTE_ERROR)
  {
    q++;
    if (q == RECORD_NUM)
      q = 0;
    record_w[q] = _w;
    Serial.println(String("Record: " + String(_w)));
  }
  prev_w = _w;

  //----------------------------------------------------------------------------------------------------------------------
  // Interrupt handling
  while (tare == 1 || mode == 1 || up == 1 || down == 1 || record == 1)
  {
    _interrupt = 1;
    Serial.println('*');
    byte _tare = 0;
    byte _mode = 0;
    byte _up = 0;
    byte _down = 0;
    byte _record = 0;

    //
    if (tare == 1 || mode == 1)
    {
      lcd.setCursor(1, 0);
      lcd.print("Digital Scale   ");
    }
    // feature: Adjust the scale to 0 kg
    if (tare == 1)
    {
      _tare = 1;
      tare = 0;
      lcd.setCursor(1, 1);
      lcd.print("Taring...       ");
      for (byte i = 0; i < 5; i++)
      {
        Zero = getData(50, &sensor_error);
        if (sensor_error < (int)(Scale * (ABSOLUTE_ERROR / 5)))
          break;
      }
      sensor.setZero(Zero);
      lcd_(getWeight(10));
    }

    // feature: Change weight unit from kilogram to pound
    if (mode == 1)
    {
      _mode = 1;
      mode = 0;
      Mode = (Mode == KG_MODE) ? LB_MODE : KG_MODE;
      lcd_(_w);
    }
    //
    if (up == 1 || down == 1)
    {
      lcd.setCursor(1, 0);
      lcd.print("Adjust Scale    ");
    }
    // feature: Adjust weighting result up
    float w;
    while (up == 1)
    {
      _up = 1;
      Scale -= 0.5;
      w = _d / Scale;
      lcd_(w);

      if (digitalRead(UP) == LOW)
        Scale -= 2;
      else
        up = 0;
    }

    // feature: Adjust weighting result down
    while (down == 1)
    {
      _down = 1;
      Scale += 0.5;
      w = _d / Scale;
      lcd_(w);

      if (digitalRead(DOWN) == LOW)
        Scale += 2;
      else
        down = 0;
    }

    // feature: View the results of the last weightings
    if (record == 1)
    {
      _record = 1;
      lcd.setCursor(1, 0);
      lcd.print("Record Weight:  ");
    }
    k = 0;
    while (record == 1)
    {
      record = false;
      k++;
      if (k == RECORD_NUM + 1)
      {
        k = 0;
        break;
      }

      lcd_(record_w[(q - k + RECORD_NUM + 1) % RECORD_NUM]);
      if (delay_(SHOW_RECORD_DELAY) == false)
        continue;
    }
    //----------------------------------------------------------------------------------------------------------------------
    if (_up == 1 || _down == 1)
    {
      Absolute_error = (uint32_t)(Scale * ABSOLUTE_ERROR);
      sensor.setScale(Scale);
    }

    if (_record == 1)
      NULL;
    else
    {
      if (delay_(SHOW_INTERRUPT_RESULT_DELAY) == 1)
        continue;
    }

    if (_sleep == true)
      sleep_();
  }

  delay(50);
}

// Additional functions
//----------------------------------------------------------------------------------------------------------------------
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
  Serial.println("sleep_");
  lcd.clear();
  lcd.noBacklight();
  byte flag = 0;
  int32_t d = 0;
  byte prev_interrupt = interrupt;

  while (interrupt == prev_interrupt)
  {
    delay(500);

    d = getData(20);
    Serial.println(d);
    
    if (flag == 1)
    {
      Serial.println(flag);
      if (abs(d) > Absolute_error)
        break;
      else
        continue;
    }

    if (abs(d) < 5 * Absolute_error)
    {
      flag = 1;
      continue;
    }

    if (abs(_d - d) > 5 * Absolute_error)
    {
      delay(50);
      if (abs(d - getData(20)) < 2 * Absolute_error)
        break;
    }
  }
  lcd.backlight();
}

// Delay function with the ability to detect the interruption signal and weight changes
byte delay2_(uint32_t timeout)
{
  Serial.print("delay2_");
  Serial.print(detect);
  if (detect == 1)
    return 1;

  unsigned long t = millis() + timeout;
  byte flag = 0;
  int32_t d = 0;
  byte prev_interrupt = interrupt;

  while (interrupt == prev_interrupt && millis() < t)
  {
    delay(200);

    d = getData(10);

    if (flag == 1)
      if (abs(d) > Absolute_error)
        break;
      else
        continue;

    if (abs(d) < Absolute_error)
    {
      flag = 1;
      continue;
    }

    if (abs(_d - d) > 5 * Absolute_error)
    {
      delay(50);
      if (abs(d - getData(20)) < Absolute_error)
        break;
    }
  }
  if (millis() >= t)
    return 0;
  else
  {
    detect = 1;
    return 1;
  }
}

byte delay_(uint32_t timeout)
{
  Serial.println("delay_");
  unsigned long t = millis() + timeout;
  byte prev_interrupt = interrupt;
  while (interrupt == prev_interrupt && millis() < t)
  {
    delay(50);
  }
  return (millis() >= t) ? 0 : 1;
}

void sort_(int32_t arr[], byte n, int32_t avg)
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

int32_t getData(byte K, int32_t *sensor_error, byte N)
{
  int32_t d[N];
  int32_t d_avg = 0;
  int32_t d_worst = 0;
  int32_t d2 = 0;
  byte k = 0;

  for (byte i = 0; i < N; i++)
  {
    d2 = sensor.getData();
    if (abs(d2) < Scale * MAX_LOAD)
    {
      d[i] = d2;
      d_avg += d2;
    }
    else
    {
      i--;
    }
  }

  d_avg /= N;
  sort_(d, N, d_avg);
  d_worst = abs(d[N - 1] - d_avg);
  while (k < K && d_worst > (int)(Scale * Absolute_error))
  {
    d2 = sensor.getData();
    if (abs(d2 - d_avg) < d_worst)
    {
      d_avg += (d2 - d[N - 1]) / N;
      d[N - 1] = d2;
      sort_(d, N, d_avg);
      d_worst = abs(d[N - 1] - d_avg);
    }
    k++;
  }

  if (sensor_error != NULL)
  {
    *sensor_error = d_worst;
  }
  return d_avg;
}

float toWeight(int32_t data)
{
  return (long)(data - Zero) / Scale;
}

float getWeight(byte K, int32_t *sensor_error, byte N)
{
  return (long)(getData(K, sensor_error, N) - Zero) / Scale;
}
//----------------------------------------------------------------------------------------------------------------------
void IRAM_ATTR recordInterrupt()
{
  if (millis() - prev_record < DEBOUNCE_TIME)
    return;
  prev_record = millis();
  record = 1;
  interrupt++;
}

void IRAM_ATTR tareInterrupt()
{
  if (millis() - prev_tare < DEBOUNCE_TIME)
    return;
  prev_tare = millis();
  tare = 1;
  interrupt++;
}

void IRAM_ATTR modeInterrupt()
{
  if (millis() - prev_mode < DEBOUNCE_TIME)
    return;
  prev_mode = millis();
  mode = 1;
  interrupt++;
}

void IRAM_ATTR upInterrupt()
{
  if (millis() - prev_up < DEBOUNCE_TIME)
    return;
  prev_up = millis();
  up = 1;
  interrupt++;
}

void IRAM_ATTR downInterrupt()
{
  if (millis() - prev_down < DEBOUNCE_TIME)
    return;
  prev_down = millis();
  down = 1;
  interrupt++;
}