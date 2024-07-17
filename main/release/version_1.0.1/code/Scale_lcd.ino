#include "HX711-HEDSPI.h"
#include <LiquidCrystal_I2C.h>

#ifdef ARDUINO_ESP32C3_DEV
#define DOUT 6
#define PD_SCK 7
#define SDA 2
#define SCL 3
#define I2C_ADDRESS 0x27
#define TARE 4
#define MODE 5
#define DOWN 8
#define UP 9
#define RECORD 18
#elif defined(ARDUINO_LOLIN_S2_MINI)
#define DOUT 6
#define PD_SCK 7
#define SDA 33
#define SCL 35
#define I2C_ADDRESS 0x3c
#define TARE 4
#define MODE 5
#define DOWN 8
#define UP 9
#define RECORD 10
#endif

#define SCALE 420
// #define SCALE 1030
#define MAX_LOAD 200
// #define AVG_TIME2READ_G128 105 // 105ms
#define ABSOLUTE_ERROR 0.1f
#define AUTO_SLEEP_TIME 3000
#define RECORD_TIME 600
#define FLICKER_DELAY 350
#define SLEEP_DELAY 2000
#define SHOW_RECORD_TIME 2500
#define SHOW_ISR_TIME 2000
#define DEBOUNCE_TIME 200
#define RECORD_NUM 3
#define KG_MODE 0
#define LB_MODE 1
#define KG_TO_LB 2.204623f
#define MAIN_TITLE "Digital Scale"

HX711 sensor(DOUT, PD_SCK, GAIN_128, SCALE);
LiquidCrystal_I2C lcd(I2C_ADDRESS, 16, 2);

byte Gain = GAIN_128;
float Scale = SCALE;
int32_t Zero = 0;
byte Mode = MODE_VI;
uint16_t Absolute_error = (uint16_t)(Scale * ABSOLUTE_ERROR);
uint32_t sensor_error = 0;
volatile byte tare = 0;
volatile byte mode = 0;
volatile byte up = 0;
volatile byte down = 0;
volatile byte record = 0;
volatile byte interrupt = 0;
volatile long prev_press = millis();
byte prev_interrupt = 0;
int32_t _d = 0;
float prev_d = 0;
float _w = 0;
unsigned long timer = millis();
float record_w[RECORD_NUM];
byte q = 0;
byte k = 0;
int32_t prev_getData = 0;
byte _sleep = 0;
byte _detect = 0;
byte _isr = 0;

//----------------------------------------------------------------------------------------------------------------------
void IRAM_ATTR recordISR();
void IRAM_ATTR tareISR();
void IRAM_ATTR modeISR();
void IRAM_ATTR upISR();
void IRAM_ATTR downISR();
void lcd_(float _w);
void sort_(int32_t arr[], byte n, int32_t avg);
int32_t getData_Avg();
int32_t getData_(byte allow_delay = 0);
float getWeight();
float toWeight(int32_t data);
byte sleep_(byte sensitivity = 2);
byte delay_W(uint16_t timeout, uint16_t time2listen = 50, uint16_t error = Absolute_error);
byte delay_I(uint32_t timeout, volatile byte *isrCtl = NULL);
void setGain(byte gain);

