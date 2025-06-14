#include "utility.h"

hx711_gain_t Gain = CHAN_A_GAIN_128;
int getData_Avg_return = 0;
uint32_t sensor_error = 0;

#if defined(HW_DHT)
float readTemperature()
{
  float t = dht.readTemperature() + 24;
  unsigned long timer = millis();
  while (isnan(t) && millis() - timer < 5)
    t = dht.readTemperature();
  return t;
}
#elif defined(HW_NTC)
float readTemperature()
{
  int data = analogRead(PIN_NTC);
  float temp = 1 / (log(1 / (1023. / data - 1)) / BETA + 1 / 298.15) - 273.15;
  return temp;
}
#elif defined(HW_LM35)
float readTemperature()
{
  int data = analogRead(PIN_LM35);
  float temp = data * (ADC_VREF / ADC_RESOLUTION) / 10;
  return temp;
}
#endif

#if defined(HW_RFID)
String readRFID()
{
  String id = "";
  if (rfid.PICC_IsNewCardPresent()) {
    Serial.println("RFID card detected!");
    if (rfid.PICC_ReadCardSerial()) {
      Serial.println("RFID serial read successfully.");
      for (byte i = 0; i < rfid.uid.size; i++)
      {
        id.concat(String(rfid.uid.uidByte[i] < 0x10 ? "0" : ""));
        id.concat(String(rfid.uid.uidByte[i], HEX));
      }
      Serial.print("Raw RFID UID: ");
      Serial.println(id);
    } else {
      Serial.println("Failed to read RFID serial.");
    }
  }
  rfid.PICC_HaltA();

  return id;
}
#endif
//
/**
 * @param sensitivity   Absolute error of the scale will be set to (sensitivity * Absolute_error)
 */
uint8_t sleep_(uint8_t sensitivity)
{
  Serial.print("Sleeping...");
  // screen.clear();
  // screen.noBacklight();
  // uint8_t retval = 0;
  // setGain(CHAN_A_GAIN_64);

  // retval = waitForWeightChange(0xffff, 500, sensitivity * Absolute_error);
  // if (retval == 1)
  //   Serial.println(" > Awake: Detect Weight Changes");
  // else if (retval == 2)
  //   Serial.println(" > Awake: Detect Interrupt");
  // else if (retval == 3)
  //   Serial.println(" > Awake: Detect Weight Changes from Zero");
  // else if (retval == 4)
  //   Serial.println(" > Awake: detect_new_weight_flag have been set before!!!");

  // setGain(CHAN_A_GAIN_128);
  // screen.backlight();
  // return retval;
  return 1;
}

/**
 * @brief Delay function with the ability to detect the interruption signal and weight changes
 *
 * @param timeout     The maximum time to wait. if timeout = 0xffff, the function will wait indefinitely
 * @param time2listen The time to listen weight changes
 * @param error       Default is absolute error of the scale
 * @return uint8_t    0: timeout, 1: weight changes, 2: interrupt signal
 *                    3: weight changes from zero
 *                    4: detect_new_weight_flag have been set before!!!
 */
