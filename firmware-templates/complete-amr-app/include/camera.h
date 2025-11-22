/**
 * @file camera.h
 * @brief Camera interface module header
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize camera sensor and configure settings
 *
 * @return 0 on success, -1 on failure
 */
int camera_init(void);

/**
 * @brief Capture a single frame from camera
 *
 * @param buffer Output buffer for captured frame (must be width*height bytes)
 * @param width Expected frame width
 * @param height Expected frame height
 * @return 0 on success, -1 on failure
 */
int camera_capture_frame(uint8_t* buffer, uint16_t width, uint16_t height);

/**
 * @brief Configure camera settings
 *
 * @param brightness Brightness level (-100 to 100, 0 = auto)
 * @param contrast Contrast level (-100 to 100, 0 = auto)
 * @return 0 on success, -1 on failure
 */
int camera_set_config(int8_t brightness, int8_t contrast);

/**
 * @brief Get camera status
 *
 * @param width Output current width
 * @param height Output current height
 * @param fps Output current FPS
 * @return 0 on success, -1 on failure
 */
int camera_get_status(uint16_t* width, uint16_t* height, uint8_t* fps);

/**
 * @brief Deinitialize camera
 */
void camera_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // CAMERA_H
