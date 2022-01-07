/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  This example code is in the public domain.
  Adapted from:
  http://www.arduino.cc/en/Tutorial/Blink
*/

// define statements make code more legible by letting us write descriptive
// names in the code (e.g. 'ledPin') instead of values (e.g. the integer 13)
#define ledPin 13

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(ledPin, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level)
  // The number after delay is how many milliseconds the microcontroller should
  // wait for before going to the next step. Try changing it to something else
  // for example: 'delay(100);'
  delay(1000);                       // wait
  digitalWrite(ledPin, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait
}
