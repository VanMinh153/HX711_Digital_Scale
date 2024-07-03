const int buttonPin = 2; // Replace with your desired button pin

volatile bool buttonPressed = false;  // Flag to store button state

void IRAM_ATTR buttonInterrupt() {
  buttonPressed = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);  // Set button pin as input with pull-up resistor

  // Attach interrupt to rising edge (button press) on buttonPin
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonInterrupt, RISING);
}

void loop() {
  if (buttonPressed) {
    Serial.println("Button pressed!");
    buttonPressed = false;  // Reset flag after handling press
  }
  delay(10);  // Avoid busy loop
}
