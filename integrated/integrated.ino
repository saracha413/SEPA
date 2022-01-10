//libraries to include 
//go to Sketch -> Include Libraries then select the library to include it
//if you don't see the library you're looking for, you can install it by clicking "Manage Libraries"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"
#include <SPI.h>
#include <SD.h>


//data logging from https://makersportal.com/blog/2019/3/24/arduino-sd-card-module-data-logger (currently this logs an increasing integer, not data)

//what to write data to
File testfile;
//name the file to write data to (here it's "test.csv")
//.csv stands for "comma separated values" and is the file type you should use
String fileName = "test.csv"; 
//setup the data logger
volatile int int_iter = 0;

//introduce sensors
#define ONE_WIRE_BUS1 6 //temp sensor at pin 6
#define DHTPIN 3 //humidity-temperature ("DHT") sensor at pin 3
#define DHTTYPE DHT11 //the type of DHT sensor is DHT11

//==========================================
//           temperature sensor
//==========================================

//ask installed libraries to set up temperature sensor
OneWire oneWire1(ONE_WIRE_BUS1);
DallasTemperature sensors1(&oneWire1);

float Celcius1=0;
float Fahrenheit1=0;


//==========================================
//            moisture sensor
//==========================================
// Variables that we'll use in moisture sensor
// This value could be calibrated by reading sensor in air
const int AirValue = 620;   
// This value could be calibrated by reading sensor in water
// DO NOT SUBMERGE ELECTRONIC COMPONENTS, JUST END OF THE PROBE   
const int WaterValue = 310; 
int soilMoistureValue = 0;
int soilmoisturepercent=0;


//==========================================
//            humidity sensor
//==========================================
DHT dht(DHTPIN, DHTTYPE);


//=========================================
//*****************************************
//
// The setup block is where you set up your 
// sensors to begin measurements.
//
//*****************************************
//=========================================


void setup() {

  
  //==========================================
  //           temperature sensor
  //==========================================
    
  Wire.begin();

  Serial.begin(9600); //have Serial monitor begin reading data
  sensors1.begin(); //have temperature sensor begin reading data
 
  Wire.beginTransmission(0x68); //0x68 is the temperature sesnor device address
  //assign a specific "register" (chunk of memory in the microcontroller) to the temperature sensor
  Wire.write(0x0E);
  //write a byte of information to the temperature sensor
  Wire.write(0b00011100); 
  //end the transmission to the temperature sensor
  Wire.endTransmission();

  //==========================================
  //            humidity sensor
  //==========================================  
  Serial.println(F("DHTxx test!")); //test that humidity-temperature sensor is working

  dht.begin(); //humidity-temperature sensor begins reading data


  //==========================================
  //            data logger
  //==========================================  
  // wait for SD module to start
  if (!SD.begin(4)) {
    //if computer cannot find SD module, print that module wasn't detected
    Serial.println("No SD Module Detected");
    while (1); //while this condition is true, take no other action
  }
  // see if test file exists, delete it if it does
  // then prints headers and starts new one
  if (SD.exists(fileName)){
    Serial.println(fileName+" exists, deleting and starting new");
    SD.remove(fileName);
  }
  testfile = SD.open(fileName, FILE_WRITE);
  if (testfile) {
    // save headers to file
    testfile.println("Timestamp,Data");
    testfile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening file");
  }
}

//=========================================
//*****************************************
//
// The loop block is where the action that
// the microcontroller should take goes. 
// This action is run over and over in a 
// loop.
//
//*****************************************
//=========================================

void loop() {

  
  testfile = SD.open(fileName, FILE_WRITE); //file to write data

  if (testfile) {
    //print time (number of milliseconds since Arduino began running program)
    testfile.println(String(millis()));
  
    // send request to receive data starting at register 0
    Wire.beginTransmission(0x68); // 0x68 is DS3231 device address
    Wire.write((byte)0); // start at register 0
    Wire.endTransmission();
    Wire.requestFrom(0x68, 3); // request three bytes (seconds, minutes, hours)
  
      while(Wire.available())  { 
  
        int seconds = Wire.read(); // get seconds
        int minutes = Wire.read(); // get minutes
        int hours = Wire.read();   // get hours
     
        seconds = (((seconds & 0b11110000)>>4)*10 + (seconds & 0b00001111)); // convert BCD to decimal
        minutes = (((minutes & 0b11110000)>>4)*10 + (minutes & 0b00001111)); // convert BCD to decimal
        hours = (((hours & 0b00100000)>>5)*20 + ((hours & 0b00010000)>>4)*10 + (hours & 0b00001111)); // convert BCD to decimal (assume 24 hour mode)
     
        //Serial.print(hours); Serial.print(":"); Serial.print(minutes); Serial.print(":"); Serial.println(seconds);
        testfile.print(hours); testfile.print(":"); testfile.print(minutes); testfile.print(":"); testfile.println(seconds);
      
        //==========================================
        //           temperature sensors
        //==========================================
        
        sensors1.requestTemperatures(); 
        Celcius1=sensors1.getTempCByIndex(0);
        Fahrenheit1=sensors1.toFahrenheit(Celcius1);
        //Serial.print(" C  ");
        //Serial.print(Celcius);
        testfile.println("Sensor 1: "+String(Fahrenheit1)+" F");
        Serial.println("Temperature sensor: "+String(Fahrenheit1)+" F");
      
        //==========================================
        //           moisture sensor
        //==========================================
    
        soilMoistureValue = analogRead(A0);  //ask sensor to read soil moisture value
        soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
        if(soilmoisturepercent >= 100){
          testfile.print("Soil moisture is 100 %");
          Serial.print("Soil moisture is 100 %");
         }
        else if(soilmoisturepercent <=0)  {
          testfile.print("Soil moisture is 0 %");
          Serial.print("Soil moisture is 0 %");
          }
        else if(soilmoisturepercent >0 && soilmoisturepercent < 100) {
          testfile.print("Soil moisture is " + String(soilmoisturepercent)+"%");
          Serial.print("Soil moisture is " + String(soilmoisturepercent)+"%");
      
          }
          
        //==========================================
        //           humidity sensor
        //==========================================
  
          // Reading temperature or humidity takes about 250 milliseconds!
          // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
          
          float h = dht.readHumidity(); //read humidity, h, from sensor
          // Read temperature as Celsius (the default)
          float t = dht.readTemperature();
          // Read temperature as Fahrenheit (isFahrenheit = true)
          float f = dht.readTemperature(true);
        
          // Check if any reads failed and exit early (to try again).
          if (isnan(h) || isnan(t) || isnan(f)) {
            Serial.println(F("Failed to read from DHT sensor!"));
            testfile.print(F("Failed to read from DHT sensor!"));
            return;
          }
        
          // Compute heat index in Fahrenheit (the default)
          float hif = dht.computeHeatIndex(f, h);
          // Compute heat index in Celsius (isFahreheit = false)
          float hic = dht.computeHeatIndex(t, h, false);


          testfile.println("Humidity: "+String(h)+ "%  Temperature: "+String(t)+"°C ");
          Serial.println("Humidity: "+String(h)+ "%  Temperature: "+String(t)+"°C ");
  
        
      }
      
      testfile.close(); //close and save file you've been writing to
      
      } else {
        Serial.println("error opening file");
      }
      int_iter+=1; //increase number of times loop has been run
      
    delay(2000); //wait 2s before beginning loop again
  

}
