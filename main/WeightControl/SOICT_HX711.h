/**
 * @brief      HX711 load cell ADC interface header
 * @author     Nguyen Van Minh - SOICT-HUST
 * @date       2025-06-14
 *
 * Defines classes for single and multi-HX711 chip support.
 */
#ifndef SOICT_HX711_H
#define SOICT_HX711_H

#include <Arduino.h>

// #define DEBUG_SOICT_HX711 // Uncomment to enable debug output

#define HX711_FAIL 0xFFFF // Error code for failed read

// Gain settings: 32 for B+-, 128/64 for A+-
enum hx711_gain_t
{
  CHAN_A_GAIN_128 = 25, // Channel A, gain 128
  CHAN_A_GAIN_64 = 27,  // Channel A, gain 64
  CHAN_B_GAIN_32 = 26   // Channel B, gain 32
};
#define HX711_DEFAULT_GAIN CHAN_A_GAIN_128
//--------------------------------------------------------------------------------------------------
// Basic low-level HX711 interface
class HX711Mini
{
public:
  HX711Mini();
  HX711Mini(uint8_t dataPin, uint8_t clockPin);
  void begin(); // Initialize pins
  void powerDown(); // Enter power down mode
  void powerUp();   // Exit power down mode
  bool isReady();   // Check if data is ready
  int readData(hx711_gain_t gain); // Read 24 bits from HX711

  uint8_t dataPin;  // Data pin number
  uint8_t clockPin; // Clock pin number
};

//--------------------------------------------------------------------------------------------------
// High-level HX711 interface with tare, scale, and gain
class HX711 : public HX711Mini
{
public:
  HX711();
  HX711(uint8_t dataPin, uint8_t clockPin, hx711_gain_t gain = HX711_DEFAULT_GAIN);
  void setGain(hx711_gain_t gain_val); // Set gain
  void setPowerDown(bool powerDown_val); // Power down/up

  void setTare(int tare_val, hx711_gain_t gain = HX711_DEFAULT_GAIN); // Set tare value
  void setScale(float scale_val, hx711_gain_t gain = HX711_DEFAULT_GAIN); // Set scale factor
  void tare(hx711_gain_t gain = HX711_DEFAULT_GAIN); // Perform tare
  float getWeight(hx711_gain_t gain = HX711_DEFAULT_GAIN); // Get weight
  int readData(hx711_gain_t gain); // Read data with gain
  int readData(); // Read data with current gain

protected:
  hx711_gain_t gain = HX711_DEFAULT_GAIN; // Current gain
  hx711_gain_t setting_gain = HX711_DEFAULT_GAIN; // Last set gain
  bool _powerDown = 0; // Power down state
  int _tareA_128 = 0; // Tare for channel A
  int _tareB = 0;     // Tare for channel B
  float scaleA_128 = 1; // Scale for channel A
  float scaleB = 1;     // Scale for channel B
};
//--------------------------------------------------------------------------------------------------
// Multi-HX711 support (for multiple load cells)
class HX711List
{
public:
  HX711List(uint8_t hx711_num,
            uint8_t *dataPin, uint8_t *clockPin,
            hx711_gain_t gain = CHAN_A_GAIN_128);
  void begin(); // Initialize all HX711s
  bool isReady(); // Check if all are ready
  void powerDown(); // Power down all
  void powerUp();   // Power up all
  void setGain(hx711_gain_t gain_val); // Set gain for all

  int readData(uint8_t hx711_num); // Read from one HX711
  int readData(); // Read from all (average)
  int readDataAsync(hx711_gain_t gain_val); // Async read
  int readDataSync(hx711_gain_t gain_val);  // Sync read

  uint8_t errRead = 0; // Error count
  int DataUnitMax;     // Max data value

protected:
  HX711Mini *hx711;    // Array of HX711s
  uint8_t Size;        // Number of HX711s
  hx711_gain_t gain = CHAN_A_GAIN_128; // Current gain
  bool _powerDown = 0; // Power down state
  bool SyncMode;       // Sync mode flag
  uint8_t clockPin;    // Shared clock pin (if sync)
};

#endif
