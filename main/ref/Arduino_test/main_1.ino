#include <util/atomic.h>

#define DEBOUNCE_TIME 200
#define RECORD 2

unsigned long buttonTime = millis();
volatile uint8_t i = 0;

void recordInterrupt()
{
  if (millis() - buttonTime < DEBOUNCE_TIME)
  {
    return;
  }
  buttonTime = millis();
  i++;
  Serial.print("Record ");
  Serial.println(i);
  // ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
  // }
}

void setup()
{
  Serial.begin(57600);
  attachInterrupt(digitalPinToInterrupt(RECORD), recordInterrupt, CHANGE);
  noInterrupts();
  delay(1000);
}

// int i = 0;
volatile unsigned long k = 0;
void loop()
{
  k = 0;
  while (k != 12345)
  {
    k++;
  }
  Serial.println(k);
  while (k != 0xfffffL)
  {
    k++;
  }
  Serial.println("!");
  delay(500);
}