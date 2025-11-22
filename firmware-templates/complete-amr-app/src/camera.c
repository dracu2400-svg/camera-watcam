/**
 * @file camera.c
 * @brief Camera interface module for HIMAX WE2
 *
 * This module handles camera initialization, configuration,
 * and frame capture for the water meter AMR application.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "camera.h"

// HIMAX SDK camera driver includes
#include "hx_drv_CIS.h"
#include "sensor_dp_lib.h"

// Camera configuration
#define CAMERA_WIDTH  640
#define CAMERA_HEIGHT 480
#define CAMERA_FPS    15

// Frame buffer
static uint8_t frame_buffer[CAMERA_WIDTH * CAMERA_HEIGHT] __attribute__((aligned(32)));
static volatile uint8_t frame_ready = 0;

/**
 * @brief Camera frame callback
 *
 * Called by the camera driver when a new frame is available.
 */
static void camera_frame_callback(void)
{
    frame_ready = 1;
}

/**
 * @brief Initialize camera sensor and configure settings
 *
 * @return 0 on success, -1 on failure
 */
int camera_init(void)
{
    printf("Initializing camera sensor...\n");

    // Initialize CIS (CMOS Image Sensor) module
    HX_CIS_ERROR_E ret;

    // Set sensor to HM0360 (or configured sensor)
    ret = hx_drv_cis_init(CIS_SENSOR_TYPE_HM0360);
    if (ret != HX_CIS_NO_ERROR) {
        printf("ERROR: Camera sensor init failed (error: %d)\n", ret);
        return -1;
    }

    // Configure sensor for our requirements
    hx_drv_cis_setFormat(CAMERA_WIDTH, CAMERA_HEIGHT, CIS_FORMAT_YUV422);
    hx_drv_cis_set_fps(CAMERA_FPS);

    // Configure auto exposure and white balance
    hx_drv_cis_set_ae(1);  // Enable auto exposure
    hx_drv_cis_set_awb(1); // Enable auto white balance

    // Register frame callback
    hx_drv_cis_register_frame_callback(camera_frame_callback);

    // Start sensor data path
    sensordp_start_capture(CAMERA_WIDTH, CAMERA_HEIGHT, SENSORDP_STREAM_MODE);

    printf("✓ Camera initialized:\n");
    printf("  Sensor: HM0360\n");
    printf("  Resolution: %dx%d\n", CAMERA_WIDTH, CAMERA_HEIGHT);
    printf("  Frame rate: %d FPS\n", CAMERA_FPS);
    printf("  Format: YUV422\n");

    return 0;
}

/**
 * @brief Capture a single frame from camera
 *
 * @param buffer Output buffer for captured frame (must be width*height bytes)
 * @param width Expected frame width
 * @param height Expected frame height
 * @return 0 on success, -1 on failure
 */
int camera_capture_frame(uint8_t* buffer, uint16_t width, uint16_t height)
{
    if (buffer == NULL) {
        printf("ERROR: Invalid buffer\n");
        return -1;
    }

    if (width != CAMERA_WIDTH || height != CAMERA_HEIGHT) {
        printf("ERROR: Invalid dimensions (%dx%d, expected %dx%d)\n",
               width, height, CAMERA_WIDTH, CAMERA_HEIGHT);
        return -1;
    }

    // Trigger capture
    frame_ready = 0;
    sensordp_trigger_capture();

    // Wait for frame (with timeout)
    uint32_t timeout = 1000; // 1 second timeout
    uint32_t start_time = hx_drv_timer_get_ms();

    while (!frame_ready) {
        if ((hx_drv_timer_get_ms() - start_time) > timeout) {
            printf("ERROR: Camera capture timeout\n");
            return -1;
        }
    }

    // Get frame data from sensor datapath
    uint32_t frame_size;
    uint8_t* frame_ptr;

    if (sensordp_get_frame(&frame_ptr, &frame_size) != 0) {
        printf("ERROR: Failed to get frame data\n");
        return -1;
    }

    // Convert YUV422 to grayscale (just use Y channel)
    // YUV422 format: Y0 U0 Y1 V0 Y2 U1 Y3 V1 ...
    // We extract every other byte (Y values) for grayscale
    for (uint32_t i = 0; i < width * height; i++) {
        buffer[i] = frame_ptr[i * 2];  // Extract Y channel
    }

    return 0;
}

/**
 * @brief Configure camera settings
 *
 * @param brightness Brightness level (-100 to 100, 0 = auto)
 * @param contrast Contrast level (-100 to 100, 0 = auto)
 * @return 0 on success, -1 on failure
 */
int camera_set_config(int8_t brightness, int8_t contrast)
{
    // Configure brightness
    if (brightness != 0) {
        hx_drv_cis_set_brightness(brightness);
    }

    // Configure contrast
    if (contrast != 0) {
        hx_drv_cis_set_contrast(contrast);
    }

    printf("Camera settings updated: brightness=%d, contrast=%d\n",
           brightness, contrast);

    return 0;
}

/**
 * @brief Get camera status
 *
 * @param width Output current width
 * @param height Output current height
 * @param fps Output current FPS
 * @return 0 on success, -1 on failure
 */
int camera_get_status(uint16_t* width, uint16_t* height, uint8_t* fps)
{
    if (width != NULL) {
        *width = CAMERA_WIDTH;
    }

    if (height != NULL) {
        *height = CAMERA_HEIGHT;
    }

    if (fps != NULL) {
        *fps = CAMERA_FPS;
    }

    return 0;
}

/**
 * @brief Deinitialize camera
 */
void camera_deinit(void)
{
    sensordp_stop_capture();
    hx_drv_cis_deinit();
    printf("Camera deinitialized\n");
}
