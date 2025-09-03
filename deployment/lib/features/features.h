#ifndef FEATURES_H
#define FEATURES_H


#include <Arduino.h>
#include <arm_math.h>


#define NUM_CHANNELS 3

#define X_DATA_LEN 20
#define Y_DATA_LEN 20
#define Z_DATA_LEN 20

#define X_IDX 0
#define Y_IDX 1
#define Z_IDX 2

#define FEATURES_WIDTH    NUM_CHANNELS
#define FEATURES_HEIGHT   20
#define FEATURES_CHANNELS NUM_CHANNELS

#define FEATURE_R_CHANNEL 0
#define FEATURE_G_CHANNEL 1
#define FEATURE_B_CHANNEL 2

void extract_features(float32_t xdata[], float32_t ydata[], float32_t zdata[], int features[][NUM_CHANNELS][NUM_CHANNELS]);

#endif