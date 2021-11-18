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


//data logging from https://makersportal.com/blog/2019/3/24/arduino-sd-card-module-data-logger

//what to write data to
File testfile;
String fileName = "test.csv";
//setup the data logger
volatile int int_iter = 0;

//pins
#define ONE_WIRE_BUS1 6 //temp sensor 1
#define DHTPIN 3 //humidity sensor
#define DHTTYPE DHT11 //humidity sensor

//==========================================
//           temperature sensor
//==========================================
OneWire oneWire1(ONE_WIRE_BUS1);

DallasTemperature sensors1(&oneWire1);

float Celcius1=0;
float Fahrenheit1=0;


//==========================================
//            moisture sensor
//==========================================
const int AirValue = 620;   
const int WaterValue = 310; 
int soilMoistureValue = 0;
int soilmoisturepercent=0;


//==========================================
//            humidity sensor
//==========================================
DHT dht(DHTPIN, DHTTYPE);





void setup() {

  
  //==========================================
  //           temperature sensor
  //==========================================
    
  Wire.begin();

  Serial.begin(9600);
  sensors1.begin();

  Wire.beginTransmission(0x68);
  Wire.write(0x0E); 
  Wire.write(0b00011100); 
  Wire.endTransmission();

  //==========================================
  //            humidity sensor
  //==========================================  
  Serial.println(F("DHTxx test!"));

  dht.begin();


  //==========================================
  //            data logger
  //==========================================  
  // wait for SD module to start
  if (!SD.begin(4)) {
    Serial.println("No SD Module Detected");
    while (1);
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

void loop() {

  
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
   
      Serial.print(hours); Serial.print(":"); Serial.print(minutes); Serial.print(":"); Serial.println(seconds);

    
      //==========================================
      //           temperature sensors
      //==========================================
      
      sensors1.requestTemperatures(); 
      Celcius1=sensors1.getTempCByIndex(0);
      Fahrenheit1=sensors1.toFahrenheit(Celcius1);
      //Serial.print(" C  ");
      //Serial.print(Celcius);
      Serial.print(" Sensor 1");
      Serial.print(" F  ");
      Serial.println(Fahrenheit1);
    
      //==========================================
      //           moisture sensor
      //==========================================
  
      soilMoistureValue = analogRead(A0);  //put Sensor insert into soil
      //Serial.println(soilMoistureValue);
      soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
      if(soilmoisturepercent >= 100){
        Serial.println("100 %");
       }
      else if(soilmoisturepercent <=0)  {
        Serial.println("0 %");
        }
      else if(soilmoisturepercent >0 && soilmoisturepercent < 100) {
        Serial.print(soilmoisturepercent);
        Serial.println("%");
    
        }
        
      //==========================================
      //           humidity sensor
      //==========================================

        // Reading temperature or humidity takes about 250 milliseconds!
        // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
        float h = dht.readHumidity();
        // Read temperature as Celsius (the default)
        float t = dht.readTemperature();
        // Read temperature as Fahrenheit (isFahrenheit = true)
        float f = dht.readTemperature(true);
      
        // Check if any reads failed and exit early (to try again).
        if (isnan(h) || isnan(t) || isnan(f)) {
          Serial.println(F("Failed to read from DHT sensor!"));
          return;
        }
      
        // Compute heat index in Fahrenheit (the default)
        float hif = dht.computeHeatIndex(f, h);
        // Compute heat index in Celsius (isFahreheit = false)
        float hic = dht.computeHeatIndex(t, h, false);
      
        Serial.print(F("Humidity: "));
        Serial.print(h);
        Serial.print(F("%  Temperature: "));
        Serial.print(t);
        Serial.print(F("°C "));
        Serial.print(f);
        Serial.print(F("°F  Heat index: "));
        Serial.print(hic);
        Serial.print(F("°C "));
        Serial.print(hif);
        Serial.println(F("°F"));

      
    }
    // save new integer every loop
      testfile = SD.open(fileName, FILE_WRITE);
      if (testfile) {
        // save a different number each loop
        testfile.println(String(millis())+","+String(int_iter));
        testfile.close();
        Serial.println("Saving "+String(int_iter));
      } else {
        Serial.println("error opening file");
      }
      int_iter+=1;

      
    delay(2000);
  

}
