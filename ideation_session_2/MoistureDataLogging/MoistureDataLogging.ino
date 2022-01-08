/* Moisture sensor and data logging */


// Example for demonstrating the reading data from a moisture sensor
// and writing to an SD card

#include <SD.h>
#include <SPI.h>

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

// Variables for data logging
File dataFile;
String fileName = "data.csv";

// Code in the setup block will be run once when the program starts (or device is powered up)
void setup(void) 
{
  // Serial is the connection between the microcontroller and the computer. 
  // This line is needed to send text from the microcontroller to the computer.
  Serial.begin(9600);

  // SD setup
  // wait for SD module to start
  if (!SD.begin(4)) {
    Serial.println("No SD Module Detected");
    while (1);
  }
  // check if fileName is already in use
  while (SD.exists(fileName)){
    String newStr = "new_";
    fileName = newStr + fileName;
  }
  dataFile = SD.open(fileName, FILE_WRITE);
  if (!dataFile) {
    // if the file didn't open, print an error:
    Serial.println("error opening file");
    while (1);
  } 
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

  // Write the data to the SD card
  dataFile = SD.open(fileName, FILE_WRITE); //file to write data to
  dataFile.println(output);
  dataFile.close();
  
  // Change the value below to change the interval (in milliseconds) between measurements
  // For example, replacing line below with 'delay(2000);' will wait 2 seconds between measurements.
  delay(500);
}
