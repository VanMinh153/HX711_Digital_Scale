#ifndef config_h
#define config_h

#define CONFIG_IDF_TARGET_ESP32 1
// Enable this to use debug code in program
// #define DEBUG_MODE

/* 
Choose hardware opponent
1 HW_HX711
2 HW_HX711x4: 4 HX711 chips connected to 4 load cells
  2.1 HW_MODE_SYNC: 4 HX711 chips are connected to the same clock pin
  2.2 HW_MODE_ASYNC: 4 HX711 chips are connected to different clock pins
*/

#define HW_HX711
// #define HW_HX711x4
// #define HW_MODE_SYNC
// #define HW_MODE_ASYNC

// Choose display screen
// #define HW_LCD
#define HW_OLED

// Choose temperature sensor
// #define HW_TEMPERATURE
// #define HW_NTC
#define PIN_NTC 15
#define BETA 3950 // WOKWI NTC Constants
// #define HW_DHT
// #define HW_LM35
// #define ADC_VREF 3300
// #define ADC_RESOLUTION 4096

// Choose RFID
#define HW_RFID
#if defined(HW_RFID)
#define PIN_SS 5
#define PIN_RST 17
#endif

// Config for HX711 and load cell
#define UNIT_MAX_LOAD 50
#define UNIT_ABSOLUTE_ERROR 0.1f
#define SCALE 420

#if defined(HW_HX711)
#define MAX_LOAD 50
#define ABSOLUTE_ERROR 0.1f
#elif defined(HW_HX711x4)
#define MAX_LOAD (UNIT_MAX_LOAD * 4)
#define ABSOLUTE_ERROR (UNIT_ABSOLUTE_ERROR * 4)
#endif

// Config for main program
#define MAIN_DELAY 5
#define AUTO_SLEEP_TIME 12000
#define DELAY_RFID_TIME 6000
#define RECORD_TIME 600
#define FLICKER_DELAY 350
#define SLEEP_DELAY 2000
#define SHOW_RECORD_TIME 2500
#define SHOW_ISR_TIME 2000
#define DEBOUNCE_TIME 200
#define RECORD_NUM 6
#define MODE_VN 0
#define MODE_US 1
#define KG_TO_LB 2.204623f
#define MAIN_TITLE "Digital Scale"
#define LCD_I2C_ADDRESS 0x27
#define OLED_I2C_ADDRESS 0x3C

// Config for pins
#if defined(CONFIG_IDF_TARGET_ESP32)
#define PIN_SDA 21
#define PIN_SCL 22
#define RECORD 12
#define TARE 13
#define MODE 14
#define DOWN 25
#define UP 26

#if defined(HW_HX711)
#define DATA_PIN 16
#define CLOCK_PIN 2
#elif defined(HW_HX711x4) && !defined(HW_MODE_SYNC)
uint8_t DATA_PIN[4] = {0, 17, 18, 23};
uint8_t CLOCK_PIN[4] = {2, 16, 5, 19};
#elif defined(HW_HX711x4) && defined(HW_MODE_SYNC)
uint8_t DATA_PIN[4] = {0, 17, 18, 23};
uint8_t CLOCK_PIN[4] = {2, 2, 2, 2};
#endif

#elif defined(CONFIG_IDF_TARGET_ESP32C3)
#define PIN_SDA 2
#define PIN_SCL 3
#define TARE 4
#define MODE 5
#define DOWN 8
#define UP 9
#define RECORD 18

#if defined(HW_HX711)
#define DATA_PIN 6
#define CLOCK_PIN 7
#elif defined(HW_HX711x4) && !defined(HW_MODE_SYNC)
uint8_t DATA_PIN[4] = {6, 16, 37, 39};
uint8_t CLOCK_PIN[4] = {7, 17, 38, 40};
#endif

#elif defined(CONFIG_IDF_TARGET_ESP32S2)
#define PIN_SDA 33
#define PIN_SCL 35
#define TARE 15
#define MODE 18
#define DOWN 17
#define UP 16
#define RECORD 21

#if defined(HW_HX711)
#define DATA_PIN 39
#define CLK_PIN 38
#elif defined(HW_HX711x4) && !defined(HW_MODE_SYNC)
uint8_t DATA_PIN[4] = {39, 12, 10, 8};
uint8_t CLOCK_PIN[4] = {38, 13, 11, 9};
#elif defined(HW_HX711x4) && defined(HW_MODE_SYNC)
uint8_t DATA_PIN[4] = {39, 13, 11, 9};
uint8_t CLOCK_PIN[4] = {10, 10, 10, 10};
#endif
#endif

#endif