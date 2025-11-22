/**
 * @file inference.cpp
 * @brief ML inference using TensorFlow Lite Micro
 *
 * This module handles loading and running the digit recognition model
 * using TensorFlow Lite for Microcontrollers on the Cortex-M55 + Ethos-U55 NPU.
 */

#include <stdio.h>
#include <stdint.h>
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"

// Include your trained model (will be copied here during build)
// This header file contains the model data as a const unsigned char array
#include "water_meter_digit_model.h"

// Tensor arena size - this memory is used for TFLite internal operations
// Adjust if you get allocation errors or want to optimize memory usage
constexpr int kTensorArenaSize = 100 * 1024;  // 100KB
static uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16)));

// Global pointers for TFLite objects
static const tflite::Model* model = nullptr;
static tflite::MicroInterpreter* interpreter = nullptr;
static TfLiteTensor* input_tensor = nullptr;
static TfLiteTensor* output_tensor = nullptr;

// Error reporter
static tflite::MicroErrorReporter micro_error_reporter;
static tflite::ErrorReporter* error_reporter = &micro_error_reporter;

// Extern "C" block to make functions callable from C code
extern "C" {

/**
 * @brief Initialize TensorFlow Lite Micro inference engine
 *
 * This function:
 * 1. Loads the model from flash memory
 * 2. Sets up the operation resolver with required ops
 * 3. Creates the interpreter
 * 4. Allocates tensors
 * 5. Gets pointers to input/output tensors
 *
 * @return 0 on success, -1 on failure
 */
int ml_init(void)
{
    printf("Initializing TensorFlow Lite Micro...\n");

    // Load model from flash memory
    model = tflite::GetModel(digit_recognition_model);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        printf("ERROR: Model schema version %lu != TFLite schema %d\n",
               model->version(), TFLITE_SCHEMA_VERSION);
        return -1;
    }
    printf("  Model schema version: %lu (matches TFLite schema %d)\n",
           model->version(), TFLITE_SCHEMA_VERSION);

    // Setup operation resolver
    // Add only the operations your model uses (keeps binary size small)
    // Common ops for CNN models:
    static tflite::MicroMutableOpResolver<6> micro_op_resolver(error_reporter);
    micro_op_resolver.AddConv2D();           // Convolution layers
    micro_op_resolver.AddMaxPool2D();        // Max pooling layers
    micro_op_resolver.AddReshape();          // Reshape operations
    micro_op_resolver.AddFullyConnected();   // Fully connected (dense) layers
    micro_op_resolver.AddSoftmax();          // Softmax activation for output
    micro_op_resolver.AddQuantize();         // Quantization operations

    printf("  Registered %d operations\n", 6);

    // Build interpreter
    static tflite::MicroInterpreter static_interpreter(
        model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    // Allocate tensors from the tensor arena
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        printf("ERROR: AllocateTensors() failed\n");
        return -1;
    }

    // Get pointers to input and output tensors
    input_tensor = interpreter->input(0);
    output_tensor = interpreter->output(0);

    // Print model information
    printf("✓ TFLite initialized successfully\n");
    printf("  Model size: %u bytes\n", digit_recognition_model_len);
    printf("  Input shape: %d x %d x %d\n",
           input_tensor->dims->data[1],
           input_tensor->dims->data[2],
           input_tensor->dims->data[3]);
    printf("  Input type: %s\n",
           input_tensor->type == kTfLiteInt8 ? "INT8" :
           input_tensor->type == kTfLiteFloat32 ? "FLOAT32" : "UNKNOWN");
    printf("  Output shape: %d classes\n",
           output_tensor->dims->data[1]);
    printf("  Output type: %s\n",
           output_tensor->type == kTfLiteInt8 ? "INT8" :
           output_tensor->type == kTfLiteFloat32 ? "FLOAT32" : "UNKNOWN");
    printf("  Tensor arena: %zu / %d bytes used (%.1f%%)\n",
           interpreter->arena_used_bytes(),
           kTensorArenaSize,
           (interpreter->arena_used_bytes() * 100.0f) / kTensorArenaSize);

    return 0;
}

/**
 * @brief Run inference on a single digit image
 *
 * This function:
 * 1. Converts input image to correct format (INT8)
 * 2. Copies data to input tensor
 * 3. Runs inference
 * 4. Extracts predicted class and confidence
 *
 * @param image Input image data (28x28 grayscale, values 0-255)
 * @param result Output predicted class (0-9 for digits)
 * @param confidence Output confidence score (0.0-1.0)
 * @return 0 on success, -1 on failure
 */
int ml_inference(const uint8_t* image, int* result, float* confidence)
{
    if (image == NULL || result == NULL || confidence == NULL) {
        printf("ERROR: Invalid input parameters\n");
        return -1;
    }

    if (input_tensor == NULL || output_tensor == NULL) {
        printf("ERROR: Model not initialized\n");
        return -1;
    }

    // Copy image to input tensor
    // Convert from uint8 [0, 255] to int8 [-128, 127] if model expects INT8
    if (input_tensor->type == kTfLiteInt8) {
        int8_t* input_data = input_tensor->data.int8;
        for (int i = 0; i < 28 * 28; i++) {
            input_data[i] = (int8_t)(image[i] - 128);
        }
    } else if (input_tensor->type == kTfLiteFloat32) {
        float* input_data = input_tensor->data.f;
        for (int i = 0; i < 28 * 28; i++) {
            input_data[i] = image[i] / 255.0f;
        }
    }

    // Run inference with timing
    uint32_t start_time = hx_drv_timer_get_ms();

    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        printf("ERROR: Invoke() failed\n");
        return -1;
    }

    uint32_t end_time = hx_drv_timer_get_ms();
    uint32_t inference_time = end_time - start_time;

    // Get output and find class with highest score
    int max_idx = 0;

    if (output_tensor->type == kTfLiteInt8) {
        int8_t* output_data = output_tensor->data.int8;
        int8_t max_val = output_data[0];

        for (int i = 1; i < 10; i++) {
            if (output_data[i] > max_val) {
                max_val = output_data[i];
                max_idx = i;
            }
        }

        // Convert INT8 score to confidence [0.0, 1.0]
        *confidence = (max_val + 128) / 255.0f;
    } else if (output_tensor->type == kTfLiteFloat32) {
        float* output_data = output_tensor->data.f;
        float max_val = output_data[0];

        for (int i = 1; i < 10; i++) {
            if (output_data[i] > max_val) {
                max_val = output_data[i];
                max_idx = i;
            }
        }

        *confidence = max_val;
    }

    *result = max_idx;

    // Print inference details
    printf("  Inference time: %lu ms\n", inference_time);
    printf("  Predicted class: %d (confidence: %.2f%%)\n",
           *result, *confidence * 100.0f);

    return 0;
}

/**
 * @brief Deinitialize ML inference engine
 *
 * For TFLite Micro, there's no dynamic memory to free,
 * so this is mostly a placeholder for symmetry.
 */
void ml_deinit(void)
{
    printf("ML deinitialized\n");
    // TFLite Micro uses static memory, nothing to free
}

/**
 * @brief Get model information
 *
 * @param model_size Output model size in bytes
 * @param arena_used Output arena bytes used
 * @return 0 on success, -1 if not initialized
 */
int ml_get_info(uint32_t* model_size, uint32_t* arena_used)
{
    if (interpreter == NULL) {
        return -1;
    }

    if (model_size != NULL) {
        *model_size = digit_recognition_model_len;
    }

    if (arena_used != NULL) {
        *arena_used = interpreter->arena_used_bytes();
    }

    return 0;
}

} // extern "C"
