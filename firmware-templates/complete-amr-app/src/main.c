/**
 * @file main.c
 * @brief Water Meter AMR Main Application
 *
 * This is the main application file for the water meter automatic meter reading (AMR) system.
 * It integrates camera capture, ML inference, LoRaWAN communication, and power management.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "WE2_device.h"
#include "hx_drv_timer.h"

// Module headers
#include "camera.h"
#include "inference.h"
#include "lorawan_comm.h"
#include "power_mgmt.h"
#include "digit_detection.h"

// Configuration
#define READING_INTERVAL_SECONDS 3600  // 1 hour between readings
#define NUM_DIGITS 8                   // Number of digits on meter display

// LoRaWAN Credentials (replace with actual values during provisioning)
// These should be unique per device and configured during manufacturing
#define DEV_EUI  "70B3D57ED0050001"
#define APP_EUI  "0000000000000000"
#define APP_KEY  "00000000000000000000000000000000"

/**
 * @brief Main application entry point
 *
 * Initializes all subsystems (camera, ML, LoRaWAN, power management),
 * then enters the main reading loop.
 */
int main(void)
{
    printf("\n");
    printf("========================================\n");
    printf("  Water Meter AMR Application\n");
    printf("  Version: 1.0.0\n");
    printf("  Build: %s %s\n", __DATE__, __TIME__);
    printf("========================================\n");
    printf("\n");

    // Initialize all subsystems
    printf("Initializing modules...\n");

    // Initialize camera
    if (camera_init() != 0) {
        printf("ERROR: Camera initialization failed\n");
        return -1;
    }
    printf("✓ Camera initialized\n");

    // Initialize ML inference engine
    if (ml_init() != 0) {
        printf("ERROR: ML initialization failed\n");
        return -1;
    }
    printf("✓ ML initialized\n");

    // Initialize LoRaWAN communication
    if (lorawan_init() != 0) {
        printf("ERROR: LoRaWAN initialization failed\n");
        return -1;
    }
    printf("✓ LoRaWAN initialized\n");

    // Initialize power management
    if (power_init() != 0) {
        printf("ERROR: Power management initialization failed\n");
        return -1;
    }
    printf("✓ Power management initialized\n");

    // Configure LoRaWAN with device credentials
    printf("\nConfiguring LoRaWAN...\n");
    if (lorawan_configure_otaa(DEV_EUI, APP_EUI, APP_KEY) != 0) {
        printf("ERROR: LoRaWAN configuration failed\n");
        return -1;
    }
    printf("✓ LoRaWAN configured (OTAA mode)\n");

    // Join LoRaWAN network (try up to 3 times)
    printf("\nJoining LoRaWAN network...\n");
    if (lorawan_join(3) != 0) {
        printf("ERROR: Failed to join LoRaWAN network\n");
        printf("  Check: 1) Gateway coverage\n");
        printf("         2) Device credentials\n");
        printf("         3) Network server configuration\n");
        return -1;
    }
    printf("✓ Successfully joined network\n");

    printf("\n");
    printf("========================================\n");
    printf("✓ AMR System Ready\n");
    printf("  Reading interval: %d seconds (%d minutes)\n",
           READING_INTERVAL_SECONDS, READING_INTERVAL_SECONDS / 60);
    printf("  Expected digits: %d\n", NUM_DIGITS);
    printf("========================================\n");
    printf("\n");

    // Main reading loop
    uint32_t reading_count = 0;

    while (1) {
        reading_count++;

        printf("\n");
        printf("========================================\n");
        printf("  Reading Cycle #%lu\n", reading_count);
        printf("  Time: %lu seconds since boot\n", hx_drv_timer_get_ms() / 1000);
        printf("========================================\n");
        printf("\n");

        // Capture image from camera
        printf("Capturing image...\n");
        uint8_t* image = (uint8_t*)malloc(640 * 480);

        if (image == NULL) {
            printf("ERROR: Failed to allocate memory for image\n");
            continue;
        }

        if (camera_capture_frame(image, 640, 480) == 0) {
            printf("✓ Image captured (640x480 pixels)\n");

            // Process digits from captured image
            uint8_t digits[NUM_DIGITS];
            uint8_t digit_count = 0;
            float total_confidence = 0.0f;

            printf("\nRecognizing digits...\n");

            // Process each digit position
            for (int i = 0; i < NUM_DIGITS; i++) {
                // Extract digit ROI (Region of Interest) and resize to 28x28
                uint8_t digit_roi[28 * 28];

                if (extract_digit_roi(image, i, digit_roi) == 0) {
                    // Run ML inference on the extracted digit
                    int result;
                    float confidence;

                    if (ml_inference(digit_roi, &result, &confidence) == 0) {
                        digits[digit_count] = result;
                        total_confidence += confidence;
                        digit_count++;

                        printf("  Digit %d: %d (%.2f%% confidence)\n",
                               i, result, confidence * 100.0f);
                    } else {
                        printf("  Digit %d: Inference failed\n", i);
                    }
                } else {
                    printf("  Digit %d: ROI extraction failed\n", i);
                }
            }

            // Process and transmit results if we got at least some digits
            if (digit_count > 0) {
                float avg_confidence = total_confidence / digit_count;

                printf("\n--- Reading Results ---\n");
                printf("Meter Reading: ");
                for (int i = 0; i < digit_count; i++) {
                    printf("%d", digits[i]);
                }
                printf("\n");
                printf("Digits Recognized: %d / %d\n", digit_count, NUM_DIGITS);
                printf("Average Confidence: %.2f%%\n", avg_confidence * 100.0f);
                printf("----------------------\n");

                // Send reading via LoRaWAN
                printf("\nTransmitting reading via LoRaWAN...\n");
                if (lorawan_send_meter_reading(digits, digit_count, avg_confidence) == 0) {
                    printf("✓ Reading transmitted successfully\n");
                } else {
                    printf("✗ Failed to transmit reading\n");
                    printf("  Data will be retried on next cycle\n");
                }
            } else {
                printf("\n✗ No digits recognized\n");
                printf("  Possible causes:\n");
                printf("  - Poor lighting conditions\n");
                printf("  - Meter not in camera view\n");
                printf("  - Camera focus issue\n");
            }
        } else {
            printf("✗ Image capture failed\n");
        }

        // Free allocated image memory
        free(image);

        // Enter low-power sleep mode until next reading
        printf("\nEntering sleep mode...\n");
        printf("  Duration: %d seconds (%d minutes)\n",
               READING_INTERVAL_SECONDS, READING_INTERVAL_SECONDS / 60);
        printf("  Next reading: Cycle #%lu\n", reading_count + 1);
        printf("\n");

        power_enter_sleep(READING_INTERVAL_SECONDS);

        // System wakes up here after sleep period
        printf("Waking up from sleep...\n\n");
    }

    // Should never reach here
    return 0;
}
