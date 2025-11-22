/**
 * @file training_mode.c
 * @brief Custom firmware for water meter digit training
 *
 * This firmware is designed specifically for collecting training data.
 * It captures images from the camera and streams them to PC via UART.
 *
 * Commands:
 *   CAPTURE  - Capture and send a single frame
 *   STREAM   - Start continuous streaming
 *   STOP     - Stop streaming
 *   STATUS   - Get device status
 *
 * Usage:
 *   1. Flash this firmware to WE2
 *   2. Connect via UART (921600 baud)
 *   3. Use training_gui.py to collect images
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "WE2_device.h"
#include "WE2_core.h"
#include "hx_drv_timer.h"
#include "hx_drv_uart.h"
#include "hx_drv_gpio.h"
#include "sensor_dp_lib.h"
#include "hx_drv_CIS_HM0360_regs.h"

// ==================== Configuration ====================

#define UART_PORT               USE_DW_UART_0
#define UART_BAUDRATE           UART_BR_921600

#define CAPTURE_WIDTH           640
#define CAPTURE_HEIGHT          480
#define IMAGE_SIZE              (CAPTURE_WIDTH * CAPTURE_HEIGHT)

#define CMD_BUFFER_SIZE         128
#define TIMEOUT_MS              5000

// ==================== Global Variables ====================

static uint8_t cmd_buffer[CMD_BUFFER_SIZE];
static uint32_t cmd_index = 0;

static bool streaming_enabled = false;
static uint32_t frame_count = 0;

// ==================== UART Communication ====================

/**
 * @brief Initialize UART for PC communication
 */
int uart_init(void)
{
    HX_DRV_UART_CONFIG uart_cfg;

    uart_cfg.uart_baudrate = UART_BAUDRATE;
    uart_cfg.uart_protocol = UART_PROTOCOL_8N1;
    uart_cfg.uart_fifo = UART_FIFO_ENABLE;

    if (hx_drv_uart_init(UART_PORT, &uart_cfg) != HX_DRV_LIB_PASS) {
        return -1;
    }

    printf("\n");
    printf("========================================\n");
    printf("  WE2 Training Mode Firmware\n");
    printf("========================================\n");
    printf("  Baudrate: %d\n", 921600);
    printf("  Image size: %dx%d\n", CAPTURE_WIDTH, CAPTURE_HEIGHT);
    printf("========================================\n");
    printf("\n");

    return 0;
}

/**
 * @brief Send response to PC
 */
void send_response(const char* response)
{
    uint32_t len = strlen(response);

    for (uint32_t i = 0; i < len; i++) {
        hx_drv_uart_write(UART_PORT, response[i]);
    }

    hx_drv_uart_write(UART_PORT, '\r');
    hx_drv_uart_write(UART_PORT, '\n');
}

/**
 * @brief Read command from PC
 *
 * @return true if command received, false if timeout
 */
bool read_command(char* cmd, uint32_t max_len, uint32_t timeout_ms)
{
    uint32_t start_time = hx_drv_timer_get_ms();
    uint32_t index = 0;

    memset(cmd, 0, max_len);

    while ((hx_drv_timer_get_ms() - start_time) < timeout_ms) {
        uint8_t byte;

        if (hx_drv_uart_read(UART_PORT, &byte) == HX_DRV_LIB_PASS) {
            if (byte == '\r' || byte == '\n') {
                if (index > 0) {
                    cmd[index] = '\0';
                    return true;
                }
            } else if (index < max_len - 1) {
                cmd[index++] = byte;
            }
        }

        hx_drv_timer_delay_ms(1);
    }

    return false;
}

/**
 * @brief Check for command (non-blocking)
 */
bool check_command(char* cmd, uint32_t max_len)
{
    uint8_t byte;

    while (hx_drv_uart_read(UART_PORT, &byte) == HX_DRV_LIB_PASS) {
        if (byte == '\r' || byte == '\n') {
            if (cmd_index > 0) {
                cmd_buffer[cmd_index] = '\0';
                strncpy(cmd, (char*)cmd_buffer, max_len);
                cmd_index = 0;
                return true;
            }
        } else if (cmd_index < CMD_BUFFER_SIZE - 1) {
            cmd_buffer[cmd_index++] = byte;
        }
    }

    return false;
}

// ==================== Camera Functions ====================

/**
 * @brief Initialize camera sensor
 */
int camera_init(void)
{
    printf("Initializing camera...\n");

    // Initialize sensor datapath
    if (sensordp_init() != 0) {
        printf("ERROR: sensordp_init failed\n");
        return -1;
    }

    // Start sensor
    if (sensordp_start() != 0) {
        printf("ERROR: sensordp_start failed\n");
        return -1;
    }

    printf("✓ Camera initialized\n");
    return 0;
}

