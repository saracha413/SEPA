/* TSL2591 Digital Light Sensor */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

// Example for demonstrating the TSL2591 library - public domain!

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)

void setup(void) 
{
  Serial.begin(9600);
  if (tsl.begin()) 
  {
    Serial.println(F("Found a TSL2591 sensor"));} 
  else 
  {
    Serial.println(F("No sensor found ... check your wiring?"));
    while (1);
  }

  // Configure the sensor, these settings can be changed...
  tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
}

void loop(void) 
{ 
  uint16_t x = tsl.getLuminosity(TSL2591_VISIBLE);
  Serial.print(F("Luminosity: "));
  Serial.println(x, DEC);
  delay(500);
}
