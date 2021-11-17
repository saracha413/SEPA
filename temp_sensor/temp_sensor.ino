#include <OneWire.h>

#include <DallasTemperature.h>

//copied from https://create.arduino.cc/projecthub/iotboys/how-to-use-ds18b20-water-proof-temperature-sensor-2adecc

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS1 5
#define ONE_WIRE_BUS2 6

OneWire oneWire1(ONE_WIRE_BUS1);
OneWire oneWire2(ONE_WIRE_BUS2);

DallasTemperature sensors1(&oneWire1);

DallasTemperature sensors2(&oneWire2);

 float Celcius1=0;
 float Fahrenheit1=0;

 float Celcius2=0;
 float Fahrenheit2=0;
void setup(void)
{
  
  Serial.begin(9600);
  sensors1.begin();
  sensors2.begin();
}

void loop(void)
{ 
  sensors1.requestTemperatures(); 
  Celcius1=sensors1.getTempCByIndex(0);
  Fahrenheit1=sensors1.toFahrenheit(Celcius1);
  //Serial.print(" C  ");
  //Serial.print(Celcius);
  Serial.print(" Sensor 1");
  Serial.print(" F  ");
  Serial.println(Fahrenheit1);

  sensors2.requestTemperatures(); 
  Celcius2=sensors2.getTempCByIndex(0);
  Fahrenheit2=sensors2.toFahrenheit(Celcius2);
  Serial.print(" Sensor 2");
  Serial.print(" F  ");
  Serial.println(Fahrenheit2);
 
  delay(1000);
}
