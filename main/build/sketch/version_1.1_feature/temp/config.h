#line 1 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\version_1.1_feature\\temp\\config.h"
#ifndef config_h
#define config_h

#include "HX711-HEDSPI.h"

// define one of the following macros to choose the correct hardware
// DIAGRAM_HX711, DIAGRAM_HX711x4
// #define DIAGRAM_HX711
#define DIAGRAM_HX711x4
// #define DIAGRAM_HX711x4_Sync

#define SCALE 420
// #define SCALE 1030
#define MAX_LOAD 200
// #define AVG_TIME2READ_G128 105 // 105ms
#define ABSOLUTE_ERROR 0.1f
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

#if defined(CONFIG_IDF_TARGET_ESP32C3)
#define SDA 2
#define SCL 3
#define TARE 4
#define MODE 5
#define DOWN 8
#define UP 9
#define RECORD 18

#if defined(DIAGRAM_HX711)
#define DATA_PIN 6
#define CLK_PIN 7
#elif defined(DIAGRAM_HX711x4)
uint8_t data_pin[4] = {6, 16, 37, 39};
uint8_t clk_pin[4] = {7, 17, 38, 40};
#endif

#elif defined(CONFIG_IDF_TARGET_ESP32S2)
#define SDA 33
#define SCL 35
#define TARE 15
#define MODE 18
#define DOWN 17
#define UP 16
#define RECORD 21

#if defined(DIAGRAM_HX711)
#define DATA_PIN 39
#define CLK_PIN 38
#elif defined(DIAGRAM_HX711x4)
uint8_t data_pin[4] = {39, 12, 20, 8};
uint8_t clk_pin[4] = {38, 13, 11, 9};
#endif
#endif

#if defined(DIAGRAM_HX711)
HX711 sensor(DATA_PIN, CLK_PIN, GAIN_128, SCALE);
#elif defined(DIAGRAM_HX711x4)
HX711List sensor(4, data_pin, clk_pin, CHAN_A_GAIN_128);
#endif


#endif