/**
 * @brief Capture frame from camera
 *
 * @param buffer Buffer to store image data
 * @param width Image width
 * @param height Image height
 * @return 0 on success, -1 on failure
 */
int camera_capture_frame(uint8_t* buffer, uint32_t width, uint32_t height)
{
    sensordp_frame_t frame_data;

    // Get frame from sensor
    if (sensordp_get_frame(&frame_data, TIMEOUT_MS) != 0) {
        printf("ERROR: sensordp_get_frame failed\n");
        return -1;
    }

    // Copy frame data
    uint32_t size = width * height;
    if (frame_data.raw_data && frame_data.raw_size >= size) {
        memcpy(buffer, frame_data.raw_data, size);
    } else {
        printf("ERROR: Invalid frame data\n");
        sensordp_release_frame(&frame_data);
        return -1;
    }

    // Release frame
    sensordp_release_frame(&frame_data);

    return 0;
}

/**
 * @brief Send image data via UART
 */
void send_image_data(const uint8_t* image, uint32_t size)
{
    // Send image data in chunks
    const uint32_t CHUNK_SIZE = 512;

    for (uint32_t offset = 0; offset < size; offset += CHUNK_SIZE) {
        uint32_t chunk_size = (offset + CHUNK_SIZE <= size) ? CHUNK_SIZE : (size - offset);

        for (uint32_t i = 0; i < chunk_size; i++) {
            hx_drv_uart_write(UART_PORT, image[offset + i]);
        }

        // Small delay to prevent buffer overflow
        hx_drv_timer_delay_ms(5);
    }
}

// ==================== Command Handlers ====================

/**
 * @brief Handle CAPTURE command
 */
void handle_capture_command(void)
{
    static uint8_t image_buffer[IMAGE_SIZE];

    printf("Capturing frame...\n");

    send_response("OK");

    // Capture frame
    if (camera_capture_frame(image_buffer, CAPTURE_WIDTH, CAPTURE_HEIGHT) == 0) {
        printf("Sending image data (%d bytes)...\n", IMAGE_SIZE);

        // Send image data
        send_image_data(image_buffer, IMAGE_SIZE);

        frame_count++;
        printf("✓ Frame %lu sent\n", frame_count);
    } else {
        send_response("ERROR");
        printf("✗ Capture failed\n");
    }
}

/**
 * @brief Handle STREAM command
 */
void handle_stream_command(void)
{
    printf("Starting stream mode...\n");
    streaming_enabled = true;
    send_response("OK");
}

/**
 * @brief Handle STOP command
 */
void handle_stop_command(void)
{
    printf("Stopping stream mode...\n");
    streaming_enabled = false;
    send_response("OK");
}

/**
 * @brief Handle STATUS command
 */
void handle_status_command(void)
{
    char status[128];
    snprintf(status, sizeof(status),
             "STATUS: frames=%lu streaming=%d width=%d height=%d",
             frame_count,
             streaming_enabled ? 1 : 0,
             CAPTURE_WIDTH,
             CAPTURE_HEIGHT);

    send_response(status);
}

/**
 * @brief Process command
 */
void process_command(const char* cmd)
{
    printf("Command received: %s\n", cmd);

    if (strcmp(cmd, "CAPTURE") == 0) {
        handle_capture_command();
    }
    else if (strcmp(cmd, "STREAM") == 0) {
        handle_stream_command();
    }
    else if (strcmp(cmd, "STOP") == 0) {
        handle_stop_command();
    }
    else if (strcmp(cmd, "STATUS") == 0) {
        handle_status_command();
    }
    else {
        printf("Unknown command: %s\n", cmd);
        send_response("ERROR: Unknown command");
    }
}

// ==================== Main Application ====================

/**
 * @brief Main function
 */
int main(void)
{
    char cmd[CMD_BUFFER_SIZE];

    // Initialize UART
    if (uart_init() != 0) {
        printf("UART init failed!\n");
        return -1;
    }

    // Initialize camera
    if (camera_init() != 0) {
        printf("Camera init failed!\n");
        send_response("ERROR: Camera init failed");
        return -1;
    }

    printf("\n");
    printf("✓ Training mode ready\n");
    printf("  Waiting for commands...\n");
    printf("\n");

    send_response("READY");

    // Main loop
    while (1) {
        // Check for commands
        if (check_command(cmd, sizeof(cmd))) {
            process_command(cmd);
        }

        // Streaming mode
        if (streaming_enabled) {
            handle_capture_command();
            hx_drv_timer_delay_ms(100);  // 10 FPS
        }

        // Small delay
        hx_drv_timer_delay_ms(10);
    }

    return 0;
}
