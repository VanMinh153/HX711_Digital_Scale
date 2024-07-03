#line 1 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\config.h"
#ifndef config_h
#define config_h


#define DEBUB_MODE
// Define the hardware diagram and hardware opponent
// choose one of: HW_HX711, HW_HX711x4, HW_HX711x4_Sync
// #define HW_HX711
#define HW_HX711x4
#define HW_MODE_SYNC

// Choose screen: HW_LCD, HW_OLED
#define HW_LCD
// #define HW_OLED

#define UNIT_MAX_LOAD 55
#define UNIT_ABSOLUTE_ERROR 0.1f
#define SCALE 420
// for SOICT_HX711.h
// int MaxofData = (int)(UNIT_MAX_LOAD * SCALE)


#if defined(HW_HX711)
#define MAX_LOAD UNIT_MAX_LOAD
#define ABSOLUTE_ERROR UNIT_ABSOLUTE_ERROR
#elif defined(HW_HX711x4)
#define MAX_LOAD (UNIT_MAX_LOAD * 4)
#define ABSOLUTE_ERROR (UNIT_ABSOLUTE_ERROR * 4)
#endif


#define MAIN_DELAY 5
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
#define LCD_I2C_ADDRESS 0x27
#define OLED_I2C_ADDRESS 0x3C

#if defined(CONFIG_IDF_TARGET_ESP32)
#define SDA 21
#define SCL 22

#define RECORD 12
#define TARE 13
#define MODE 14
#define DOWN 25
#define UP 26

#if defined(HW_HX711)
#define DATA_PIN 0
#define CLOCK_PIN 2
#elif defined(HW_HX711x4) && !defined(HW_MODE_SYNC)
uint8_t DATA_PIN[4] = {0, 17, 18, 23};
uint8_t CLOCK_PIN[4] = {2, 16, 5, 19};
#elif defined(HW_HX711x4) && defined(HW_MODE_SYNC)
uint8_t DATA_PIN[4] = {0, 17, 18, 23};
uint8_t CLOCK_PIN[4] = {2, 2, 2, 2};
#endif

#elif defined(CONFIG_IDF_TARGET_ESP32C3)
#define SDA 2
#define SCL 3
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
#define SDA 33
#define SCL 35
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

// #define SCALE 1030
// #define AVG_TIME2READ_G128 105 // 105ms