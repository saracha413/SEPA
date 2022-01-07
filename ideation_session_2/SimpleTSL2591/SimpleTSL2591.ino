/* TSL2591 Digital Light Sensor */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

// Example for demonstrating the TSL2591 library - public domain!

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)

// Code in the setup block will be run once when the program starts (or device is powered up)
void setup(void) 
{
  // Serial is the connection between the microcontroller and the computer. 
  // This line is needed to send text from the microcontroller to the computer.
  Serial.begin(9600);
  
  // This section turns on the sensor, and notifies user if it cannot find the sensor.
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

// Code in the loop block is run repeatedly until the device is powered off 
void loop(void) 
{ 
  // This line collects a light measurement and stores it as the variable 'x'
  uint16_t x = tsl.getLuminosity(TSL2591_VISIBLE);
  
  // These lines format the text with the measurement that will go to the computer.
  String measName = "Luminosity (lux), ";
  String trailingComma = ", ";
  String output = measName + x + trailingComma;
  
  // This line sends the text with the measurement to the computer.
  Serial.println(output);
  
  // Change the value below to change the interval (in milliseconds) between measurements
  // For example, replacing line below with 'delay(2000);' will wait 2 seconds between measurements.
  delay(500);
}
