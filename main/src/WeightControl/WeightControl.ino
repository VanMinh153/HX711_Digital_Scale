#include "SOICT_HX711.h"
#include "config.h"
#include "gg_sheets.h"
#include "main.h"
#include "screen.h"
#include "utility.h"
#include <HTTPClient.h>
#include <WiFi.h>

#if defined(HW_HX711)
HX711 sensor(DATA_PIN, CLOCK_PIN, CHAN_A_GAIN_128);
#elif defined(HW_HX711x4)
HX711List sensor(4, DATA_PIN, CLOCK_PIN, CHAN_A_GAIN_128);
#endif

#if defined(HW_LCD) && defined(HW_OLED)
TEST_Screen screen;
#elif defined(HW_LCD)
LCD_I2C screen(LCD_I2C_ADDRESS, 16, 2);
#elif defined(HW_OLED)
OLED_SSD1306 screen(128, 64);
#endif

#if defined(HW_DHT)
DHT dht(PIN_DHT, DHT11);
#endif

#if defined(HW_RFID)
MFRC522 rfid(PIN_SS, PIN_RST);
#endif

// hx711_gain_t Gain = CHAN_A_GAIN_128;
int Tare = 0;
float Scale = SCALE;
uint8_t Mode = MODE_VN;
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
uint8_t record_weight_idx = 0;

String record_id[RECORD_NUM];
unsigned long rfid_timer = millis();
String _id = "";
String prev_id = "";

unsigned long sleep_timer = millis();
// int prev_readData_val = 0;
// uint32_t sensor_error = 0;
uint8_t sleep_flag = 0;
uint8_t detect_new_weight_flag = 0;
uint8_t interrupt_flag = 0;
String title = MAIN_TITLE;

Student students[MAX_STUDENTS];
int studentCount = 0;
const String Web_App_URL = "https://script.google.com/macros/s/"
                           "AKfycbyzcyRQRKRckv6g1rGAd5GbPtAK-IJqVBevDz7CeM1me-"
                           "j8HsB5l6RdxnltrxlQ6zxuzw/exec";

const char *ssid = "AaBb";
const char *password = "22446688";

//----------------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Welcome to Digital Scale!");
  Serial.println();
  Serial.println("-------------");
  Serial.println("WIFI mode : STA");
  Serial.println("-------------");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nWiFi connected!");
  Serial.println(
      "[setup] WiFi connected, start reading students from Google Sheets...");

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

#if defined(HW_NTC)
  analogReadResolution(10);
  pinMode(PIN_NTC, INPUT);
#endif

#if defined(HW_LM35)
  analogReadResolution(10);
  pinMode(PIN_LM35, INPUT);
#endif

#if defined(HW_DHT)
  dht.begin();
#endif

#if defined(HW_RFID)
  SPI.begin();
  rfid.PCD_Init();
#endif

  Wire.begin(PIN_SDA, PIN_SCL);
  sensor.begin();
  screen.begin();
  delay(1000);

  sensor.readData();
  Tare = sensor.readData();
  screen.printTitle(MAIN_TITLE);
  screen.printContent("SOICT-K66");
  delay(2000);
  screen.clear();
  screen.printTitle(MAIN_TITLE);

  // Đọc danh sách học sinh từ Google Sheet
  if (!gg_read_students(students, studentCount, Web_App_URL)) {
    Serial.println("[setup] Can't read data from google sheet!");
  } else {
    Serial.print("[setup] Read ");
    Serial.print(studentCount);
    Serial.println(" students from Google Sheets.");
  }
}

