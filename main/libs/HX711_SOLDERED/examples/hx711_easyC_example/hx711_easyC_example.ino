/**
 **************************************************
 *
 * @file        hx711_aesyC_example.ino
 * @brief       Arduino example for hx711 sensor
 *
 *
 *
 * @authors     @ soldered.com
 *              www.solde.red/333006
 ***************************************************/


#include "HX711-SOLDERED.h"


HX711 hxSensor;

void setup()
{
    Serial.begin(9600);
    //Call begin and give it address (0x30 - 0x37)
    hxSensor.begin(0X30);
    // Set to zerro should be called only at the beggining and only when scale is tightly placed in its position, if you get weird data, restart device
    hxSensor.setToZerro();
    hxSensor.setGain(GAIN_32);
    // Set scale will accept scale value that you can get by doing following.
    // First call setScale(1)
    // Place known weight on scale to get some value S, then use foloving formula to get scale,          
    //scale = S/knownWeight. Now change setScale(1) to setScale(scale) and scale will be calibrated
    hxSensor.setScale(1);
}

void loop()
{
    //get weight by calling getWeight function
    Serial.println(hxSensor.getWeight());
    Serial.println();
    delay(1500);
}
