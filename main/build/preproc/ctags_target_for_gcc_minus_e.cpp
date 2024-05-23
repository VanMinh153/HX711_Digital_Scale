# 1 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
# 2 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino" 2
# 3 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino" 2
# 15 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
// #define SCALE 1030
# 29 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
HX711 sensor(6, 7, 25, 420);
LiquidCrystal_I2C lcd(0x27, 16, 2);

float Scale = 420;
int32_t Zero = 0;
uint8_t Mode = 0;
float w = 0;
float w_prev = 0;

int32_t sensor_error = 0;
unsigned long timer = millis();

float record[3];
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
int32_t getData(uint8_t K = 50, int32_t *sensor_error = 
# 68 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino" 3 4
                                                       __null
# 68 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
                                                           , uint8_t N = 5);
float getWeight(uint8_t K = 50, int32_t *sensor_error = 
# 69 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino" 3 4
                                                       __null
# 69 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
                                                           , uint8_t N = 5);
void sleep_();

//-----------------------------------------------------------------------------------------------
void setup()
{
  Serial.begin(57600);
  // Serial.println("Starting");
  pinMode(4, 0x2);
  pinMode(5, 0x2);
  pinMode(9, 0x2);
  pinMode(8, 0x2);
  pinMode(18, 0x2);
  attachInterrupt(18, recordInterrupt, 3);
  attachInterrupt(4, tareInterrupt, 3);
  attachInterrupt(5, modeInterrupt, 3);
  attachInterrupt(9, upInterrupt, 3);
  attachInterrupt(8, downInterrupt, 3);
  Wire.begin(2, 3);

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
  if (((w - w_prev)>0?(w - w_prev):-(w - w_prev)) < 0.2f)
  {
    if (millis() - timer > 4800)
    {
      if (w == 0)
      {
        sleep_();
      }
      else
      {
        lcd.noBacklight();
        delay_(600);
        lcd.backlight();
        delay_(600);
        lcd.noBacklight();
        delay_(600);
        lcd.backlight();
        delay_(3000);
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
  if (millis() - timer > 720 && ((w - record[q])>0?(w - record[q]):-(w - record[q])) > 0.2f && ((w)>0?(w):-(w)) > 0.2f)
  {
    q++;
    if (q == 3)
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
      if (k == 3 + 1)
        k = 0;

      _record = false;
      _interrupt = false;
      if (k != 0)
      {
        lcd.clear();
        lcd.backlight();
        lcd.setCursor(1, 0);
        lcd.print("Record Weight: ");
        lcd_(record[(q - k + 3 + 1) % 3]);
        if (delay_(3000) == false)
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
        if (sensor_error < (int)(Scale * (0.2f / 10)))
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
      Mode = (Mode == 0) ? 1 : 0;
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
      if (digitalRead(9) == 0x0)
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
      if (digitalRead(8) == 0x0)
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
  if (w < 0 && w > -0.2f)
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
  lcd.clear();
  lcd.noBacklight();
  uint8_t i = 0;
  uint8_t flag = 0;
  while (_interrupt == false)
  {
    float temp = getWeight(10);

    if (flag == 1)
      if (((temp)>0?(temp):-(temp)) > 0.2f)
      {
        _sleep = false;
        break;
      }
      else
        continue;

    if (((temp)>0?(temp):-(temp)) < 0.2f)
    {
      flag = 1;
      continue;
    }

    if (((w - temp)>0?(w - temp):-(w - temp)) > 0.2f)
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
  while (_interrupt == false && i > 0 && ((w - getWeight(10))>0?(w - getWeight(10)):-(w - getWeight(10))) < 0.2f)
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
    while (j >= 0 && ((arr[j] - avg)>0?(arr[j] - avg):-(arr[j] - avg)) > ((key - avg)>0?(key - avg):-(key - avg)))
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
    if (((temp)>0?(temp):-(temp)) < Scale * 200)
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
  w_worst = ((w[N - 1] - w_avg)>0?(w[N - 1] - w_avg):-(w[N - 1] - w_avg));
  while (k < K && w_worst > (int)(Scale * (0.2f / 2)))
  {
    temp = sensor.getData();
    if (((temp - w_avg)>0?(temp - w_avg):-(temp - w_avg)) < w_worst)
    {
      w_avg += (temp - w[N - 1]) / N;
      w[N - 1] = temp;
      sort_(w, N, w_avg);
      w_worst = ((w[N - 1] - w_avg)>0?(w[N - 1] - w_avg):-(w[N - 1] - w_avg));
    }
    k++;
  }

  if (sensor_error != 
# 372 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino" 3 4
                     __null
# 372 "C:\\Users\\Admin\\Desktop\\Documents\\GR1 Scale\\main\\main.ino"
                         )
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
  if (millis() - recordPrev < 200)
    return;
  recordPrev = millis();
  _record = true;
  _interrupt = true;
}

void IRAM_ATTR tareInterrupt()
{
  if (millis() - tarePrev < 200)
    return;
  tarePrev = millis();
  _tare = true;
  _interrupt = true;
}

void IRAM_ATTR modeInterrupt()
{
  if (millis() - modePrev < 200)
    return;
  modePrev = millis();
  _mode = true;
  _interrupt = true;
}

void IRAM_ATTR upInterrupt()
{
  if (millis() - upPrev < 200)
    return;
  upPrev = millis();
  _up = true;
  _interrupt = true;
}

void IRAM_ATTR downInterrupt()
{
  if (millis() - downPrev < 200)
    return;
  downPrev = millis();
  _down = true;
  _interrupt = true;
}
