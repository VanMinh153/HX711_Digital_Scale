# 1 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
# 2 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino" 2
# 3 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino" 2
# 15 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
// #define SCALE 1030
# 31 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
HX711 sensor(6, 7, 25, 420);
LiquidCrystal_I2C lcd(0x27, 16, 2);

float Scale = 420;
int32_t Zero = 0;
byte Mode = 0;
uint32_t Absolute_error = (uint32_t)(Scale * 0.1f);
int32_t _d = 0;
float _w = 0;
float prev_w = 0;
int32_t sensor_error = 0;
unsigned long timer = millis();
float record_w[3];
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
void __attribute__((section(".iram1" "." "27"))) recordInterrupt();
void __attribute__((section(".iram1" "." "28"))) tareInterrupt();
void __attribute__((section(".iram1" "." "29"))) modeInterrupt();
void __attribute__((section(".iram1" "." "30"))) upInterrupt();
void __attribute__((section(".iram1" "." "31"))) downInterrupt();
void lcd_(float _w);
void sort_(int32_t arr[], byte n, int32_t avg);
int32_t getData(byte K = 50, int32_t *sensor_error = 
# 68 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino" 3 4
                                                    __null
# 68 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
                                                        , byte N = 5);
float getWeight(byte K = 50, int32_t *sensor_error = 
# 69 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino" 3 4
                                                    __null
# 69 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
                                                        , byte N = 5);
float toWeight(int32_t data);
void sleep_();
byte delay_(uint32_t timeout);
byte delay2_(uint32_t timeout);

//----------------------------------------------------------------------------------------------------------------------
void setup()
{
  Serial.begin(57600);
  pinMode(4, 0x05);
  pinMode(5, 0x05);
  pinMode(9, 0x05);
  pinMode(8, 0x05);
  pinMode(18, 0x05);
  attachInterrupt(4, tareInterrupt, 0x02);
  attachInterrupt(5, modeInterrupt, 0x02);
  attachInterrupt(9, upInterrupt, 0x02);
  attachInterrupt(8, downInterrupt, 0x02);
  attachInterrupt(18, recordInterrupt, 0x02);
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
  byte _interrupt = 0;
   if (_sleep = true || _interrupt == 1)
  {
    _sleep = false;
    lcd.setCursor(1, 0);
    lcd.print("Digital Scale");
  }
  _d = getData();
  _w = toWeight(_d);
  lcd_(_w);

  // feature: Auto turn off the screen backlight
  // if the weighing result does not change by more than (ABSOLUTE_ERROR)kg in 3s
  if (detect == true)
    detect = false;
  if (abs(_w - prev_w) < 0.1f)
  {
    while (millis() - timer > 3600)
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
        if (delay2_(540) != 0)
        {
          lcd.backlight();
          break;
        }

        lcd.backlight();
        if (delay2_(540) != 0)
          break;
      }
      if (delay2_(540) != 0)
        break;
      _sleep = true;
      sleep_();
      break;
    }
  }
  else
    timer = millis();

  // feature: Save the results of the last RECORD_NUM weightings
  if (millis() - timer > 720 && abs(_w - record_w[q]) > 0.1f && abs(_w) > 0.1f)
  {
    q++;
    if (q == 3)
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
        if (sensor_error < (int)(Scale * (0.1f / 5)))
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
      Mode = (Mode == 0) ? 1 : 0;
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

      if (digitalRead(9) == 0x0)
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

      if (digitalRead(8) == 0x0)
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
      if (k == 3 + 1)
      {
        k = 0;
        break;
      }

      lcd_(record_w[(q - k + 3 + 1) % 3]);
      if (delay_(2400) == false)
        continue;
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
      if (delay_(1800) == 1)
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
    if (abs(d2) < Scale * 200)
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

  if (sensor_error != 
# 460 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino" 3 4
                     __null
# 460 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
                         )
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
void __attribute__((section(".iram1" "." "32"))) recordInterrupt()
{
  if (millis() - prev_record < 200)
    return;
  prev_record = millis();
  record = 1;
  interrupt++;
}

void __attribute__((section(".iram1" "." "33"))) tareInterrupt()
{
  if (millis() - prev_tare < 200)
    return;
  prev_tare = millis();
  tare = 1;
  interrupt++;
}

void __attribute__((section(".iram1" "." "34"))) modeInterrupt()
{
  if (millis() - prev_mode < 200)
    return;
  prev_mode = millis();
  mode = 1;
  interrupt++;
}

void __attribute__((section(".iram1" "." "35"))) upInterrupt()
{
  if (millis() - prev_up < 200)
    return;
  prev_up = millis();
  up = 1;
  interrupt++;
}

void __attribute__((section(".iram1" "." "36"))) downInterrupt()
{
  if (millis() - prev_down < 200)
    return;
  prev_down = millis();
  down = 1;
  interrupt++;
}
