/* DHT11 Temperature and Humidity Sensor */

#include <DHT.h>

// Defines
#define DHTPIN 12     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11

// Initialize variable for sensor
DHT dht(DHTPIN, DHTTYPE);

// Code in the setup block will be run once when the program starts (or device is powered up)
void setup(void) 
{
  // Serial is the connection between the microcontroller and the computer. 
  // This line is needed to send text from the microcontroller to the computer.
  Serial.begin(9600);
  
  // This section turns on the sensor, and notifies user if it cannot find the sensor.
  if (dht.begin()) 
  {
    Serial.println(F("Found a DHT sensor"));
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
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // These lines format the text with the measurement that will go to the computer.
  String measName = "Temperature (C), ";
  String trailingComma = ", ";
  String output = measName + t + trailingComma;

  String measName = "Humidity (%), ";
  String trailingComma = ", ";
  // Compare with above, the output string has already been created
  // (in line 41). Now we are just adding the additional measurement to it
  output = output + measName + h + trailingComma;
  
  // This line sends the text with the measurement to the computer.
  Serial.println(output);
  
  // Change the value below to change the interval (in milliseconds) between measurements
  // For example, replacing line below with 'delay(2000);' will wait 2 seconds between measurements.
  delay(3000);
}
