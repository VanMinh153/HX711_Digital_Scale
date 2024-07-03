# 1 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\main.ino"
# 2 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\main.ino" 2
// #include "main.h"
# 4 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\main.ino" 2
# 5 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\main.ino" 2
# 6 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\main.ino" 2




HX711List sensor(4, DATA_PIN, CLOCK_PIN, CHAN_A_GAIN_128);



LCD_I2C screen(0x27, 16, 2);




// hx711_gain_t Gain = CHAN_A_GAIN_128;
int Tare = 0;
float Scale = 420;
uint8_t Mode = 0;
uint16_t Absolute_error = (uint16_t)(Scale * (0.1f * 4));
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
uint8_t record_weight_idx = 0;

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

  pinMode(13, 0x05);
  pinMode(14, 0x05);
  pinMode(26, 0x05);
  pinMode(25, 0x05);
  pinMode(12, 0x05);
  attachInterrupt(13, tareISR, 0x01);
  attachInterrupt(14, modeISR, 0x01);
  attachInterrupt(26, upISR, 0x01);
  attachInterrupt(25, downISR, 0x01);
  attachInterrupt(12, recordISR, 0x01);

  Wire.begin(21, 22);
  sensor.begin();
  sensor.readData();
  screen.begin();
  delay(1000);

  screen.printTitle("Digital Scale");
  screen.printContent("SOICT-K66");
  delay(2000);
  screen.clear();
  screen.printTitle("Digital Scale");
}

//----------------------------------------------------------------------------------------------------------------------
void loop()
{

  sensor.DataUnitMax = 55 * Scale;


  _data = getData_();
  _weight = toWeight(_data);


  if (sleep_flag == 1 || interrupt_flag == 1)
    screen.printTitle("Digital Scale");

  if (_data != prev_data)
    screen.printWeight(_weight);
# 102 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\main.ino"
  sleep_flag = 0;
  interrupt_flag = 0;

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
      for (int i = 0; i < 2; i++)
      {
        screen.noBacklight();
        if (waitForWeightChange(350) != 0)
        {
          screen.backlight();
          break;
        }

        screen.backlight();
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
  if (millis() - timer > 600 && abs(_weight - record_weight[record_weight_idx]) > (0.1f * 4) && abs(_weight) > (0.1f * 4))
  {
    record_weight_idx++;
    if (record_weight_idx == 3)
      record_weight_idx = 0;
    record_weight[record_weight_idx] = _weight;
    Serial.println("Record: " + String(_weight));
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
      delay(1000);
      Tare = getData_();
      screen.printTitle("Digital Scale");
      screen.printWeight(getWeight());
    }

    // feature: Change weight unit from kilogram to pound
    if (_mode == 1)
    {
      mode = 0;
      screen.printTitle("Digital Scale");
      Mode = (Mode == 0) ? 1 : 0;
      screen.printWeight(_weight);
    }
    //
    if (_up == 1 || _down == 1)
    {
      delay(200);
      screen.printTitle("Adjust Scale    ");
    }
    // feature: Adjust weighting result up
    float w;
    while (_up == 1 && up == 1)
    {
      Scale -= 0.5;
      w = _data / Scale;
      screen.printWeight(w);

      if (digitalRead(26) == 0x0)
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

      if (digitalRead(25) == 0x0)
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
      if (k == 3 + 1)
      {
        k = 0;
        break;
      }

      screen.printWeight(record_weight[(record_weight_idx - k + 3 + 1) % 3]);
      if (waitOnInterrupt(2500, &record) == 1)
        prev_interrupt++;
    }
    //----------------------------------------------------------------------------------------------------------------------
    if (_up == 1 || _down == 1)
      Absolute_error = (uint32_t)(Scale * (0.1f * 4));

    if (_record == 1)
      
# 246 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\main.ino" 3 4
     __null
# 246 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\main.ino"
         ;
    else
    {
      if (waitOnInterrupt(2000) == 1)
        continue;
    }

    if (sleep_flag == 1)
      sleep_();
  }
  prev_interrupt = interrupt;

  delay(5);
}