uint8_t waitForWeightChange(uint16_t timeout, uint16_t time2listen, uint16_t error)
{
  if (detect_new_weight_flag == 1)
    return 3;

  unsigned long t = millis();
  unsigned long TIME_END = t + timeout;
  uint8_t tare_flag = 0;
  int d = 0;

  while (interrupt == prev_interrupt)
  {
    if (timeout == 0xffff)
    {
      delay(time2listen);
      d = getData_();
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

    if (tare_flag == 1)
      if (abs(d - Tare) > 3 * error)
      {
        detect_new_weight_flag = 1;
        break;
      }
      else
        continue;

    if (abs(d - Tare) < error)
    {
      tare_flag = 1;
      continue;
    }

    if (abs(_data - d) > error)
    {
      if (abs(d - getData_()) < 2 * error)
      {
        detect_new_weight_flag = 1;
        break;
      }
    }
  }

  if (detect_new_weight_flag == 1)
  {
    sleep_timer = millis();
    return tare_flag == 0 ? 1 : 3;
  }
  return (t == TIME_END) ? 0 : 2;
}

uint8_t waitOnInterrupt(uint32_t timeout, volatile uint8_t *isrCtl)
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

void sort_(int arr[], uint8_t n, int avg)
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
//----------------------------------------------------------------------------------------------------------------------
#define GET_DATA_FAIL 505
#if defined(HW_HX711)
int getData_Avg(HX711 adc)
{
  const uint8_t N = 3;
  const uint8_t K = 3;
  int d[N];
  int d_avg = 0;
  uint32_t d_worst = 0;
  int d_temp = 0;
  uint8_t count = 0;

  for (uint8_t i = 0; i < 3 * N; i++)
  {
    d_temp = adc.readData();

    if (d_temp == HX711_FAIL)
      continue;
    if (d_temp == getData_Avg_return)
      return getData_Avg_return;
    if (abs(d_temp - Tare) < (int)(Scale * MAX_LOAD))
    {
      d[count] = d_temp;
      d_avg += d_temp;
      count++;
      if (count == N)
        break;
    }
  }

  // if (count < N)
  //   return GET_DATA_FAIL;
  d_avg /= N;
  sort_(d, N, d_avg);
  d_worst = abs(d[N - 1] - d_avg);

  count = 0;
  while (count < K && d_worst > Absolute_error)
  {
    d_temp = adc.readData();
    if (d_temp == HX711_FAIL)
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
  getData_Avg_return = d_avg;
  // Serial.print('_');
  return d_avg;
}
#elif defined(HW_HX711x4)
int getData_Avg(HX711List adc)
{
  const uint8_t N = 3;
  const uint8_t K = 5;
  int d[N];
  int d_avg = 0;
  uint32_t d_worst = 0;
  int d_temp = 0;
  uint8_t count = 0;

  for (uint8_t i = 0; i < 2 * N; i++)
  {
    d_temp = adc.readData();

    // FIXBUG
    // if (adc.errRead != 0)
    //   Serial.print("`ERR_READ`=" + String(adc.errRead) + " | ");
    // if (abs(d_temp) > (int) (Scale * MAX_LOAD))
    //   Serial.print("`ERR_MAXLOAD`=" + String(abs(d_temp)) + " | ");

    if (adc.errRead != 0)
      continue;

    if (d_temp == getData_Avg_return)
      return getData_Avg_return;

    if (abs(d_temp) < (int)(Scale * MAX_LOAD))
    {
      d[count] = d_temp;
      d_avg += d_temp;
      count++;
      if (count == N)
        break;
    }
  }

  if (count < N)
    return GET_DATA_FAIL;
  d_avg /= N;
  sort_(d, N, d_avg);
  d_worst = abs(d[N - 1] - d_avg);

  count = 0;
  while (count < K && d_worst > Absolute_error)
  {
    d_temp = adc.readData();
    if (adc.errRead != 0)
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
  getData_Avg_return = d_avg;
  // Serial.print('_');
  return d_avg;
}
#endif

int getData_(uint8_t allow_delay)
{
  int d = getData_Avg(sensor);
  if (d != GET_DATA_FAIL && sensor_error < 10*Absolute_error)
    return d;

  if (allow_delay == 1)
    d = getData_Avg(sensor);

  if (allow_delay == 2)
  {
    delay(105);
    d = getData_Avg(sensor);
  }

  if (d == GET_DATA_FAIL)
  {
    // FIXBUG
    Serial.println("\r\ngetData_Avg()_Failed - " + String(getData_Avg_return) + " | ");
    // Serial.println("Error: Failed to get data from HX711");
    return _data;
  }

  if (sensor_error > Absolute_error)
    Serial.println("Warning: Low accurary value: " + String(toWeight(d)) + " +-" + String(sensor_error / Scale));

  return -d;
}

float toWeight(int data)
{
  return (long)(data - Tare) / Scale;
}

float getWeight()
{
  return (long)(getData_() - Tare) / Scale;
}

void setGain(hx711_gain_t gain)
{
  float k;
  if (Gain == CHAN_A_GAIN_64 && gain == CHAN_A_GAIN_128)
    k = 2;
  else if (Gain == CHAN_A_GAIN_128 && gain == CHAN_A_GAIN_64)
    k = 0.5;
  else
    return;

  Gain = gain;
  _data *= k;
  Scale *= k;
  Absolute_error *= k;
  Tare *= k;
  sensor.setGain(gain);
}

//----------------------------------------------------------------------------------------------------------------------
void IRAM_ATTR recordISR()
{
  unsigned long t = millis();
  if (t - prev_press < DEBOUNCE_TIME)
    return;
  prev_press = t;
  record = 1;
  interrupt++;
}

void IRAM_ATTR tareISR()
{
  unsigned long t = millis();
  if (t - prev_press < DEBOUNCE_TIME)
    return;
  prev_press = t;
  tare = 1;
  interrupt++;
}

void IRAM_ATTR modeISR()
{
  unsigned long t = millis();
  if (t - prev_press < DEBOUNCE_TIME)
    return;
  prev_press = t;
  mode = 1;
  interrupt++;
}

void IRAM_ATTR upISR()
{
  unsigned long t = millis();
  if (t - prev_press < DEBOUNCE_TIME)
    return;
  prev_press = t;
  up = 1;
  interrupt++;
}

void IRAM_ATTR downISR()
{
  unsigned long t = millis();
  if (t - prev_press < DEBOUNCE_TIME)
    return;
  prev_press = t;
  down = 1;
  interrupt++;
}