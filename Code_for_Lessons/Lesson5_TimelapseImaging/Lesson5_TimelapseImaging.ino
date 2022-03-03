// S. Streeter
// February-March 2022
//
// Based on ArduCAM Mini demo (C)2017 Lee
//
// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM_Mini_5MP_Plus
// and use Arduino IDE 1.6.8 compiler or above
// This sketch is based on the example "ArduCAM_Mini_5MP_Plus_Multi_Capture2SD.ino" found here:
// https://github.com/ArduCAM/Arduino/tree/master/ArduCAM/examples/mini/ArduCAM_Mini_5MP_Plus_Multi_Capture2SD
//
// Version (v) 3 of this sketch includes real time clock (RTC) code for pulling the date and time
// from the RTC built into the HiLetgo data logger AND includes code to take a photo at the top of every hour
// instead of every, e.g., 60 minutes, which results in "drifting" timestamps, because they don't align
// perfectly with the hour.

//----------------------------------------------------------------------------------------

#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <SD.h>
#include "memorysaver.h"
#include "RTClib.h"

// Set whether currently observing Daylight Savings Time (DST)
// DST is observed the second Sunday of March through the first
// Sunday of November. If not currently in DST, set = 0.
int DST = 1;

// This demo can only work with the OV5642_MINI_5MP_PLUS platform
#if !(defined (OV5642_MINI_5MP_PLUS))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif

//You can change the FRAMES_NUM count to change the number of the picture
#define FRAMES_NUM 0x00

// Set digital pin 7 as the slave select for the digital pot
const int CS = 7;

// Set digital pin 10 for the SD card (see p. 20 of https://cdn-learn.adafruit.com/downloads/pdf/adafruit-data-logger-shield.pdf)
#define SD_CS 10

// Set the resolution of the saved images
//uint8_t resolution = OV5642_320x240;
uint8_t resolution = OV5642_2592x1944;

//----------------------------------------------------------------------------------------

// Buffer flag and timer
bool is_header = false;
unsigned long total_time = 0;

// Setup Arducam
ArduCAM myCAM(OV5642, CS);
uint8_t read_fifo_burst(ArduCAM myCAM);

// Define RTC class object
RTC_DS3231 rtc;

// Days of the week for serial monitor timestamp
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//----------------------------------------------------------------------------------------

