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
#define AUTO_SLEEP_TIME 2500
#define RECORD_TIME 600
#define FLICKER_DELAY 300
#define SLEEP_DELAY 3500
#define SHOW_RECORD_DELAY 2500
#define SHOW_INTERRUPT_RESULT_DELAY 2500
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
uint16_t Absolute_error = (uint16_t)(Scale * ABSOLUTE_ERROR);
int32_t _d = 0;
float _w = 0;
float prev_w = 0;
unsigned long timer = millis();
float record_w[RECORD_NUM];
byte q = 0;
byte k = 0;
uint32_t sensor_error = 0;
volatile byte tare = 0;
volatile byte mode = 0;
volatile byte up = 0;
volatile byte down = 0;
volatile byte record = 0;
volatile byte interrupt = 0;
byte prev_interrupt = 0;
unsigned long prev_record = millis();
unsigned long prev_tare = millis();
unsigned long prev_mode = millis();
unsigned long prev_up = millis();
unsigned long prev_down = millis();
byte _sleep = 0;
byte detect = 0;
byte _interrupt = 0;

//----------------------------------------------------------------------------------------------------------------------
void IRAM_ATTR recordInterrupt();
void IRAM_ATTR tareInterrupt();
void IRAM_ATTR modeInterrupt();
void IRAM_ATTR upInterrupt();
void IRAM_ATTR downInterrupt();
void lcd_(float _w);
void sort_(int32_t arr[], byte n, int32_t avg);
int32_t getData_(uint32_t *error = &sensor_error);
int32_t getData(uint16_t delay_time = 0);
float getWeight();
float toWeight(int32_t data);
byte sleep_(byte sensitivity = 2);
byte sleep2_();
byte delay_has_listen(uint16_t timeout, uint16_t time2listen = 50, uint16_t error = Absolute_error);
byte delay_(uint32_t timeout);

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
  if (_sleep = true || _interrupt == 1)
  {
    _sleep = false;
    _interrupt = 0;
    lcd.setCursor(1, 0);
    lcd.print(MAIN_TITLE);
  }
  _d = getData(200);
  _w = toWeight(_d);
  lcd_(_w);

  // feature: Auto turn off the screen backlight
  // if the weighing result does not change by more than (ABSOLUTE_ERROR)kg in 3s
  if (detect == true)
    detect = false;
  if (abs(_w - prev_w) < 2 * ABSOLUTE_ERROR)
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
        if (delay_has_listen(FLICKER_DELAY) != 0)
        {
          lcd.backlight();
          break;
        }

        lcd.backlight();
        if (delay_has_listen(FLICKER_DELAY) != 0)
          break;
      }
      if (delay_has_listen(FLICKER_DELAY) != 0)
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
    Serial.println(String("\r\nRecord: " + String(_w)));
  }
  prev_w = _w;

  //----------------------------------------------------------------------------------------------------------------------
  // Interrupt handling
  while (tare == 1 || mode == 1 || up == 1 || down == 1 || record == 1)
  {
    prev_interrupt = interrupt;
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
        Zero = getData_();
        if (sensor_error < (int)(Scale * (ABSOLUTE_ERROR / 5)))
          break;
      }
      sensor.setZero(Zero);
      lcd_(getWeight());
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
      delay(100);
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
      {
        Scale -= 2;
      }
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
      record = 0;
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
  prev_interrupt = interrupt;

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

byte sleep2_()
{
  lcd.clear();
  lcd.noBacklight();
  byte flag = 0;
  int32_t d = 0;

  while (interrupt == prev_interrupt)
  {
    delay(500);

    d = getData(200);

    if (flag == 1)
    {
      if (abs(d) > Absolute_error)
      {
        detect = 1;
        break;
      }
      else
        continue;
    }

    if (abs(d) < 2 * Absolute_error)
    {
      flag = 1;
      continue;
    }

    if (abs(_d - d) > 2 * Absolute_error)
    {
      delay(50);
      if (abs(d - getData(200)) < 2 * Absolute_error)
      {
        detect = 1;
        break;
      }
    }
  }
  lcd.backlight();
  if (detect == 1)
  {
    timer = millis();
    return 1;
  }
  else
    return 2;
}


