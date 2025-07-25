/* Edge Impulse ingestion SDK
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/* Includes ---------------------------------------------------------------- */
#include <harry-potter-wand_inferencing.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Defining the MPU6050 Handler
Adafruit_MPU6050 mpu;

/* Data collection parameters */
#define FREQUENCY_HZ 20
#define SAMPLING_PERIOD 1000 / FREQUENCY_HZ
#define NUM_SAMPLES 20

/* Features array parameters */
#define FEATURES_LENGTH 120U

static unsigned long last_interval_ms = 0;

static float features[FEATURES_LENGTH];

/**
 * @brief      Copy raw feature data in out_ptr
 *             Function called by inference library
 *
 * @param[in]  offset   The offset
 * @param[in]  length   The length
 * @param      out_ptr  The out pointer
 *
 * @return     0
 */
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
  memcpy(out_ptr, features + offset, length * sizeof(float));
  return 0;
}

void print_inference_result(ei_impulse_result_t result);
int find_max_index(int arr[], int length);

/**
 * @brief      Arduino setup function
 */
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // comment out the below line to cancel the wait for USB connection (needed for native USB)
  while (!Serial)
    ;

  pinMode(USER_BTN, INPUT_PULLUP);

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
}

/**
 * @brief      Arduino main function
 */
void loop() {

  unsigned long timestamp = 0; // Timestamp in ms


  uint32_t featureCounter = 0; // Index counter for the features array

  // Waiting for button press
  if (digitalRead(USER_BTN) == LOW) {

    // last_interval_ms = millis();

    // Collecting the number of samples required
    for (int i = 0; i < NUM_SAMPLES; i++) {
      
      // Current timestamp
      timestamp = millis();
      /* Get new sensor events with the readings */
      sensors_event_t a, g, temp;

      // Collecting the data
      mpu.getEvent(&a, &g, &temp);

      // Storing the data into the features array
      features[featureCounter++] = a.acceleration.x;
      features[featureCounter++] = a.acceleration.y;
      features[featureCounter++] = a.acceleration.z;
      features[featureCounter++] = g.gyro.x;
      features[featureCounter++] = g.gyro.y;
      features[featureCounter++] = g.gyro.z;

      while (millis() < timestamp + SAMPLING_PERIOD); // Waiting until 50 ms has passed
      
    }

    if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
      ei_printf("The size of your 'features' array is not correct. Expected %lu items, but had %lu\n",
                EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(features) / sizeof(float));
      delay(1000);
      return;
    }

    ei_impulse_result_t result = { 0 };

    // the features are stored into flash, and we don't want to load everything into RAM
    signal_t features_signal;
    features_signal.total_length = sizeof(features) / sizeof(features[0]);
    features_signal.get_data = &raw_feature_get_data;

    // invoke the impulse
    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false /* debug */);
    if (res != EI_IMPULSE_OK) {
      ei_printf("ERR: Failed to run classifier (%d)\n", res);
      return;
    }

    // print inference return code
    ei_printf("run_classifier returned: %d\r\n", res);
    print_inference_result(result);

    delay(1000);
  }
}

void print_inference_result(ei_impulse_result_t result) {

  // int tmpArray[5];
  // Print the prediction results (object detection)
#if EI_CLASSIFIER_OBJECT_DETECTION == 1
  ei_printf("Object detection bounding boxes:\r\n");
  for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
    ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
    if (bb.value == 0) {
      continue;
    }
    ei_printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
              bb.label,
              bb.value,
              bb.x,
              bb.y,
              bb.width,
              bb.height);
  }

  // Print the prediction results (classification)
#else
  // ei_printf("Predictions:\r\n");
  for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    // tmpArray[i] =  static_cast<int>(result.classification[i].value * 100);
    ei_printf("  %s: ", ei_classifier_inferencing_categories[i]);
    ei_printf("%d", static_cast<int>(result.classification[i].value * 100));
    
    if (i != EI_CLASSIFIER_LABEL_COUNT - 1) {
      ei_printf(", ");
    }
  }

  // int tmpIndex = find_max_index(tmpArray, 5U);
  // ei_printf(" %s", ei_classifier_inferencing_categories[tmpIndex]);
  // ei_printf("%d", static_cast<int>(result.classification[tmpIndex].value * 100));
#endif
  Serial.println();
  // Print anomaly result (if it exists)
#if EI_CLASSIFIER_HAS_ANOMALY
  ei_printf("Anomaly prediction: %.3f\r\n", result.anomaly);
#endif

#if EI_CLASSIFIER_HAS_VISUAL_ANOMALY
  ei_printf("Visual anomalies:\r\n");
  for (uint32_t i = 0; i < result.visual_ad_count; i++) {
    ei_impulse_result_bounding_box_t bb = result.visual_ad_grid_cells[i];
    if (bb.value == 0) {
      continue;
    }
    ei_printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
              bb.label,
              bb.value,
              bb.x,
              bb.y,
              bb.width,
              bb.height);
  }
#endif
}

int find_max_index(int arr[], int length) {
    if (length <= 0) {
        return -1; // Return -1 for invalid input
    }
    
    int max_index = 0;
    int max_value = arr[0];
    
    for (int i = 1; i < length; i++) {
        if (arr[i] > max_value) {
            max_value = arr[i];
            max_index = i;
        }
    }
    
    return max_index;
}