void setup() {
  
  uint8_t vid, pid;
  uint8_t temp;
  
  #if defined(__SAM3X8E__)
    Wire1.begin();
  #else
    Wire.begin();
  #endif
  Serial.begin(115200);
  Serial.println(F("ArduCAM Start!"));
  
  // Set the CS pin as an output
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  
  // Itialize SPI connection
  SPI.begin();
  //Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(100); 
  
  // Check if the ArduCAM SPI bus is OK
  while(1){
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if(temp != 0x55)
    {
      Serial.println(F("SPI interface Error!"));
      delay(1000);continue;
    }else{
      Serial.println(F("SPI interface OK."));break;
    }
  }
  
  // Confirm that the the camera module type is OV5642
  while(1){
    myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x42)){
      Serial.println(F("Can't find OV5642 module!"));
      delay(1000);continue;
    }else{
      Serial.println(F("OV5642 detected."));delay(1000);break;      
    }
  }
  
  // Initialize SD Card
  while(!SD.begin(SD_CS))
  {
    Serial.println(F("SD Card Error!"));
    delay(1000);
  }
  Serial.println(F("SD Card detected."));
  
  // Set image format
  myCAM.set_format(JPEG);
  //myCAM.set_format(RAW);
  
  // Initialize camera
  myCAM.InitCAM();
  myCAM.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
  myCAM.clear_fifo_flag();
  myCAM.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);

  // Set up the RTC
  if (! rtc.begin()) {
  Serial.println("Couldn't find RTC!");
  Serial.flush();
  while (1) delay(10);
  }
  
  Serial.println("RTC DS3231 module detected.");
  Serial.print("Current date and time: ");
  DateTime currentDateTime = rtc.now();
  Serial.print(currentDateTime.year(), DEC);
  Serial.print('/');
  Serial.print(currentDateTime.month(), DEC);
  Serial.print('/');
  Serial.print(currentDateTime.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[currentDateTime.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(currentDateTime.hour(), DEC);
  Serial.print(':');
  Serial.print(currentDateTime.minute(), DEC);
  Serial.print(':');
  Serial.print(currentDateTime.second(), DEC);
  if (DST == 1) {
    Serial.println(" Eastern Daylight Savings Time.");
  } else {
    Serial.println(" Eastern Standard Time.");
  }
  
}

//----------------------------------------------------------------------------------------

// Enter main processing loop
void loop() {

  DateTime currentDateTime = rtc.now();

  // NOTE: Adjust if statement immediately below depending on how often you want an image.
  //if (currentDateTime.hour() == 0) { // This is one image per day (at midnight)
  //if (currentDateTime.minute() == 0) { // This is one image per hour (when a new hour starts)
  //if (currentDateTime.second() == 0) { // This is one image per minute
  if (currentDateTime.second() % 15 == 0) { // This is one image every 15 seconds
  
    // First in, first out (FIFO) buffer
    myCAM.flush_fifo();
    myCAM.clear_fifo_flag();
    
    // Set image resolution
    myCAM.OV5642_set_JPEG_size(resolution);
    //myCAM.OV5642_set_RAW_size(resolution);
    delay(1000);  
    
    // Start/stop capture
    myCAM.start_capture();
    Serial.println(F("Start capture."));
    total_time = millis();
    while ( !myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)); 
    Serial.println(F("Stop capture."));
    total_time = millis() - total_time;
    Serial.print(F("Capture total time used (ms): "));
    Serial.print(total_time, DEC);
    Serial.println(F("."));
    total_time = millis();
    read_fifo_burst(myCAM);
    total_time = millis() - total_time;
    Serial.print(F("Save capture total time used (ms): "));
    Serial.print(total_time, DEC);
    Serial.println(F("."));
    
    // Timestamp for image
    Serial.print("Image taken: ");
    DateTime currentDateTime = rtc.now();
    Serial.print(currentDateTime.year(), DEC);
    Serial.print('/');
    Serial.print(currentDateTime.month(), DEC);
    Serial.print('/');
    Serial.print(currentDateTime.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[currentDateTime.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(currentDateTime.hour(), DEC);
    Serial.print(':');
    Serial.print(currentDateTime.minute(), DEC);
    Serial.print(':');
    Serial.print(currentDateTime.second(), DEC);
    if (DST == 1) {
      Serial.println(" Eastern Daylight Savings Time.");
    } else {
      Serial.println(" Eastern Standard Time.");
    }
    
    // Clear the capture done flag
    myCAM.clear_fifo_flag();
    
  }
}

//----------------------------------------------------------------------------------------

// Function called in main loop above
uint8_t read_fifo_burst(ArduCAM myCAM)
{
  
  uint8_t temp = 0, temp_last = 0;
  uint32_t length = 0;
  static int i = 0;
  static int k = 0;
  // char str[8];
  File outFile;
  byte buf[256]; 
  length = myCAM.read_fifo_length();
  Serial.print(F("Fifo length (bytes): "));
  Serial.print(length, DEC);
  Serial.println(F("."));
  if (length >= MAX_FIFO_SIZE) //8M
  {
    Serial.println("Over 8M in size!");
    return 0;
  }
  if (length == 0 ) //0 kb
  {
    Serial.println(F("Size is 0."));
    return 0;
  } 
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();//Set fifo burst mode
  i = 0;
  while ( length-- )
  {
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    // Read JPEG data from FIFO
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
    {
      buf[i++] = temp;  //save the last  0XD9     
      
      // Write the remain bytes in the buffer
      myCAM.CS_HIGH();
      outFile.write(buf, i);    
      
      // Close the file
      outFile.close();
      Serial.println(F("Done."));
      is_header = false;
      myCAM.CS_LOW();
      myCAM.set_fifo_burst();
      i = 0;
      
    }  
    if (is_header == true)
    { 
      
      // Write image data to buffer if not full
      if (i < 256)
       buf[i++] = temp;
      else
      {
        // Write 256 bytes image data to file
        myCAM.CS_HIGH();
        outFile.write(buf, 256);
        i = 0;
        buf[i++] = temp;
        myCAM.CS_LOW();
        myCAM.set_fifo_burst();
      }        
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      is_header = true;
      myCAM.CS_HIGH();

      // Create unique image filename
      DateTime datetimenow = rtc.now();
      // UTC
      long timeStamp = datetimenow.unixtime();
      if (DST == 1) {
        // Convert to Eastern Standard Time (+5 hours)
        timeStamp = timeStamp + 18000;         
      } else {
        // Convert to Eastern Daylight Savings Time (+4 hours)
        timeStamp = timeStamp + 14400;         
      }
      timeStamp = timeStamp%100000000;
      String fileName = String(timeStamp) + ".jpg";
      Serial.print("Image filename: ");
      Serial.print(fileName);
      Serial.println(".");
      Serial.print(F("Saving image..."));

      // Open the new file
      outFile = SD.open(fileName, O_WRITE | O_CREAT | O_TRUNC);
      if (! outFile)
      {
        Serial.println(F("File open failed"));
        while (1);
      }
      myCAM.CS_LOW();
      myCAM.set_fifo_burst();   
      buf[i++] = temp_last;
      buf[i++] = temp;   
    }
  }
  myCAM.CS_HIGH();
  return 1;
  
}