/**
 * @brief Delay function with the ability to detect the interruption signal and weight changes
 *
 * @param timeout     The maximum time to wait. if timeout = 0xffff, the function will wait indefinitely
 * @param time2listen The time to listen weight changes
 * @param error       Default is absolute error of the scale
 * @return byte       0: timeout, 1: weight changes, 2: interrupt signal
 */
byte delay_has_listen(uint16_t timeout, uint16_t time2listen, uint16_t error)
{
  if (detect == 1)
    return 1;

  unsigned long t = millis();
  unsigned long TIME_END = t + timeout;
  byte flag = 0;
  int32_t d = 0;

  while (interrupt == prev_interrupt)
  {
    if (timeout == 0xffff)
    {
      delay(time2listen);
    }
    else if (TIME_END > t + time2listen)
    {
      delay(time2listen);
      t = millis();
    }
    else
    {
      delay(abs((int)(TIME_END - t)));
      t = TIME_END;
      break;
    }

    d = getData();

    if (flag == 1)
      if (abs(d) > error)
      {
        detect = 1;
        break;
      }
      else
        continue;

    if (abs(d) < error)
    {
      flag = 1;
      continue;
    }

    if (abs(_d - d) > error)
    {
      if (abs(d - getData()) < 2 * error)
      {
        detect = 1;
        break;
      }
    }
  }

  if (detect == 1)
  {
    timer = millis();
    return 1;
  }
  return (t == TIME_END) ? 0 : 2;
}

/**
 * @param sensitivity   Absolute error of the scale will be set to (sensitivity * Absolute_error)
 */
byte sleep_(byte sensitivity)
{
  lcd.clear();
  lcd.noBacklight();
  byte retval = 0;

  retval = delay_has_listen(0xffff, 300, sensitivity * Absolute_error);
  lcd.backlight();
  return retval;
}

byte delay_(uint32_t timeout)
{
  unsigned long t = millis() + timeout;
  prev_interrupt = interrupt;
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

int32_t getData_(uint32_t *error)
{
  const byte N = 5;
  const byte K = 10;
  int32_t d[N];
  int32_t d_avg = 0;
  int32_t d_worst = 0;
  int32_t d_temp = 0;
  byte k = 0;

  for (byte i = 0; i < N; i++)
  {
    d_temp = sensor.getData();
    if (abs(d_temp) < Scale * MAX_LOAD)
    {
      d[i] = d_temp;
      d_avg += d_temp;
    }
    else
    {
      i--;
    }
  }

  d_avg /= N;
  sort_(d, N, d_avg);
  d_worst = abs(d[N - 1] - d_avg);
  while (k < K && d_worst > Absolute_error)
  {
    d_temp = sensor.getData();
    if (abs(d_temp - d_avg) < d_worst)
    {
      d_avg += (d_temp - d[N - 1]) / N;
      d[N - 1] = d_temp;
      sort_(d, N, d_avg);
      d_worst = abs(d[N - 1] - d_avg);
    }
    k++;
  }

  if (k == K)
  {
    Serial.print(String('E' + String(k) + '-' + String(toWeight(d_avg))));
    for (byte i = 0; i < 5; i++)
    {
      Serial.print('-');
      Serial.print(toWeight(d[i]));
    }
  }
  else
  {
    Serial.print("_");
    if (k > 3)
      Serial.print(k);
  }

  if (error != NULL)
  {
    *error = d_worst;
  }
  return d_avg;
}
int32_t getData(uint16_t delay_time)
{
  int32_t d = getData_();
  if (sensor_error < Absolute_error)
    d = getData_();
  if (sensor_error < Absolute_error && delay_time > 0)
  {
    delay(delay_time);
    d = getData_();
  }

  if (sensor_error > Absolute_error)
    Serial.println(String("E-" + String(toWeight(d)) + '-' + String(sensor_error)));
  return d;
}

float toWeight(int32_t data)
{
  return (long)(data - Zero) / Scale;
}

float getWeight()
{
  return (long)(getData() - Zero) / Scale;
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