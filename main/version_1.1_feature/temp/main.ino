#include "SOICT_HX711.h"
// #include "main.h"
#include "config.h"
#include "utility.h"
#include "screen.h"

#if defined(HW_HX711)
HX711 sensor(DATA_PIN, CLK_PIN, CHAN_A_GAIN_128);
#elif defined(HW_HX711x4)
HX711List sensor(4, data_pin, clk_pin, CHAN_A_GAIN_128);
#endif

#if defined(HW_LCD)
LCD_I2C screen(LCD_I2C_ADDRESS, 16, 2);
#elif defined(HW_OLED)
OLED_SSD1306 screen(128, 64);
#endif

// hx711_gain_t Gain = CHAN_A_GAIN_128;
int Tare = 0;
float Scale = SCALE;
uint8_t Mode = KG_MODE;
uint16_t Absolute_error = (uint16_t)(Scale * ABSOLUTE_ERROR);
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
float record_weight[RECORD_NUM];

unsigned long timer = millis();
// int prev_readData_val = 0;
// uint32_t sensor_error = 0;
uint8_t sleep_flag = 0;
uint8_t wake_up_flag = 0;
uint8_t interrupt_flag = 0;

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
  sensor.begin();

  screen.begin();
  delay(1000);
  getData_();
  getData_();

  screen.printTitle(MAIN_TITLE);
  screen.printContent("SOICT-K66");
  delay(2000);
  screen.clear();
  screen.printTitle(MAIN_TITLE);
}

//----------------------------------------------------------------------------------------------------------------------
void loop()
{
  _data = getData_();
  _weight = toWeight(_data);

#if defined(HW_LCD)
  if (sleep_flag == 1 || interrupt_flag == 1)
    screen.printTitle(MAIN_TITLE);

  if (_data != prev_data)
    screen.printWeight(_weight);
#endif

#if defined(HW_OLED)
  if (_data != prev_data)
  {
    screen.printTitle(MAIN_TITLE);
    screen.printWeight(_weight);
  }
#endif

  sleep_flag = 0;
  interrupt_flag = 0;

  // feature: Auto turn off the screen backlight
  // if the weighing result does not change by more than (ABSOLUTE_ERROR)kg in 3s
  if (wake_up_flag == 1)
    wake_up_flag = 0;

  if (abs(_data - prev_data) < 2 * Absolute_error)
  {
    while (millis() - timer > AUTO_SLEEP_TIME)
    {
      if (_data == 0)
      {
        sleep_flag = 1;
        sleep_();
        break;
      }
      for (int i = 0; i < 2; i++)
      {
        screen.noBacklight();
        if (waitForWeightChange(FLICKER_DELAY) != 0)
        {
          screen.backlight();
          break;
        }

        screen.backlight();
        if (waitForWeightChange(FLICKER_DELAY) != 0)
          break;
      }
      if (waitForWeightChange(FLICKER_DELAY) != 0)
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
  if (millis() - timer > RECORD_TIME && abs(_weight - record_weight[q]) > ABSOLUTE_ERROR && abs(_weight) > ABSOLUTE_ERROR)
  {
    q++;
    if (q == RECORD_NUM)
      q = 0;
    record_weight[q] = _weight;
    // Serial.println("Record: " + String(_weight));
  }

  if (_data != prev_data)
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

    // feature: Adjust the scale to 0 kg
    if (_tare == 1)
    {
      tare = 0;
      screen.printTitle("Digital Scale");
      screen.printContent("Taring...");
      Tare = getData_();
      screen.printTitle("Digital Scale");
      screen.printWeight(getWeight());
    }

    // feature: Change weight unit from kilogram to pound
    if (_mode == 1)
    {
      mode = 0;
      screen.printTitle("Digital Scale");
      Mode = (Mode == KG_MODE) ? LB_MODE : KG_MODE;
      screen.printWeight(_weight);
    }
    //
    if (_up == 1 || _down == 1)
    {
      delay(DEBOUNCE_TIME);
      screen.printTitle("Adjust Scale    ");
    }
    // feature: Adjust weighting result up
    float w;
    while (_up == 1 && up == 1)
    {
      Scale -= 0.5;
      w = _data / Scale;
      screen.printWeight(w);

      if (digitalRead(UP) == LOW)
        Scale -= 2;
      else
        up = 0;
    }

    // feature: Adjust weighting result down
    while (_down == 1 && down == 1)
    {
      Scale += 0.5;
      w = _data / Scale;
      screen.printWeight(w);

      if (digitalRead(DOWN) == LOW)
        Scale += 2;
      else
        down = 0;
    }

    // feature: View the results of the last weightings
    if (_record == 1)
      screen.printTitle("Record Weight:  ");

    uint8_t k = 0;
    while (_record == 1 && record == 1)
    {
      record = 0;
      k++;
      if (k == RECORD_NUM + 1)
      {
        k = 0;
        break;
      }

      screen.printWeight(record_weight[(q - k + RECORD_NUM + 1) % RECORD_NUM]);
      if (waitOnInterrupt(SHOW_RECORD_TIME, &record) == 1)
        prev_interrupt++;
    }
    //----------------------------------------------------------------------------------------------------------------------
    if (_up == 1 || _down == 1)
      Absolute_error = (uint32_t)(Scale * ABSOLUTE_ERROR);

    if (_record == 1)
      NULL;
    else
    {
      if (waitOnInterrupt(SHOW_ISR_TIME) == 1)
        continue;
    }

    if (sleep_flag == true)
      sleep_();
  }
  prev_interrupt = interrupt;

  delay(MAIN_DELAY);
}