//----------------------------------------------------------------------------------------------------------------------
void setup()
{
  Serial.begin(57600);
  pinMode(TARE, INPUT_PULLUP);
  pinMode(MODE, INPUT_PULLUP);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(RECORD, INPUT_PULLUP);
  attachInterrupt(TARE, tareISR, RISING);
  attachInterrupt(MODE, modeISR, RISING);
  attachInterrupt(UP, upISR, RISING);
  attachInterrupt(DOWN, downISR, RISING);
  attachInterrupt(RECORD, recordISR, RISING);

  Wire.begin(SDA, SCL);
  sensor.init();
  lcd.init();
  delay(1000);
  getData_(true);
  getData_(true);
  getData_(true);

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
  if (_sleep = true || _isr == 1)
  {
    _sleep = false;
    _isr = 0;
    lcd.setCursor(1, 0);
    lcd.print(MAIN_TITLE);
  }
  _d = getData_(true);
  _w = toWeight(_d);
  lcd_(_w);

  // feature: Auto turn off the screen backlight
  // if the weighing result does not change by more than (ABSOLUTE_ERROR)kg in 3s
  if (_detect == true)
    _detect = false;
  if (abs(_d - prev_d) < 2 * Absolute_error)
  {
    while (millis() - timer > AUTO_SLEEP_TIME)
    {
      if (_d == 0)
      {
        _sleep = true;
        sleep_();
        break;
      }
      for (byte i = 0; i < 2; i++)
      {
        lcd.noBacklight();
        if (delay_W(FLICKER_DELAY) != 0)
        {
          lcd.backlight();
          break;
        }

        lcd.backlight();
        if (delay_W(FLICKER_DELAY) != 0)
          break;
      }
      if (delay_W(FLICKER_DELAY) != 0)
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
    Serial.println("Record: " + String(_w));
  }
  prev_d = _d;

  //----------------------------------------------------------------------------------------------------------------------
  // Interrupt handling
  while (tare == 1 || mode == 1 || up == 1 || down == 1 || record == 1)
  {
    // Serial.print('*');
    byte _tare = tare;
    byte _mode = mode;
    byte _up = up;
    byte _down = down;
    byte _record = record;
    prev_interrupt = interrupt;
    _isr = 1;
    //
    if (_tare == 1 || _mode == 1)
    {
      lcd.setCursor(1, 0);
      lcd.print("Digital Scale   ");
    }
    // feature: Adjust the scale to 0 kg
    if (_tare == 1)
    {
      tare = 0;
      lcd.setCursor(1, 1);
      lcd.print("Taring...       ");
      Zero = getData_();
      sensor.setZero(Zero);
      lcd_(getWeight());
    }

    // feature: Change weight unit from kilogram to pound
    if (_mode == 1)
    {
      mode = 0;
      Mode = (Mode == MODE_VI) ? LB_MODE : MODE_VI;
      lcd_(_w);
    }
    //
    if (_up == 1 || _down == 1)
    {
      delay(DEBOUNCE_TIME);
      lcd.setCursor(1, 0);
      lcd.print("Adjust Scale    ");
    }
    // feature: Adjust weighting result up
    float w;
    while (_up == 1 && up == 1)
    {
      Scale -= 0.5;
      w = _d / Scale;
      lcd_(w);

      if (digitalRead(UP) == LOW)
        Scale -= 2;
      else
        up = 0;
    }

    // feature: Adjust weighting result down
    while (_down == 1 && down == 1)
    {
      Scale += 0.5;
      w = _d / Scale;
      lcd_(w);

      if (digitalRead(DOWN) == LOW)
        Scale += 2;
      else
        down = 0;
    }

    // feature: View the results of the last weightings
    if (_record == 1)
    {
      lcd.setCursor(1, 0);
      lcd.print("Record Weight:  ");
    }
    k = 0;
    while (_record == 1 && record == 1)
    {
      record = 0;
      k++;
      if (k == RECORD_NUM + 1)
      {
        k = 0;
        break;
      }

      lcd_(record_w[(q - k + RECORD_NUM + 1) % RECORD_NUM]);
      if (delay_I(SHOW_RECORD_TIME, &record) == 1)
        prev_interrupt++;
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
      if (delay_I(SHOW_ISR_TIME) == 1)
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
/**
 * @param sensitivity   Absolute error of the scale will be set to (sensitivity * Absolute_error)
 */
byte sleep_(byte sensitivity)
{
  Serial.print("Sleeping...");
  lcd.clear();
  lcd.noBacklight();
  byte retval = 0;
  setGain(GAIN_64);

  retval = delay_W(0xffff, 500, sensitivity * Absolute_error);
  if (retval == 1)
    Serial.println(" > Awake: Detect Weight Changes");
  if (retval == 2)
    Serial.println(" > Awake: Detect Interrupt");
  setGain(GAIN_128);
  lcd.backlight();
  return retval;
}
/**
 * @brief Delay function with the ability to _detect the interruption signal and weight changes
 *
 * @param timeout     The maximum time to wait. if timeout = 0xffff, the function will wait indefinitely
 * @param time2listen The time to listen weight changes
 * @param error       Default is absolute error of the scale
 * @return byte       0: timeout, 1: weight changes, 2: interrupt signal
 */
byte delay_W(uint16_t timeout, uint16_t time2listen, uint16_t error)
{
  if (_detect == 1)
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
      d = getData_(true);
    }
    else if (TIME_END > t + time2listen + 105)
    {
      delay(time2listen);
      d = getData_();
      t = millis();
    }
    else
    {
      delay(abs((int)(TIME_END - t)));
      t = TIME_END;
      break;
    }

    if (flag == 1)
      if (abs(d - Zero) > 5 * error)
      {
        _detect = 1;
        break;
      }
      else
        continue;

    if (abs(d - Zero) < error)
    {
      flag = 1;
      continue;
    }

    if (abs(_d - d) > error)
    {
      if (abs(d - getData_()) < 2 * error)
      {
        _detect = 1;
        break;
      }
    }
  }

  if (_detect == 1)
  {
    timer = millis();
    return 1;
  }
  return (t == TIME_END) ? 0 : 2;
}
//
byte delay_I(uint32_t timeout, volatile byte *isrCtl)
{
  unsigned long t = millis() + timeout;
  if (isrCtl != NULL)
  {
    while (*isrCtl == 0 && millis() < t)
      delay(50);
  }
  else
  {
    prev_interrupt = interrupt;
    while (interrupt == prev_interrupt && millis() < t)
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

int32_t getData_Avg()
{
  const byte N = 5;
  const byte K = 5;
  int32_t d[N];
  int32_t d_avg = 0;
  uint32_t d_worst = 0;
  int32_t d_temp = 0;
  byte count = 0;
  byte countZ = 0;

  for (byte i = 0; i < 2 * N; i++)
  {
    d_temp = sensor.getData();
    if (d_temp == -1)
      continue;
    if (d_temp == prev_getData)
    {
      countZ++;
      if (countZ == 2 && count == 0)
        return d_temp;
      continue;
    }
    if (abs(d_temp) < Scale * MAX_LOAD)
    {
      d[count] = d_temp;
      d_avg += d_temp;
      count++;
      if (count == N)
        break;
    }
  }

  if (count < N)
    return 0x7fffff;
  d_avg /= N;
  sort_(d, N, d_avg);
  d_worst = abs(d[N - 1] - d_avg);

  count = 0;
  while (count < K && d_worst > Absolute_error)
  {
    d_temp = sensor.getData();
    if (d_temp == -1)
      continue;
    if (abs(d_temp - d_avg) < d_worst)
    {
      d_avg += (d_temp - d[N - 1]) / N;
      d[N - 1] = d_temp;
      sort_(d, N, d_avg);
      d_worst = abs(d[N - 1] - d_avg);
    }
    count++;
  }

  sensor_error = d_worst;
  prev_getData = d_avg;
  // Serial.print('_');
  return d_avg;
}
//
int32_t getData_(byte allow_delay)
{
  int32_t d = getData_Avg();
  if (d != 0x7fffff && sensor_error < Absolute_error)
    return d;

  if (allow_delay == 0)
    d = getData_Avg();

  if (allow_delay == 1)
  {
    for (byte i = 0; i < 2; i++)
    {
      delay(105);
      d = getData_Avg();
      if (sensor_error < Absolute_error)
        break;
    }
  }

  if (d == 0x7fffff)
  {
    Serial.println("Error: Failed to get data from HX711");
    return _d;
  }
  if (sensor_error > Absolute_error)
    Serial.println("Error Weight: " + String(toWeight(d)) + " +-" + String(sensor_error / Scale));
  return d;
}

float toWeight(int32_t data)
{
  return (long)(data - Zero) / Scale;
}

float getWeight()
{
  return (long)(getData_() - Zero) / Scale;
}

void setGain(byte gain)
{
  float k;
  if (Gain == GAIN_64 && gain == GAIN_128)
    k = 2;
  else if (Gain == GAIN_128 && gain == GAIN_64)
    k = 0.5;
  else
    return;

  Gain = gain;
  _d *= k;
  Scale *= k;
  Absolute_error *= k;
  Zero *= k;
  sensor.setGain(gain);
  sensor.getData();
}

//----------------------------------------------------------------------------------------------------------------------
void IRAM_ATTR recordISR()
{
  if (millis() - prev_press < DEBOUNCE_TIME)
    return;
  prev_press = millis();
  record = 1;
  interrupt++;
}

void IRAM_ATTR tareISR()
{
  if (millis() - prev_press < DEBOUNCE_TIME)
    return;
  prev_press = millis();
  tare = 1;
  interrupt++;
}

void IRAM_ATTR modeISR()
{
  if (millis() - prev_press < DEBOUNCE_TIME)
    return;
  prev_press = millis();
  mode = 1;
  interrupt++;
}

void IRAM_ATTR upISR()
{
  if (millis() - prev_press < DEBOUNCE_TIME)
    return;
  prev_press = millis();
  up = 1;
  interrupt++;
}

void IRAM_ATTR downISR()
{
  if (millis() - prev_press < DEBOUNCE_TIME)
    return;
  prev_press = millis();
  down = 1;
  interrupt++;
}