#include <mpu6050.h>

// MPU6050 Handler
Adafruit_MPU6050 mpu;

// Static function definition
static void print_mpu6050_values(sensors_event_t a, sensors_event_t g, unsigned long last_ms, unsigned long timestamp);

void mpu6050_setup(void){

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
static void print_mpu6050_values(sensors_event_t a, sensors_event_t g, unsigned long last_ms, unsigned long timestamp){

   Serial.print(timestamp - last_interval_ms); Serial.print(", ");
   Serial.print(a.acceleration.x); Serial.print(", ");
   Serial.print(a.acceleration.y); Serial.print(", ");
   Serial.println(a.acceleration.z);


}
void read_mpu6050_data(float32_t xData[], float32_t yData[], float32_t zData[]){

   
      sensors_event_t a, g, temp;
      last_interval_ms = millis();
      unsigned long timestamp = 0;
      for(int i = 0; i < NUM_SAMPLES; i++){
         timestamp = millis();
         mpu.getEvent(&a, &g, &temp);
         xData[i] = a.acceleration.x;
         yData[i] = a.acceleration.y;
         zData[i] = a.acceleration.z;
         
         while(millis() < timestamp + SAMPLING_PERIOD);  
      }
      
}
