// _MACHINE__DEFAULT_TYPES_H

#ifndef main_h
#define main_h
#include "config.h"
#include "screen.h"
#include "SOICT_HX711.h"


#if defined(HW_HX711)
extern HX711 sensor;
#elif defined(HW_HX711x4)
extern HX711List sensor;
#endif

#if defined(HW_LCD)
extern LCD_I2C screen;
#elif defined(HW_OLED)
extern OLED_SSD1306 screen;
#endif

//----------------------------------------------------------------------------------------------------------------------
extern int Tare;
extern float Scale;
extern uint8_t Mode;
extern uint16_t Absolute_error;
//----------------------------------------------------------------------------------------------------------------------
extern volatile uint8_t tare;
extern volatile uint8_t mode;
extern volatile uint8_t up;
extern volatile uint8_t down;
extern volatile uint8_t record;
extern volatile uint8_t interrupt;
extern volatile unsigned long prev_press;
extern uint8_t prev_interrupt;
//----------------------------------------------------------------------------------------------------------------------
extern int _data;
// extern float prev_data;
// extern float _weight;
// extern float record_weight[RECORD_NUM];

extern unsigned long timer;
// extern uint8_t sleep_flag;
extern uint8_t wake_up_flag;
// extern uint8_t interrupt_flag;

#endif

// hx711_gain_t Gain = CHAN_A_GAIN_128;
// int Tare = 0;
// float Scale = SCALE;
// uint8_t Mode = KG_MODE;
// uint16_t Absolute_error = (uint16_t)(Scale * ABSOLUTE_ERROR);
// //----------------------------------------------------------------------------------------------------------------------
// volatile uint8_t tare = 0;
// volatile uint8_t mode = 0;
// volatile uint8_t up = 0;
// volatile uint8_t down = 0;
// volatile uint8_t record = 0;
// volatile uint8_t interrupt = 0;
// volatile unsigned long prev_press = millis();
// uint8_t prev_interrupt = 0;
// //----------------------------------------------------------------------------------------------------------------------
// int _data = 0;
// float prev_data = 0;
// float _weight = 0;
// float record_weight[RECORD_NUM];

// unsigned long timer = millis();
// int prev_readData_val = 0;
// uint32_t sensor_error = 0;
// uint8_t sleep_flag = 0;
// uint8_t wake_up_flag = 0;
// uint8_t interrupt_flag = 0;