//----------------------------------------------------------------------------------------------------------------------
void loop() {
#if defined(HW_HX711x4)
  sensor.DataUnitMax = UNIT_MAX_LOAD * Scale;
#endif

  _data = -getData_();
  _weight = toWeight(_data);

  title = MAIN_TITLE;

#if defined(HW_RFID)
  _id = readRFID();
  if (_id == "" && prev_id != "" && millis() - rfid_timer < DELAY_RFID_TIME)
    _id = prev_id;
  else {
    prev_id = _id;
    rfid_timer = millis();
  }

  if (_id != "")
    title = _id;
  else
    title = MAIN_TITLE;

  // Sau khi cân xong và có UID RFID, gửi dữ liệu lên Google Sheet
  if (_id != "") {
    char uidChar[_id.length() + 1];
    _id.toCharArray(uidChar, _id.length() + 1);
    char *studentName = gg_getStudentNameById(uidChar, students, studentCount);
    String nameStr = studentName ? String(studentName) : String("Unknown");
    String weight = String(record_weight[record_weight_idx], 2);
    Serial.println();
    Serial.println("-------------");
    Serial.println("Send data to Google Spreadsheet...");
    Serial.print("UID: ");
    Serial.println(_id);
    Serial.print("Name: ");
    Serial.println(nameStr);
    Serial.print("Weight: ");
    Serial.println(weight);
    gg_send_weight_result(_id, nameStr, weight, Web_App_URL);
    delay(500);
  }
#endif

#if defined(HW_LCD)
  if (sleep_flag == 1 || interrupt_flag == 1)
    screen.printTitle(title);

  if (abs(_data - prev_data) > Absolute_error)
    screen.printWeight(_weight);
#endif

#if defined(HW_OLED)
  if (abs(_data - prev_data) > Absolute_error) {
    screen.printTitle(title);
    screen.printWeight(_weight);
  }
#endif

  if (sleep_flag == 1)
    prev_data = 0;

  sleep_flag = 0;
  interrupt_flag = 0;
  detect_new_weight_flag = 0;

  // feature: Auto turn off the screen backlight
  // if the weighing result does not change by more than (ABSOLUTE_ERROR)kg in
  // AUTO_SLEEP_TIME seconds
  if (abs(_data - prev_data) < 2 * Absolute_error) {
    while (millis() - sleep_timer > AUTO_SLEEP_TIME) {
      if (_data == 0) {
        sleep_flag = 1;
        sleep_();
        break;
      }
      for (int i = 0; i < 2; i++) {
        screen.noBacklight();
        if (waitForWeightChange(FLICKER_DELAY) != 0) {
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
  } else
    sleep_timer = millis();

  // feature: Save the results of the last RECORD_NUM weightings
  if (millis() - sleep_timer > RECORD_TIME &&
      abs(_weight - record_weight[record_weight_idx]) > ABSOLUTE_ERROR &&
      abs(_weight) > 5*ABSOLUTE_ERROR) {
    record_weight_idx++;
    if (record_weight_idx == RECORD_NUM)
      record_weight_idx = 0;
    record_id[record_weight_idx] = _id;
    record_weight[record_weight_idx] = _weight;
    Serial.println("Record: " + String(_weight));
  }

  if (_data != prev_data)
    prev_data = _data;

  //----------------------------------------------------------------------------------------------------------------------
  // Interrupt handling
  while (tare == 1 || mode == 1 || up == 1 || down == 1 || record == 1) {
    // Serial.print('*');
    uint8_t _tare = tare;
    uint8_t _mode = mode;
    uint8_t _up = up;
    uint8_t _down = down;
    uint8_t _record = record;
    prev_interrupt = interrupt;
    interrupt_flag = 1;

    // feature: Adjust the scale to 0 kg
    if (_tare == 1) {
      tare = 0;
      screen.printTitle("Digital Scale");
      screen.printContent("Taring...");
      delay(1000);
      Tare = getData_();
      screen.printTitle("Digital Scale");
      screen.printWeight(getWeight());
    }

    // feature: Change weight unit from kilogram to pound
    if (_mode == 1) {
      mode = 0;
      screen.printTitle("Digital Scale");
      Mode = (Mode == MODE_VN) ? MODE_US : MODE_VN;
      screen.printWeight(_weight);
    }
    //
    if (_up == 1 || _down == 1) {
      delay(DEBOUNCE_TIME);
      screen.printTitle("Adjust Scale    ");
    }
    // feature: Adjust weighting result up
    float w;
    while (_up == 1 && up == 1) {
      Scale -= 0.5;
      w = _data / Scale;
      screen.printWeight(w);

      if (digitalRead(UP) == LOW)
        Scale -= 2;
      else
        up = 0;
    }

    // feature: Adjust weighting result down
    while (_down == 1 && down == 1) {
      Scale += 0.5;
      w = _data / Scale;
      screen.printWeight(w);

      if (digitalRead(DOWN) == LOW)
        Scale += 2;
      else
        down = 0;
    }

    // feature: View the results of the last weightings
    uint8_t k = 0;
    while (_record == 1 && record == 1) {
      record = 0;
      k++;
      if (k == RECORD_NUM + 1) {
        k = 0;
        break;
      }
      int idx = (record_weight_idx - k + RECORD_NUM + 1) % RECORD_NUM;

      if (record_id[idx] != "")
        screen.printTitle(record_id[idx]);
      else
        screen.printTitle("Record Weight:  ");
      screen.printWeight(record_weight[idx]);
      if (waitOnInterrupt(SHOW_RECORD_TIME, &record) == 1)
        prev_interrupt++;
    }
    //----------------------------------------------------------------------------------------------------------------------
    if (_up == 1 || _down == 1)
      Absolute_error = (uint32_t)(Scale * ABSOLUTE_ERROR);

    if (_record == 1)
      NULL;
    else {
      if (waitOnInterrupt(SHOW_ISR_TIME) == 1)
        continue;
    }

    if (sleep_flag == 1)
      sleep_();
  }
  prev_interrupt = interrupt;

  delay(MAIN_DELAY);
}