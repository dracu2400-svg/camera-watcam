/**
 * @file digit_detection.h
 * @brief Digit detection and ROI extraction module header
 */

#ifndef DIGIT_DETECTION_H
#define DIGIT_DETECTION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Extract digit ROI from full image and resize to 28x28
 *
 * @param image Full captured image (640x480)
 * @param digit_index Index of digit to extract (0-7)
 * @param output Output 28x28 grayscale image
 * @return 0 on success, -1 on failure
 */
int extract_digit_roi(const uint8_t* image, uint8_t digit_index, uint8_t* output);

/**
 * @brief Configure digit ROI positions
 *
 * @param digit_index Index of digit to configure (0-7)
 * @param x Top-left X coordinate
 * @param y Top-left Y coordinate
 * @param width ROI width
 * @param height ROI height
 * @return 0 on success, -1 on failure
 */
int configure_digit_roi(uint8_t digit_index, uint16_t x, uint16_t y,
                       uint16_t width, uint16_t height);

/**
 * @brief Apply preprocessing to digit image
 *
 * @param digit 28x28 digit image (modified in-place)
 * @return 0 on success, -1 on failure
 */
int preprocess_digit(uint8_t* digit);

/**
 * @brief Get digit ROI configuration
 *
 * @param digit_index Index of digit (0-7)
 * @param x Output X coordinate
 * @param y Output Y coordinate
 * @param width Output width
 * @param height Output height
 * @return 0 on success, -1 on failure
 */
int get_digit_roi_config(uint8_t digit_index, uint16_t* x, uint16_t* y,
                        uint16_t* width, uint16_t* height);

/**
 * @brief Validate image quality for digit recognition
 *
 * @param image Full image (640x480)
 * @return Quality score 0-100, or -1 on error
 */
int8_t validate_image_quality(const uint8_t* image);

#ifdef __cplusplus
}
#endif

#endif // DIGIT_DETECTION_H
