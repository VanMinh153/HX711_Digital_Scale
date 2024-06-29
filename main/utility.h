#ifndef utility_h
#define utility_h
#include "main.h"
// #include "SOICT_HX711.h"
// #include <LiquidCrystal_I2C.h>

void IRAM_ATTR recordISR();
void IRAM_ATTR tareISR();
void IRAM_ATTR modeISR();
void IRAM_ATTR upISR();
void IRAM_ATTR downISR();

void sort_(int arr[], uint8_t n, int avg);
int getData_Avg(HX711 adc);
int getData_(uint8_t allow_delay = 1);
float getWeight();
float toWeight(int data);
uint8_t sleep_(uint8_t sensitivity = 2);
uint8_t waitForWeightChange(uint16_t timeout, uint16_t time2listen = 50, uint16_t error = Absolute_error);
uint8_t waitOnInterrupt(uint32_t timeout, volatile uint8_t *isrCtl = NULL);
void setGain(hx711_gain_t gain);
#endif


// //----------------------------------------------------------------------------------------------------------------------
// extern hx711_gain_t Gain;
// extern int Tare;
// extern float Scale;
// extern uint8_t Mode;
// extern uint16_t Absolute_error;
// //----------------------------------------------------------------------------------------------------------------------
// extern volatile uint8_t tare;
// extern volatile uint8_t mode;
// extern volatile uint8_t up;
// extern volatile uint8_t down;
// extern volatile uint8_t record;
// extern volatile uint8_t interrupt;
// extern volatile unsigned long prev_press;
// extern uint8_t prev_interrupt;
// //----------------------------------------------------------------------------------------------------------------------
// extern int _data;
// // extern float prev_data;
// // extern float _weight;
// // extern float record_weight[RECORD_NUM];

// extern unsigned long timer;
// extern int prev_readData_val;
// extern uint32_t sensor_error;
// // extern uint8_t sleep_flag;
// extern uint8_t wake_up_flag;
// // extern uint8_t interrupt_flag;