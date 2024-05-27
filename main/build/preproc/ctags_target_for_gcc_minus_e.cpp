# 1 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
# 2 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino" 2
# 3 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino" 2
# 15 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
// #define SCALE 1030
# 33 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
HX711 sensor(6, 7, 25, 420);
LiquidCrystal_I2C lcd(0x27, 16, 2);

byte Gain = 25;
float Scale = 420;
int32_t Zero = 0;
byte Mode = 0;
uint16_t Absolute_error = (uint16_t)(Scale * 0.1f);
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
float record_w[3];
byte q = 0;
byte k = 0;
byte _sleep = 0;
byte _detect = 0;
byte _isr = 0;

//----------------------------------------------------------------------------------------------------------------------
void __attribute__((section(".iram1" "." "27"))) recordISR();
void __attribute__((section(".iram1" "." "28"))) tareISR();
void __attribute__((section(".iram1" "." "29"))) modeISR();
void __attribute__((section(".iram1" "." "30"))) upISR();
void __attribute__((section(".iram1" "." "31"))) downISR();
void lcd_(float _w);
void sort_(int32_t arr[], byte n, int32_t avg);
int32_t getData_Avg(byte K = 10, uint32_t *error = &sensor_error);
int32_t getData_(uint16_t delay_time = 0);
float getWeight();
float toWeight(int32_t data);
byte sleep_(byte sensitivity = 2);
byte delay_W(uint16_t timeout, uint16_t time2listen = 50, uint16_t error = Absolute_error);
byte delay_I(uint32_t timeout, volatile byte *isrCtl = 
# 75 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino" 3 4
                                                      __null
# 75 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
                                                          );
void setGain(byte gain);

//----------------------------------------------------------------------------------------------------------------------
void setup()
{
  Serial.begin(57600);
  pinMode(4, 0x05);
  pinMode(5, 0x05);
  pinMode(9, 0x05);
  pinMode(8, 0x05);
  pinMode(18, 0x05);
  attachInterrupt(4, tareISR, 0x02);
  attachInterrupt(5, modeISR, 0x02);
  attachInterrupt(9, upISR, 0x02);
  attachInterrupt(8, downISR, 0x02);
  attachInterrupt(18, recordISR, 0x02);
  Wire.begin(2, 3);

  sensor.init();

  lcd.init();
  lcd.backlight();
  lcd.noCursor();
  lcd.setCursor(1, 0);
  lcd.print("Digital Scale");
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
    lcd.print("Digital Scale");
  }
  _d = getData_(500);
  _w = toWeight(_d);
  lcd_(_w);

  // feature: Auto turn off the screen backlight
  // if the weighing result does not change by more than (ABSOLUTE_ERROR)kg in 3s
  if (_detect == true)
    _detect = false;
  if (abs(_d - prev_d) < 2 * Absolute_error)
  {
    while (millis() - timer > 3000)
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
        if (delay_W(350) != 0)
        {
          lcd.backlight();
          break;
        }

        lcd.backlight();
        if (delay_W(350) != 0)
          break;
      }
      if (delay_W(350) != 0)
        break;
      _sleep = true;
      sleep_();
      break;
    }
  }
  else
    timer = millis();

  // feature: Save the results of the last RECORD_NUM weightings
  if (millis() - timer > 600 && abs(_w - record_w[q]) > 0.1f && abs(_w) > 0.1f)
  {
    q++;
    if (q == 3)
      q = 0;
    record_w[q] = _w;
    Serial.println("\r\nRecord: " + String(_w));
  }
  prev_d = _d;

  //----------------------------------------------------------------------------------------------------------------------
  // Interrupt handling
  while (tare == 1 || mode == 1 || up == 1 || down == 1 || record == 1)
  {
    Serial.print('*');
    prev_interrupt = interrupt;
    _isr = 1;
    byte _tare = tare;
    byte _mode = mode;
    byte _up = up;
    byte _down = down;
    byte _record = record;

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
      for (byte i = 0; i < 5; i++)
      {
        Zero = getData_Avg();
        if (sensor_error < (int)(Scale * (0.1f / 5)))
          break;
      }
      sensor.setZero(Zero);
      lcd_(getWeight());
    }

    // feature: Change weight unit from kilogram to pound
    if (_mode == 1)
    {
      mode = 0;
      Mode = (Mode == 0) ? 1 : 0;
      lcd_(_w);
    }
    //
    if (_up == 1 || _down == 1)
    {
      delay(100);
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

      if (digitalRead(9) == 0x0)
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

      if (digitalRead(8) == 0x0)
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
      if (k == 3 + 1)
      {
        k = 0;
        break;
      }

      lcd_(record_w[(q - k + 3 + 1) % 3]);
      if (delay_I(2500, &record) == 1)
        prev_interrupt++;
    }
    //----------------------------------------------------------------------------------------------------------------------
    if (_up == 1 || _down == 1)
    {
      Absolute_error = (uint32_t)(Scale * 0.1f);
      sensor.setScale(Scale);
    }

    if (_record == 1)
      
