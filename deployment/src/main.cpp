#include <mpu6050.h>
#include <features.h>
#include <magic_wand_model.h>

#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Declaring the tflite variables
namespace {
  
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model * model = nullptr;
  tflite::MicroInterpreter * interpreter = nullptr;
  TfLiteTensor * model_input = nullptr;

  constexpr int labelCount = 5;
  const char * labels[labelCount] = {
    "aguamenti",
    "expelliarums",
    "incendio",
    "reparo",
    "serpensortia"
  };
  
  constexpr int kTensorArenaSize = 12 * 1024;
  uint8_t tensor_arena[kTensorArenaSize];

}

void setup(void) {

  // USR Button setup
  pinMode(USER_BTN, INPUT_PULLUP);
  
  // Setting up the serial monitor
  Serial.begin(115200);

  while (!Serial)
    delay(10);

  // Setting up the MPU6050
  mpu6050_setup();  
  
  // Setting up the logging and model
  static tflite::MicroErrorReporter microErrorReporter;
  error_reporter = &microErrorReporter;

  model = tflite::GetModel(magic_wand_quant_tflite);
  if(model->version() != TFLITE_SCHEMA_VERSION){
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // Getting the required ops
  static tflite::MicroMutableOpResolver<6> micro_op_resolver;

  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddAveragePool2D();
  micro_op_resolver.AddReshape();
  micro_op_resolver.AddFullyConnected();
  micro_op_resolver.AddSoftmax();
  micro_op_resolver.AddMean();

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize);

  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  interpreter->AllocateTensors();
  // Set model input settings
  model_input = interpreter->input(0);

  if((model_input->dims->size != 4)       || (model_input->dims->data[0] != 1) || 
     (model_input->dims->data[1] != 20)   || (model_input->dims->data[2] != 3) ||
     (model_input->dims->data[2] != 3)    || (model_input->type != kTfLiteInt8)){
      TF_LITE_REPORT_ERROR(error_reporter,
                         "Bad input tensor parameters in model");
    }
  
  
   // Set model output settings
  TfLiteTensor* model_output = interpreter->output(0);
  if ((model_output->dims->size != 2) || (model_output->dims->data[0] != 1) ||
      (model_output->dims->data[1] != labelCount) ||
      (model_output->type != kTfLiteInt8)) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Bad output tensor parameters in model");
    return;
  }
  
}

void loop() {


  if(digitalRead(USER_BTN) == LOW){
    delay(50);
    int features[FEATURES_HEIGHT][FEATURES_WIDTH][FEATURES_CHANNELS];
    
    // Gathering data from the accelerometer
    float32_t xData[X_DATA_LEN], yData[X_DATA_LEN], zData[Z_DATA_LEN];
    read_mpu6050_data(xData, yData, zData);

      // Extracting the features from the accelerometer data
      extract_features(xData, yData, zData, features);
      int8_t tmpData = 0;
      // Pass to the model and run the interpreter
      int dataInputIDX = 0;
      TfLiteTensor* model_input = interpreter->input(0);
        for (int i = 0; i < 20; i++) {        // samples
            for (int j = 0; j < 3; j++) {     // channels (R, G, B)  
                for (int k = 0; k < 3; k++) { // axes (x, y, z)
                  
                  tmpData = features[i][j][k];
                  model_input->data.int8[dataInputIDX++] = (int8_t)(tmpData/model_input->params.scale + model_input->params.zero_point);
                }
            }
        }

        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk) {
          TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed");
          return;
        }
        TfLiteTensor* output = interpreter->output(0);
      
        printf("Predictions: \n\n");
        for (int i = 0; i < labelCount; i++) {
          float valf32  = (output->data.int8[i] - output->params.zero_point) * output->params.scale;
          printf("\t%s: %0.6f\n", labels[i], valf32);
        }
    }

}
