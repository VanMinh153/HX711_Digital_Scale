#line 1 "C:\\Users\\Moderator\\Documents\\Documents\\GR1_Scale\\main\\main.h"
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