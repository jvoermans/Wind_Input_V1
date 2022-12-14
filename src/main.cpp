#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <BMP388_DEV.h>                           // Include the BMP388_DEV.h library, pressure sensor
#include <Adafruit_ISM330DHCX.h>

// ETL
#define ETL_NO_STL
#include "Embedded_Template_Library.h"
#include "etl/vector.h"
etl::vector<float, 1000> pres1_vec;           //initialize pres1 vector

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

// SD Card
#include "SdFat.h"
#define SD_SPI_MHZ 12
#define SD_CS_PIN 8
File myFile;
SdFat SD;

int count = 0;

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

  // SD card
  if (!SD.begin(SD_CS_PIN, SD_SPI_MHZ)) {
    return;
  }
  myFile = SD.open("Data.txt", FILE_WRITE);

  delay(100);
}

void loop() {

  while (count<1000) {
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

  while (count==1000) {
  //print vector
    print_vector(pres1_vec);
    Serial.println(millis()); //check how long it takes to write vector to SD card
    write_vector(pres1_vec);
    Serial.println(millis());
    count++;
    myFile.close();
  }
}