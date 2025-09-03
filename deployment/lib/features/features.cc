
#include <features.h>

static void print_features(int features[][NUM_CHANNELS][NUM_CHANNELS]);

void extract_features(float32_t xdata[X_DATA_LEN], float32_t ydata[Y_DATA_LEN], float32_t zdata[Z_DATA_LEN], int features[][NUM_CHANNELS][NUM_CHANNELS]){

   /* Normalising the Data */
   float32_t xMin, xMax;
   float32_t yMin, yMax;
   float32_t zMin, zMax;

   // Finding the maximum and minimum values in the x axis
   arm_max_no_idx_f32(xdata, X_DATA_LEN, &xMax);
   arm_min_no_idx_f32(xdata, X_DATA_LEN, &xMin);

   // y axis
   arm_max_no_idx_f32(ydata, Y_DATA_LEN, &yMax);
   arm_min_no_idx_f32(ydata, Y_DATA_LEN, &yMin);
   // z axis
   arm_max_no_idx_f32(zdata, Z_DATA_LEN, &zMax);
   arm_min_no_idx_f32(zdata, Z_DATA_LEN, &zMin);
   
   float32_t x_norm[X_DATA_LEN], y_norm[Y_DATA_LEN], z_norm[Z_DATA_LEN];
   for(int i = 0; i < X_DATA_LEN; i++){
      x_norm[i] = ((xdata[i] - xMin)/(xMax - xMin)) * 255.0f;
      y_norm[i] = ((ydata[i] - yMin)/(yMax - yMin)) * 255.0f;
      z_norm[i] = ((zdata[i] - zMin)/(zMax - zMin)) * 255.0f; 
   }
   // /* Converting the accelerometer data into R, G, B values */
   int r_x[X_DATA_LEN], r_y[X_DATA_LEN], r_z[X_DATA_LEN];
   int g_x[Y_DATA_LEN], g_y[Y_DATA_LEN], g_z[Y_DATA_LEN];
   int b_x[Z_DATA_LEN], b_y[Z_DATA_LEN], b_z[Z_DATA_LEN];
   for (int i = 0; i < 20; i++) {
        // X-axis RGB conversion - exactly matching Python logic
        r_x[i] = (uint8_t)floorf(x_norm[i]);
        g_x[i] = (uint8_t)floorf((x_norm[i] - floorf(x_norm[i])) * 100.0f);
        float x_temp = x_norm[i] * 100.0f - floorf(x_norm[i] * 100.0f);
        b_x[i] = (uint8_t)floorf(x_temp * 100.0f);
        
        // Y-axis RGB conversion - exactly matching Python logic
        r_y[i] = (uint8_t)floorf(y_norm[i]);
        g_y[i] = (uint8_t)floorf((y_norm[i] - floorf(y_norm[i])) * 100.0f);
        float y_temp = y_norm[i] * 100.0f - floorf(y_norm[i] * 100.0f);
        b_y[i] = (uint8_t)floorf(y_temp * 100.0f);
        
        // Z-axis RGB conversion - exactly matching Python logic
        r_z[i] = (uint8_t)floorf(z_norm[i]);
        g_z[i] = (uint8_t)floorf((z_norm[i] - floorf(z_norm[i])) * 100.0f);
        float z_temp = z_norm[i] * 100.0f - floorf(z_norm[i] * 100.0f);
        b_z[i] = (uint8_t)floorf(z_temp * 100.0f);
    }
    
    // Stack into final RGB format: features[20][3][3]
    // Each sample has 3 channels (R, G, B), each with 3 values (x, y, z)
    
    // int rgbIdx = 0;
    // int lastIdx = 0;
    // for(int i = 0; i < 7; i++){
        
    //     for(int j = 0; j < FEATURES_WIDTH; j++){
            
    //         if(rgbIdx > 19){
    //             break;
    //         }
    //         features[i][j][X_IDX] = r_x[rgbIdx];
    //         features[i][j][Y_IDX] = r_y[rgbIdx];
    //         features[i][j][Z_IDX] = r_z[rgbIdx];
    //         rgbIdx++;
    //     }
    //     lastIdx = i;
    // }
    // rgbIdx = 0;
    // features[lastIdx][2][X_IDX] = g_x[rgbIdx];
    // features[lastIdx][2][Y_IDX] = g_y[rgbIdx];
    // features[lastIdx][2][Z_IDX] = g_z[rgbIdx];
    // lastIdx++;

    // rgbIdx = 1;
    // for(int i = lastIdx; i < (lastIdx+7); i++){
    //     for(int j = 0; j < FEATURES_WIDTH; j++){  
    //         if(rgbIdx > 19){
    //             break;
    //         }
    //         features[i][j][X_IDX] = g_x[rgbIdx];
    //         features[i][j][Y_IDX] = g_y[rgbIdx];
    //         features[i][j][Z_IDX] = g_z[rgbIdx];
    //         rgbIdx++;
    //     }
    //     lastIdx = i;  
    // }

    // Create temporary stacked array (3, 20, 3) like Python's np.stack
    uint8_t temp_stack[3][20][3];
    
    // Fill R matrix (temp_stack[0])
    for (int i = 0; i < 20; i++) {
        temp_stack[0][i][0] = r_x[i];
        temp_stack[0][i][1] = r_y[i]; 
        temp_stack[0][i][2] = r_z[i];
    }
    
    // Fill G matrix (temp_stack[1])
    for (int i = 0; i < 20; i++) {
        temp_stack[1][i][0] = g_x[i];
        temp_stack[1][i][1] = g_y[i];
        temp_stack[1][i][2] = g_z[i];
    }
    
    // Fill B matrix (temp_stack[2])
    for (int i = 0; i < 20; i++) {
        temp_stack[2][i][0] = b_x[i];
        temp_stack[2][i][1] = b_y[i];
        temp_stack[2][i][2] = b_z[i];
    }
    
    // Reshape from (3, 20, 3) to (20, 3, 3) - mimicking Python's reshape
    int flat_idx = 0;
    for (int rgb = 0; rgb < 3; rgb++) {        // R, G, B
        for (int sample = 0; sample < 20; sample++) {  // 20 samples
            for (int axis = 0; axis < 3; axis++) {     // x, y, z
                int target_sample = flat_idx / 9;
                int target_channel = (flat_idx % 9) / 3;
                int target_axis = flat_idx % 3;
                
                if (target_sample < 20) {
                    features[target_sample][target_channel][target_axis] = temp_stack[rgb][sample][axis];
                }
                flat_idx++;
            }
        }
    }
    
} 

static void print_features(int features[][NUM_CHANNELS][NUM_CHANNELS]){

    for (int i = 0; i < 20; i++) { // Print first 4 samples like your example
        printf("[[[%3d. %3d. %3d.]\n", 
               features[i][0][0], features[i][0][1], features[i][0][2]);
        printf("  [%3d. %3d. %3d.]\n", 
               features[i][1][0], features[i][1][1], features[i][1][2]);
        printf("  [%3d. %3d. %3d.]]\n\n", 
               features[i][2][0], features[i][2][1], features[i][2][2]);
    }
}