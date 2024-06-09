/**
 **************************************************
 *
 * @file        hx711_native_example.ino
 * @brief       Arduino example for hx711 sensor
 *
 *
 *
 * @authors     @ soldered.com
 *              www.solde.red/333005
 ***************************************************/


#include "HX711-SOLDERED.h"

// connect pins from your board to hx711 breakout
#define DOUT    6
#define PD_SCLK 7

HX711 hxSensor(DOUT, PD_SCLK);

void setup()
{
    Serial.begin(9600);
    hxSensor.begin();
    // Set to zerro should be called only at the beggining and only when scale is tightly placed in its position
    hxSensor.setToZerro();
    hxSensor.setGain(GAIN_128);
    // Set scale will accept scale value that you can get by doing following.
    // First call setScale(1)
    // Place known weight on scale to get some value S, then use foloving formula to get scale,          scale =
    // S/knownWeight. Now change setScale(1) to setScale(scale) and scale will be calibrated
    hxSensor.setScale(13.4);
}

void loop()
{
    Serial.println(hxSensor.getWeight());
    Serial.println();
    delay(1500);
}