# 274 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino" 3 4
     __null
# 274 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
         ;
    else
    {
      if (delay_I(2000 - 300) == 1)
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
  if (w < 0 && w > -0.1f)
    return;
  lcd.setCursor(1, 1);
  if (Mode == 1)
  {
    lcd.print(w * 2.204623f);
    lcd.print(" lb    ");
  }
  else
  {
    lcd.print(w);
    lcd.print(" kg    ");
  }
}

/**

 * @brief Delay function with the ability to _detect the interruption signal and weight changes

 *

 * @param timeout     The maximum time to wait. if timeout = 0xffff, the function will wait indefinitely

 * @param time2listen The time to listen weight changes

 * @param error       Default is absolute error of the scale

 * @return byte       0: timeout, 1: weight changes, 2: interrupt signal

 */
# 316 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
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
      d = getData_(1000);
    }
    else if (TIME_END > t + time2listen)
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
      if (abs(d) > error)
      {
        _detect = 1;
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

/**

 * @param sensitivity   Absolute error of the scale will be set to (sensitivity * Absolute_error)

 */
# 382 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
byte sleep_(byte sensitivity)
{
  lcd.clear();
  lcd.noBacklight();
  byte retval = 0;
  setGain(27);

  retval = delay_W(0xffff, 300, sensitivity * Absolute_error);
  // Serial.println("\r\nAwake: " + String(retval));
  setGain(25);
  lcd.backlight();
  return retval;
}

byte delay_I(uint32_t timeout, volatile byte *isrCtl)
{
  unsigned long t = millis() + timeout;
  if (isrCtl != 
# 399 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino" 3 4
               __null
# 399 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
                   )
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

int32_t getData_Avg(byte K, uint32_t *error)
{
  const byte N = 5;
  int32_t d[N];
  int32_t d_avg = 0;
  int32_t d_worst = 0;
  int32_t d_temp = 0;
  byte k = 0;

  for (byte i = 0; i < N; i++)
  {
    d_temp = sensor.getData_H(Gain);
    if (abs(d_temp) < Scale * 200)
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
    d_temp = sensor.getData_H(Gain);
    if (abs(d_temp - d_avg) < d_worst)
    {
      d_avg += (d_temp - d[N - 1]) / N;
      d[N - 1] = d_temp;
      sort_(d, N, d_avg);
      d_worst = abs(d[N - 1] - d_avg);
    }
    k++;
  }

  *error = d_worst;
  return d_avg;
}
//
int32_t getData_(uint16_t delay_time)
{
  byte i = 0;
  int32_t d = getData_Avg();
  if (sensor_error > Absolute_error)
    d = getData_Avg();

  if (delay_time > 0 && sensor_error > Absolute_error)
  {
    for (i = 1; i < 3; i++)
    {
      delay(delay_time / 2);
      d = getData_Avg();
      if (sensor_error < Absolute_error)
        break;
    }
  }

  if (sensor_error > Absolute_error)
    Serial.println("\r\nE-" + String(i) + '-' + String(toWeight(d)) + '-' + String(sensor_error));
  else
    Serial.print('_');
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
  if (Gain == 27 && gain == 25)
    k = 2;
  else if (Gain == 25 && gain == 27)
    k = 0.5;
  else
    return;

  Gain = gain;
  _d *= k;
  Scale *= k;
  Absolute_error *= k;
  sensor.setGain(gain);
}

//----------------------------------------------------------------------------------------------------------------------
void __attribute__((section(".iram1" "." "32"))) recordISR()
{
  if (millis() - prev_press < 200)
    return;
  prev_press = millis();
  record = 1;
  interrupt++;
}

void __attribute__((section(".iram1" "." "33"))) tareISR()
{
  if (millis() - prev_press < 200)
    return;
  prev_press = millis();
  tare = 1;
  interrupt++;
}

void __attribute__((section(".iram1" "." "34"))) modeISR()
{
  if (millis() - prev_press < 200)
    return;
  prev_press = millis();
  mode = 1;
  interrupt++;
}

void __attribute__((section(".iram1" "." "35"))) upISR()
{
  if (millis() - prev_press < 200)
    return;
  prev_press = millis();
  up = 1;
  interrupt++;
}

void __attribute__((section(".iram1" "." "36"))) downISR()
{
  if (millis() - prev_press < 200)
    return;
  prev_press = millis();
  down = 1;
  interrupt++;
}
