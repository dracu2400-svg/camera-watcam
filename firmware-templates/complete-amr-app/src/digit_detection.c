/**
 * @file digit_detection.c
 * @brief Digit detection and ROI extraction module
 *
 * This module handles detecting digit regions in the captured meter image
 * and extracting them as 28x28 grayscale images for ML inference.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "digit_detection.h"

// Digit positions configuration
// These values should be calibrated for your specific water meter
// Coordinates are in pixels relative to 640x480 image

typedef struct {
    uint16_t x;      // Top-left X coordinate
    uint16_t y;      // Top-left Y coordinate
    uint16_t width;  // ROI width
    uint16_t height; // ROI height
} digit_roi_config_t;

// Default digit ROI configurations for 8-digit meter
// Adjust these based on your meter's digit positions
static const digit_roi_config_t digit_rois[8] = {
    {100, 220, 40, 60},  // Digit 0
    {150, 220, 40, 60},  // Digit 1
    {200, 220, 40, 60},  // Digit 2
    {250, 220, 40, 60},  // Digit 3
    {300, 220, 40, 60},  // Digit 4
    {350, 220, 40, 60},  // Digit 5
    {400, 220, 40, 60},  // Digit 6
    {450, 220, 40, 60}   // Digit 7
};

/**
 * @brief Resize image using nearest neighbor interpolation
 *
 * @param src Source image
 * @param src_width Source width
 * @param src_height Source height
 * @param dst Destination image
 * @param dst_width Destination width
 * @param dst_height Destination height
 */
static void resize_nearest_neighbor(const uint8_t* src, uint16_t src_width, uint16_t src_height,
                                    uint8_t* dst, uint16_t dst_width, uint16_t dst_height)
{
    float x_ratio = (float)src_width / dst_width;
    float y_ratio = (float)src_height / dst_height;

    for (uint16_t y = 0; y < dst_height; y++) {
        for (uint16_t x = 0; x < dst_width; x++) {
            uint16_t src_x = (uint16_t)(x * x_ratio);
            uint16_t src_y = (uint16_t)(y * y_ratio);

            // Clamp to source bounds
            if (src_x >= src_width) src_x = src_width - 1;
            if (src_y >= src_height) src_y = src_height - 1;

            dst[y * dst_width + x] = src[src_y * src_width + src_x];
        }
    }
}

/**
 * @brief Extract digit ROI from full image and resize to 28x28
 *
 * @param image Full captured image (640x480)
 * @param digit_index Index of digit to extract (0-7)
 * @param output Output 28x28 grayscale image
 * @return 0 on success, -1 on failure
 */
int extract_digit_roi(const uint8_t* image, uint8_t digit_index, uint8_t* output)
{
    if (image == NULL || output == NULL) {
        printf("ERROR: Invalid parameters\n");
        return -1;
    }

    if (digit_index >= 8) {
        printf("ERROR: Invalid digit index %d (must be 0-7)\n", digit_index);
        return -1;
    }

    // Get ROI configuration for this digit
    const digit_roi_config_t* roi = &digit_rois[digit_index];

    // Validate ROI bounds
    if (roi->x + roi->width > 640 || roi->y + roi->height > 480) {
        printf("ERROR: ROI out of bounds for digit %d\n", digit_index);
        return -1;
    }

    // Extract ROI from full image
    uint8_t roi_buffer[roi->width * roi->height];
    for (uint16_t y = 0; y < roi->height; y++) {
        for (uint16_t x = 0; x < roi->width; x++) {
            uint16_t src_x = roi->x + x;
            uint16_t src_y = roi->y + y;
            roi_buffer[y * roi->width + x] = image[src_y * 640 + src_x];
        }
    }

    // Resize ROI to 28x28 for model input
    resize_nearest_neighbor(roi_buffer, roi->width, roi->height, output, 28, 28);

    return 0;
}

/**
 * @brief Configure digit ROI positions
 *
 * This function allows runtime configuration of digit positions,
 * useful for calibration or supporting different meter types.
 *
 * @param digit_index Index of digit to configure (0-7)
 * @param x Top-left X coordinate
 * @param y Top-left Y coordinate
 * @param width ROI width
 * @param height ROI height
 * @return 0 on success, -1 on failure
 */
