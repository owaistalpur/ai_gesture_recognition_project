#ifndef MPU6050_H
#define MPU6050_H

/* Includes */
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <features.h>

/* Data Collection Parameters */
#define FREQUENCY_HZ        20
#define SAMPLING_PERIOD     1000 / FREQUENCY_HZ
#define NUM_SAMPLES         20

enum MPU_STATUS{

   DATA_NOT_COLLECTED = 0,
   DATA_COLLECTED
};

static unsigned long last_interval_ms = 0; // Keeping track of the last reading time

void mpu6050_setup(void);
int read_mpu6050_data(float32_t xData[], float32_t yData[], float32_t zData[]);

#endif