/**
 * @file Data_collection.ino
 * @author Owais Talpur (owaistalpur@hotmail.com)
 * @brief This arduino script is used for the data collection stage of my gesture recognition project
 * @version 0.1
 * @date 2025-07-14
 * 
 * @copyright Copyright (c) 2025
 * 
 **/

 /* Includes */
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
// Defining the MPU6050 Handler
Adafruit_MPU6050 mpu;

/* Data collection parameters */
#define FREQUENCY_HZ        20
#define SAMPLING_PERIOD     1000 / FREQUENCY_HZ
#define NUM_SAMPLES         20

static unsigned long last_interval_ms = 0;

void setup(void) {

  pinMode(USER_BTN, INPUT_PULLUP);
  
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  // Setting the MPU range and filter bandwidth
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(100);
}

void loop() {


  unsigned long timestamp = 0;

  if(digitalRead(USER_BTN) == LOW){
    delay(200);

    last_interval_ms = millis(); 
    for(int i = 0; i < NUM_SAMPLES; i++){

      timestamp = millis();
            /* Get new sensor events with the readings */
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      /* Print out the values */
      
      
      Serial.print(timestamp - last_interval_ms); Serial.print(", ");
      Serial.print(a.acceleration.x); Serial.print(", ");
      Serial.print(a.acceleration.y); Serial.print(", ");
      Serial.print(a.acceleration.z); Serial.print(", ");
      Serial.print(g.gyro.x);         Serial.print(", ");
      Serial.print(g.gyro.y);         Serial.print(", ");
      Serial.println(g.gyro.z); 
      
      while(millis() < timestamp + SAMPLING_PERIOD);  
    
    }

   Serial.println("End Data Collection");
  }

  
}
