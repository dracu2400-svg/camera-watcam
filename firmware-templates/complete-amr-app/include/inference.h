/**
 * @file inference.h
 * @brief ML inference module header
 */

#ifndef INFERENCE_H
#define INFERENCE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize TensorFlow Lite Micro inference engine
 *
 * @return 0 on success, -1 on failure
 */
int ml_init(void);

/**
 * @brief Run inference on a single digit image
 *
 * @param image Input image data (28x28 grayscale, values 0-255)
 * @param result Output predicted class (0-9 for digits)
 * @param confidence Output confidence score (0.0-1.0)
 * @return 0 on success, -1 on failure
 */
int ml_inference(const uint8_t* image, int* result, float* confidence);

/**
 * @brief Deinitialize ML inference engine
 */
void ml_deinit(void);

/**
 * @brief Get model information
 *
 * @param model_size Output model size in bytes
 * @param arena_used Output arena bytes used
 * @return 0 on success, -1 if not initialized
 */
int ml_get_info(uint32_t* model_size, uint32_t* arena_used);

#ifdef __cplusplus
}
#endif

#endif // INFERENCE_H
