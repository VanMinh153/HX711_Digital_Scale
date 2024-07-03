#define LED_PIN 2  // Replace with your LED pin

hw_timer_t *timer = NULL;  // Pointer to the hardware timer object

void IRAM_ATTR onTimerInterrupt() {
  // Toggle LED state
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

void setup() {
  pinMode(LED_PIN, OUTPUT);

  // Allocate a hardware timer for our use
  timer = timerBegin(0, 80000000, true);  // Timer 0, 80MHz clock, timer clock divider of 1 (80MHz)

  // Set timer mode as periodic (repeatedly counts to period value)
  timerAttachInterrupt(timer, &onTimerInterrupt, true);
  timerAlarmWrite(timer, 500000, false);  // Interrupt every 500ms (0.5 seconds)
  timerAlarmEnable(timer);

  // Start the timer without reset
  timerStart(timer);
}

void loop() {
  // No need for additional code in loop here, timer handles blinking
}
