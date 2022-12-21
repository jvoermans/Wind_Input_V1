#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <BMP388_DEV.h>                           // Include the BMP388_DEV.h library, pressure sensor
#include <Adafruit_ISM330DHCX.h>
#include <Adafruit_GPS.h>

// ETL
#define ETL_NO_STL
#include "Embedded_Template_Library.h"
#include "etl/vector.h"
etl::vector<float, 100> pres1_vec;           //initialize pres1 vector

//GPS
#define GPSSerial Serial1
Adafruit_GPS GPS(&GPSSerial);
#define GPSECHO false

//Pressure sensor
float temperature1, pressure1, altitude1;            // Create the temperature, pressure and altitude variables
float temperature2, pressure2, altitude2;            // Create the temperature, pressure and altitude variables
BMP388_DEV bmp1;                                // First pressure sensor
BMP388_DEV bmp2;                                // Second pressure sensor

// IMU
Adafruit_ISM330DHCX ism330dhcx;

//Timer
unsigned long currentMillis;
unsigned long previousMillis;
const unsigned long dT = 50;        //sampling frequency f=1/dT
uint32_t timer = millis();

// SD Card
#include "SdFat.h"
#define SD_SPI_MHZ 12
#define SD_CS_PIN 8
File myFile;
SdFat SD;

int count = 0;
int count2 = 0;

//Print vector to Serial
template <typename T>
void print_vector(etl::ivector<T> const & vec_in)
{
  Serial.println(F("print vector"));
  for (T const & elem : vec_in) 
  {
    Serial.print(elem);
    Serial.print(F(" | "));
  }
  Serial.println();
}

//write vector to SD
template <typename T>
void write_vector(etl::ivector<T> const & vec_in)
{
  myFile.println(F("print vector"));
  for (T const & elem : vec_in) 
  {
    myFile.println(elem);
  }
}

void setup() {
  Serial.begin(1000000);

  //Initialize Pressure sensors
  bmp1.begin(0x77);
  bmp1.setTimeStandby(TIME_STANDBY_40MS);     // minimum is 40MS with 16x oversampling pressure
  bmp1.setPresOversampling(OVERSAMPLING_X16);
  bmp1.setTempOversampling(OVERSAMPLING_X2);
  bmp1.setIIRFilter(IIR_FILTER_OFF);             // Turn off, we are interested in fluctuations
  bmp1.startNormalConversion();                 // Start BMP388 continuous conversion in NORMAL_MODE

  bmp2.begin(0x76);
  bmp2.setTimeStandby(TIME_STANDBY_40MS);
  bmp2.setPresOversampling(OVERSAMPLING_X16);
  bmp2.setTempOversampling(OVERSAMPLING_X2);
  bmp2.setIIRFilter(IIR_FILTER_OFF);
  bmp2.startNormalConversion();

  //Initialize IMU
  if (!ism330dhcx.begin_I2C(0x6A)) {
    Serial.println("Failed to find ISM330DHCX chip");
    while (1) {
      delay(10);
    }
  }
  ism330dhcx.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
  ism330dhcx.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);
  ism330dhcx.setAccelDataRate(LSM6DS_RATE_104_HZ);
  ism330dhcx.setGyroDataRate(LSM6DS_RATE_104_HZ);
  ism330dhcx.configInt1(false, false, true); // accelerometer DRDY on INT1
  ism330dhcx.configInt2(false, true, false); // gyro DRDY on INT2

  // GPS
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

  // SD card
  if (!SD.begin(SD_CS_PIN, SD_SPI_MHZ)) {
    return;
  }
  myFile = SD.open("Data.txt", FILE_WRITE);

  delay(100);
}

void loop() {

  while (count<100) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= dT)
    { previousMillis = currentMillis; //update timer

      bmp1.getMeasurements(temperature1, pressure1, altitude1);
      bmp2.getMeasurements(temperature2, pressure2, altitude2);

      sensors_event_t accel;
      sensors_event_t gyro;
      sensors_event_t temp;
      ism330dhcx.getEvent(&accel, &gyro, &temp);

      pres1_vec.push_back(pressure1);

      Serial.print(millis());
      Serial.print(" ");
      Serial.print(accel.acceleration.x);
      Serial.print(" ");
      Serial.print(accel.acceleration.y);
      Serial.print(" ");
      Serial.print(accel.acceleration.z);
      Serial.print(" ");
      Serial.print(gyro.gyro.x);
      Serial.print(" ");
      Serial.print(gyro.gyro.y);
      Serial.print(" ");
      Serial.print(gyro.gyro.z);
      Serial.print(" ");
      Serial.print(pressure1,4);  //print pressure in hPa
      Serial.print(" ");
      Serial.println(pressure2,4);  //print pressure in hPa
      count++;
    }
  }

  while (count==100) {
  //print vector
    print_vector(pres1_vec);
    Serial.println(millis()); //check how long it takes to write vector to SD card
    write_vector(pres1_vec);
    Serial.println(millis());
    count++;
    myFile.close();
  }

  // read data from the GPS in the 'main loop'
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  if (GPSECHO)
    if (c) Serial.print(c);
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trying to print out data
    Serial.println(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) {
    timer = millis(); // reset the timer
    Serial.print("\nTime: ");
    if (GPS.hour < 10) { Serial.print('0'); }
    Serial.print(GPS.hour, DEC); Serial.print(':');
    if (GPS.minute < 10) { Serial.print('0'); }
    Serial.print(GPS.minute, DEC); Serial.print(':');
    if (GPS.seconds < 10) { Serial.print('0'); }
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    if (GPS.milliseconds < 10) {
      Serial.print("00");
    } else if (GPS.milliseconds > 9 && GPS.milliseconds < 100) {
      Serial.print("0");
    }
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
    Serial.print("Location: ");
    Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
    Serial.print(", ");
    Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
  }

}