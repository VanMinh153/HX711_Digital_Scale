# 1 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\main.ino"
# 2 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\main.ino" 2
// #include "main.h"
# 4 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\main.ino" 2
# 5 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\main.ino" 2


HX711 sensor(39, 38, CHAN_A_GAIN_128);




// #if defined(HW_LCD)
// LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, 16, 2);
// #endif

// #if defined(OLED)
// Adafruit_SSD1306 oled(128, 64, &Wire, -1);
// #endif

// hx711_gain_t Gain = CHAN_A_GAIN_128;
int Tare = 0;
float Scale = 420;
uint8_t Mode = 0;
uint16_t Absolute_error = (uint16_t)(Scale * 0.1f);
// //-------------------------------------------------------
volatile uint8_t tare = 0;
volatile uint8_t mode = 0;
volatile uint8_t up = 0;
volatile uint8_t down = 0;
volatile uint8_t record = 0;
volatile uint8_t interrupt = 0;
volatile unsigned long prev_press = millis();
uint8_t prev_interrupt = 0;
// //-------------------------------------------------------
int _data = 0;
float prev_data = 0;
float _weight = 0;
float record_weight[3];

unsigned long timer = millis();
// int prev_readData_val = 0;
// uint32_t sensor_error = 0;
uint8_t sleep_flag = 0;
uint8_t wake_up_flag = 0;
uint8_t interrupt_flag = 0;

void __attribute__((section(".iram1" "." "33"))) modeISR()
{
  if (millis() - prev_press < 200)
    return;
  prev_press = millis();
  // mode = 1;
  // interrupt++;
}

//----------------------------------------------------------------------------------------------------------------------

void setup()
{
  Serial.begin(57600);

  pinMode(15, 0x05);
  pinMode(18, 0x05);
  pinMode(16, 0x05);
  pinMode(17, 0x05);
  pinMode(21, 0x05);
  attachInterrupt(15, tareISR, 0x01);
  attachInterrupt(18, modeISR, 0x01);
  attachInterrupt(16, upISR, 0x01);
  attachInterrupt(17, downISR, 0x01);
  attachInterrupt(21, recordISR, 0x01);

  Wire.begin(33, 35);
  sensor.begin();

  lcd.init();
  delay(1000);
  getData_(true);
  getData_(true);
  getData_(true);

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
  if (sleep_flag = 1 || interrupt_flag == 1)
  {
    sleep_flag = 0;
    interrupt_flag = 0;
    lcd.setCursor(1, 0);
    lcd.print("Digital Scale");
  }
  _data = getData_(true);
  _weight = toWeight(_data);
  lcd_(_weight);

  // feature: Auto turn off the screen backlight
  // if the weighing result does not change by more than (ABSOLUTE_ERROR)kg in 3s
  if (wake_up_flag == 1)
    wake_up_flag = 0;

  if (abs(_data - prev_data) < 2 * Absolute_error)
  {
    while (millis() - timer > 3000)
    {
      if (_data == 0)
      {
        sleep_flag = 1;
        sleep_();
        break;
      }
      for (uint8_t i = 0; i < 2; i++)
      {
        lcd.noBacklight();
        if (waitForWeightChange(350) != 0)
        {
          lcd.backlight();
          break;
        }

        lcd.backlight();
        if (waitForWeightChange(350) != 0)
          break;
      }
      if (waitForWeightChange(350) != 0)
        break;
      sleep_flag = 1;
      sleep_();
      break;
    }
  }
  else
    timer = millis();

  // feature: Save the results of the last RECORD_NUM weightings
  uint8_t q = 0;
  if (millis() - timer > 600 && abs(_weight - record_weight[q]) > 0.1f && abs(_weight) > 0.1f)
  {
    q++;
    if (q == 3)
      q = 0;
    record_weight[q] = _weight;
    // Serial.println("Record: " + String(_weight));
  }
  prev_data = _data;

  //----------------------------------------------------------------------------------------------------------------------
  // Interrupt handling
  while (tare == 1 || mode == 1 || up == 1 || down == 1 || record == 1)
  {
    // Serial.print('*');
    uint8_t _tare = tare;
    uint8_t _mode = mode;
    uint8_t _up = up;
    uint8_t _down = down;
    uint8_t _record = record;
    prev_interrupt = interrupt;
    interrupt_flag = 1;
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
      Tare = getData_();
      lcd_(getWeight());
    }

    // feature: Change weight unit from kilogram to pound
    if (_mode == 1)
    {
      mode = 0;
      Mode = (Mode == 0) ? 1 : 0;
      lcd_(_weight);
    }
    //
    if (_up == 1 || _down == 1)
    {
      delay(200);
      lcd.setCursor(1, 0);
      lcd.print("Adjust Scale    ");
    }
    // feature: Adjust weighting result up
    float w;
    while (_up == 1 && up == 1)
    {
      Scale -= 0.5;
      w = _data / Scale;
      lcd_(w);

      if (digitalRead(16) == 0x0)
        Scale -= 2;
      else
        up = 0;
    }

    // feature: Adjust weighting result down
    while (_down == 1 && down == 1)
    {
      Scale += 0.5;
      w = _data / Scale;
      lcd_(w);

      if (digitalRead(17) == 0x0)
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
    uint8_t k = 0;
    while (_record == 1 && record == 1)
    {
      record = 0;
      k++;
      if (k == 3 + 1)
      {
        k = 0;
        break;
      }

      lcd_(record_weight[(q - k + 3 + 1) % 3]);
      if (waitOnInterrupt(2500, &record) == 1)
        prev_interrupt++;
    }
    //----------------------------------------------------------------------------------------------------------------------
    if (_up == 1 || _down == 1)
      Absolute_error = (uint32_t)(Scale * 0.1f);

    if (_record == 1)
      
# 252 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\main.ino" 3 4
     __null
# 252 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\main.ino"
         ;
    else
    {
      if (waitOnInterrupt(2000) == 1)
        continue;
    }

    if (sleep_flag == true)
      sleep_();
  }
  prev_interrupt = interrupt;

  delay(50);
}
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