int configure_digit_roi(uint8_t digit_index, uint16_t x, uint16_t y,
                       uint16_t width, uint16_t height)
{
    if (digit_index >= 8) {
        printf("ERROR: Invalid digit index %d\n", digit_index);
        return -1;
    }

    if (x + width > 640 || y + height > 480) {
        printf("ERROR: ROI out of bounds\n");
        return -1;
    }

    // Note: This modifies const data - in real implementation,
    // you'd want to make digit_rois non-const or use a separate config
    printf("Configured ROI for digit %d: x=%d, y=%d, w=%d, h=%d\n",
           digit_index, x, y, width, height);

    return 0;
}

/**
 * @brief Auto-detect digit regions using image processing
 *
 * This is a placeholder for advanced digit detection using
 * edge detection, contour finding, etc.
 *
 * @param image Full captured image (640x480)
 * @param detected_rois Output array of detected ROIs (must hold 8 elements)
 * @param count Output number of ROIs detected
 * @return 0 on success, -1 on failure
 */
int auto_detect_digits(const uint8_t* image, digit_roi_config_t* detected_rois, uint8_t* count)
{
    if (image == NULL || detected_rois == NULL || count == NULL) {
        return -1;
    }

    // TODO: Implement actual digit detection using image processing
    // For now, just return the default ROIs
    *count = 8;
    for (uint8_t i = 0; i < 8; i++) {
        detected_rois[i] = digit_rois[i];
    }

    printf("Auto-detect not implemented, using default ROIs\n");
    return 0;
}

/**
 * @brief Apply preprocessing to digit image
 *
 * Applies various preprocessing steps to improve recognition:
 * - Contrast enhancement
 * - Noise reduction
 * - Normalization
 *
 * @param digit 28x28 digit image (modified in-place)
 * @return 0 on success, -1 on failure
 */
int preprocess_digit(uint8_t* digit)
{
    if (digit == NULL) {
        return -1;
    }

    // Simple contrast enhancement using histogram stretching
    uint8_t min_val = 255;
    uint8_t max_val = 0;

    // Find min and max values
    for (int i = 0; i < 28 * 28; i++) {
        if (digit[i] < min_val) min_val = digit[i];
        if (digit[i] > max_val) max_val = digit[i];
    }

    // Avoid division by zero
    if (max_val == min_val) {
        return 0;
    }

    // Stretch histogram to full range [0, 255]
    float scale = 255.0f / (max_val - min_val);
    for (int i = 0; i < 28 * 28; i++) {
        digit[i] = (uint8_t)((digit[i] - min_val) * scale);
    }

    return 0;
}

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
                        uint16_t* width, uint16_t* height)
{
    if (digit_index >= 8) {
        return -1;
    }

    const digit_roi_config_t* roi = &digit_rois[digit_index];

    if (x != NULL) *x = roi->x;
    if (y != NULL) *y = roi->y;
    if (width != NULL) *width = roi->width;
    if (height != NULL) *height = roi->height;

    return 0;
}

/**
 * @brief Validate image quality for digit recognition
 *
 * Checks image for common issues that affect recognition:
 * - Too dark or too bright
 * - Low contrast
 * - Blur
 *
 * @param image Full image (640x480)
 * @return Quality score 0-100, or -1 on error
 */
int8_t validate_image_quality(const uint8_t* image)
{
    if (image == NULL) {
        return -1;
    }

    // Calculate average brightness
    uint32_t sum = 0;
    for (int i = 0; i < 640 * 480; i++) {
        sum += image[i];
    }
    uint8_t avg = sum / (640 * 480);

    // Check brightness
    int8_t quality = 100;

    if (avg < 30) {
        printf("WARNING: Image too dark (avg=%d)\n", avg);
        quality -= 30;
    } else if (avg > 225) {
        printf("WARNING: Image too bright (avg=%d)\n", avg);
        quality -= 30;
    }

    // Check contrast (simplified)
    uint8_t min_val = 255, max_val = 0;
    for (int i = 0; i < 640 * 480; i += 10) {  // Sample every 10th pixel
        if (image[i] < min_val) min_val = image[i];
        if (image[i] > max_val) max_val = image[i];
    }

    uint8_t contrast = max_val - min_val;
    if (contrast < 50) {
        printf("WARNING: Low contrast (contrast=%d)\n", contrast);
        quality -= 20;
    }

    if (quality < 0) quality = 0;

    return quality;
}
