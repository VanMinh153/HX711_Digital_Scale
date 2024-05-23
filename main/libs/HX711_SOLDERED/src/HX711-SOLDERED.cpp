/**
 **************************************************
 *
 * @file        HX711.cpp
 * @brief       Arduino library for hx711 breakout
 *
 *
 *
 * @authors     @ soldered.com @Robert Peric
 ***************************************************/

#include "HX711-SOLDERED.h"

/**
 * @brief           HX711 native constructor.
 *
 * @param           int dout pin connected to hx711 dout pin
 * @param           int pd_sck pin connected to hx711 pd_sck pin
 */
HX711::HX711(int dout, int pd_sck)
{
    _pd_sck = pd_sck;
    _dout = dout;

    native = 1;
}

/**
 * @brief       HX711 default constructor used with easyC
 */
HX711::HX711()
{
}

/**
 * @brief       getData communicates with hx controller and collects data from device
 *
 * @return      data from device data register
 */
uint32_t HX711::getData()
{
    uint32_t data = 0;

    if (native)
    {
        data = readDataNative();
    }
    else
    {
        uint8_t g[1];
        g[0] = _gain;
        sendData(g, 1);

        uint8_t buf[4];

        readData(buf, 4);
        data |= ((uint32_t)buf[0] << 24);
        data |= ((uint32_t)buf[1] << 16);
        data |= ((uint32_t)buf[2] << 8);
        data |= ((uint32_t)buf[3] << 0);
    }


    return ((data / 100) - _zerroConstant);
}

/**
 * @brief                   Overloaded function for virtual in base class to initialize sensor specific.
 */
void HX711::initializeNative()
{
    pinMode(_pd_sck, OUTPUT);
    pinMode(_dout, INPUT);
}

/**
 * @brief       setToZerro sets _zerroConstant to some value that will be used to get 0 when nothing is on scale
 */
void HX711::setToZerro()
{
    _zerroConstant = getData();
}

/**
 * @brief       setScale sets scale constant that will be multiplied with data we get from sensor to get value in grams
 *
 * @param       float scale sets _scaleConstant that is used when returning weights in grams
 */
void HX711::setScale(float scale)
{
    _scaleConstant = scale;
}

/**
 * @brief       getWeight calculates weight in grams
 *
 * @return      float weight in grams
 */
float HX711::getWeight()
{
    return getData() / _scaleConstant;
}

/**
 * @brief       setGain sets _gain variable thats used for controling sensor
 *
 * @param       uint8_t gain posible values are GAIN_128, GAIN_64, GAIN_32
 */
void HX711::setGain(uint8_t gain)
{
    if (gain >= 25 && gain <= 27)
    {
        _gain = gain;
    }
}

/**
 * @brief       readDataNative reads data from sensor using pins
 *
 * @return      data from device data register
 */
uint32_t HX711::readDataNative()
{
    // power down just to make sure
    digitalWrite(_pd_sck, HIGH);
    delayMicroseconds(70);

    // power up
    digitalWrite(_pd_sck, LOW);
    delayMicroseconds(100);

    // wait for DOUT pin to go LOW and signal data available
    uint32_t timer = millis();
    while (digitalRead(_dout) == HIGH)
    {
        if (millis() - timer > 550)
        {
            return 0;
        }
    }

    // Set gain for next reading of data
    if (_gain >= 25 && _gain <= 27)
    {
        for (int i = 0; i < _gain; i++)
        {
            digitalWrite(_pd_sck, HIGH);
            delayMicroseconds(4);
            digitalWrite(_pd_sck, LOW);
            delayMicroseconds(4);
        }
    }


    // Wait aggain for DOUT pin to signal available data
    timer = millis();
    while (digitalRead(_dout) == HIGH)
    {
        if (millis() - timer > 550)
        {
            return 0;
        }
    }

    uint32_t result = 0;
    // read data
    for (int i = 0; i < 24; ++i)
    {
        digitalWrite(_pd_sck, HIGH);
        delayMicroseconds(4);

        result = ((result << 1) | digitalRead(_dout));

        digitalWrite(_pd_sck, LOW);
        delayMicroseconds(4);
    }

    // One last pulse to set dout to high
    digitalWrite(_pd_sck, HIGH);
    delayMicroseconds(4);
    digitalWrite(_pd_sck, LOW);
    delayMicroseconds(4);

    // turn chip off
    digitalWrite(_pd_sck, HIGH);
    delayMicroseconds(70);

    return result;
}