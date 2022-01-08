/* Temperature Sensor (DS18B20) */

#include <OneWire.h>
#include <DallasTemperature.h>

// Defines
#define ONEWIREPIN 11     // Digital pin connected to the DHT sensor

// Initialize variable for sensor
OneWire oneWire(ONEWIREPIN);
DallasTemperature sensors(&oneWire);

// Code in the setup block will be run once when the program starts (or device is powered up)
void setup(void) 
{
  // Serial is the connection between the microcontroller and the computer. 
  // This line is needed to send text from the microcontroller to the computer.
  Serial.begin(9600);
  
  // This section turns on the sensor, and notifies user if it cannot find the sensor.
  if (sensors.begin()) 
  {
    Serial.println(F("Found a temperature sensor"));
  } 
  else 
  {
    Serial.println(F("No sensor found ... check your wiring?"));
    while (1);
  }
}

// Code in the loop block is run repeatedly until the device is powered off 
void loop(void) 
{ 
  // These lines collect a measurement
  float t = sensors.getTempCByIndex(0);

  // These lines format the text with the measurement that will go to the computer.
  String measName = "Temperature 1 (C), ";
  String trailingComma = ", ";
  String output = measName + t + trailingComma;

  // This line sends the text with the measurement to the computer.
  Serial.println(output);
  
  // Change the value below to change the interval (in milliseconds) between measurements
  // For example, replacing line below with 'delay(2000);' will wait 2 seconds between measurements.
  delay(3000);
}
