/* Simple Moisture Sensor */


// Example for demonstrating the reading data from a moisture sensor


// Define pins
#define MOISPIN A0 // moisture sensor is plugged into A0

// Variables that we'll use in moisture sensor
// This value could be calibrated by reading sensor in air
const int AirValue = 620;
// This value could be calibrated by reading sensor in water
// DO NOT SUBMERGE ELECTRONIC COMPONENTS, JUST END OF THE PROBE   
const int WaterValue = 310; 
int soilMoistureValue = 0;
int soilMoisturePercent=0;

// Code in the setup block will be run once when the program starts (or device is powered up)
void setup(void) 
{
  // No setup is needed for the moisture sensor
}

// Code in the loop block is run repeatedly until the device is powered off 
void loop(void) 
{ 
  // This measures the voltage from our soil moisture sensor
  soilMoistureValue = analogRead(MOISPIN);
  
  // These lines convert analog voltage reading into moisture %
  soilMoisturePercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  if(soilMoisturePercent >= 100){
    soilMoisturePercent = 100;
    }
  else if(soilMoisturePercent<0){
    soilMoisturePercent = 0;
  }

  // These lines format the text with the measurement that will go to the computer.
  String measName = "Soil Moisture (%), ";
  String trailingComma = ", ";
  String output = measName + soilMoisturePercent + trailingComma;
  
  // This line sends the text with the measurement to the computer.
  Serial.println(output);
  
  // Change the value below to change the interval (in milliseconds) between measurements
  // For example, replacing line below with 'delay(2000);' will wait 2 seconds between measurements.
  delay(500);
}
