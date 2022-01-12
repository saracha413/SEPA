/*
G. Goebel and S. Streeter
1/12/2022

This sketch is for Lesson 3, Recording Data Over Time.
The sketch is setup for one sensor (either an Adafruit
TSL2591 light sensor or a Songhe B07SYBSHGX moisture
sensor), a MicroSD Card Reader (NewZoll B01MSNX0TW)for 
saving data, and an Arduino UNO R3.

Importantly, the Arduino UNO board by itself does NOT 
accomodate both sensors and the card reader given the 
limited  number of power supply pins (only 3.3V and 5V). 
Still, this code is setup to run smoothly with either 
sensor connected along with the card reader; simply 
ignore the data stream coming from the unplugged sensor 
in the serial monitor and in the generated output file.

In the future, this sketch could be broken into two
separate sketches for Lesson 3, depending on which
sensor the teachers/students wish to use. This sketch
could also be expanded upon in Lesson 4 by adding more
"measNameX" and "dataX" variables along with adding 
necessary setup/measurement code snippets.

-------PIN CONNECTION INFO-------

MicroSD card reader:
(Card reader pin <--> Arduino UNO pin)
CS   <--> Digital pin 4
SCK  <--> Digital pin 13 
MOSI <--> Digital pin 11
MISO <--> Digital pin 12
VCC  <--> 5V
GND  <--> GND

Light sensor:
(Sensor cable color <--> Arduino UNO pin)
Yellow <--> SCL
Blue   <--> SDA
Red    <--> 3.3V
Black  <--> GND

Moisture sensor:
(Sensor pin <--> Arduino UNO pin)
AOUT <--> Analog pin 0 (A0)
VCC  <--> 3.3V
GND  <--> GND

---------------------------------

*/

// Libraries needed for SD card reader
#include <SPI.h>
#include <SD.h>

// Libraries needed for light sensor (Adafruit TSL2591)
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

// Libraries needed for moisture sensor (Songhe B07SYBSHGX)
// None!

//-----------------------------------------------------------------------

// SET THE DELAY TIME BETWEEN MEAUREMENTS HERE
int DelayTime = 3; // In units of seconds

//-----------------------------------------------------------------------

// SETUP FOR SD CARD
File outputfile;
String fileName = "output.csv";

// SETUP FOR LIGHT SENSOR
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // Pass sensor ID number (for your use later)

// SETUP FOR MOISTURE SENSOR
// DO NOT SUBMERGE ELECTRONIC COMPONENTS, JUST END OF THE PROBE   
#define MOISPIN A0 // moisture sensor is plugged into A0
const int AirValue      = 620; // This value is determined by reading sensor in air
const int WaterValue    = 310; // This value is determined by reading sensor in water
int soilMoistureValue   = 0;
int soilMoisturePercent = 0;
int data2               = 0;

// SETUP FOR OTHER SENSORS HERE

// SETUP FOR MEASUREMENT STRINGS
String measName1  = "Light Intensity (lux)"; // or (infrared) or (visible) or (full), see "data1" variable
String measName2  = "Soil Moisture (%)";
String commaSpace = ", ";
String space      = " ";

//-----------------------------------------------------------------------

// SEPARATE CONFIGURATION FUNCTION NEEDED FOR LIGHT SENSOR
void configureSensor(void)
{
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  //tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain
  
  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)
}

//-----------------------------------------------------------------------

// START
void setup() {

  // Start serial port
  Serial.begin(9600);
  // wait for SD module to start
  if (!SD.begin(4)) {
    Serial.println("No SD module detected. Confirm memory card is in place!");
    while (1);
  }

  // Connect to the light sensor
  if (tsl.begin()) {
    Serial.println(F("Found a TSL2591 sensor"));
  } else {
    Serial.println(F("No sensor found ... check your wiring?"));
    //while (1);
  }
  // Configure the light sensor
  configureSensor();

  // No initial connection to the moisure sensor is needed, because it is
  // an analog sensor!

  // Connect to any other sensors here

  // See if old output data file exists, delete it if it does
  if (SD.exists(fileName)){
    Serial.println(fileName+" exists, deleting and starting new");
    SD.remove(fileName);
  }

  // Print column headers in new output file
  outputfile = SD.open(fileName, FILE_WRITE);
  if (outputfile) {
    // save headers to file
    outputfile.println("Timestamp (s),"+measName1+","+measName2); // Can add on here for more sensors
    outputfile.close();
  } else {
    // If the file didn't open, print an error:
    Serial.println("Error opening output file!");
  }
}

//-----------------------------------------------------------------------

// BEGIN DATA COLLECTION LOOP
void loop() {

  // Create output CSV file (that can be opened in Excel later on)
  outputfile = SD.open(fileName, FILE_WRITE);
  if (outputfile) {

    //-----------------------------------------------------------------------

    // LIGHT SENSOR
    // Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
    // That way you can do whatever math and comparisons you want!
    uint32_t lum  = tsl.getFullLuminosity();
    uint16_t ir   = lum >> 16;    // Infrared spectrum intensity
    uint16_t full = lum & 0xFFFF; // Full spectrum intensity
    uint16_t vis  = full - ir;    // Visible spectrum intensity
    // Unit for overall light intensity per International System of Units
    uint16_t lux  = tsl.calculateLux(full, ir);
    
    // Set what you would like to record (remember to change measName1 too!)
    uint16_t data1 = lux;
    
    //-----------------------------------------------------------------------

    // MOISTURE SENSOR
    // This measures the voltage from our soil moisture sensor
    soilMoistureValue = analogRead(MOISPIN);
    
    // These lines convert analog voltage reading into moisture %
    soilMoisturePercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
    if(soilMoisturePercent >= 100){
      soilMoisturePercent = 100;
      } else if(soilMoisturePercent<0){
      soilMoisturePercent = 0;
    }
    // Rename moisture sensor measurement
    data2 = soilMoisturePercent;

    //-----------------------------------------------------------------------

    // OTHER SENSORS

    //-----------------------------------------------------------------------

    // CREATE STRINGS FOR OUTPUT FILE AND MONITOR
    // These lines format the measurement as text...
    // for the output file
    String output_to_file    = data1 + commaSpace + data2 + commaSpace;
    // and for the serial monitor
    String output_to_monitor = measName1 + space + data1 + commaSpace + measName2 + space + data2;
    
    //-----------------------------------------------------------------------
    
    // SD CARD WRITING HERE   
    // Save new measurement in every loop and then wait delay(X), where X is 
    // milliseconds before next measurement...
    // in the output file
    outputfile.println(String(millis()/1000) + ","+output_to_file);
    outputfile.close();
    // or print it to the serial monitor 
    Serial.println("[" + String(millis()/1000) + " s] Saving: " + output_to_monitor);
    
    //-----------------------------------------------------------------------
  
    // Close and save the output file 
    outputfile.close();
  
  } else {
    Serial.println("Error opening output file!");
  }

  // WAIT FOR NEXT MEASUREMENT
  // This delay() input argument needs to be in milliseconds (hence the * 1000)
  delay(DelayTime*1000);
  
}
