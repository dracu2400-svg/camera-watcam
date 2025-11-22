/**
 * @file config.example.h
 * @brief Example configuration file for Water Meter AMR system
 *
 * Copy this file to your firmware directory and rename to config.h
 * Then customize the values for your deployment.
 */

#ifndef CONFIG_EXAMPLE_H
#define CONFIG_EXAMPLE_H

//=============================================================================
// LORAWAN CONFIGURATION
//=============================================================================

/**
 * LoRaWAN Device Credentials (OTAA)
 *
 * IMPORTANT: These are example values only!
 * Replace with actual credentials from your network server.
 *
 * Format:
 * - DevEUI: 16 hex characters (8 bytes)
 * - AppEUI: 16 hex characters (8 bytes)
 * - AppKey: 32 hex characters (16 bytes)
 *
 * Example DevEUI format: 70B3D57ED0050001
 *                        ^^^^^^^^^^^^---- Organization prefix
 *                                    ^^^^- Device number
 */
#define LORAWAN_DEV_EUI  "70B3D57ED0050001"  // Change this!
#define LORAWAN_APP_EUI  "0000000000000000"  // Change this!
#define LORAWAN_APP_KEY  "00000000000000000000000000000000"  // Change this!

/**
 * LoRaWAN Region
 *
 * Supported regions:
 * - EU868  (Europe)
 * - US915  (North America)
 * - AS923  (Asia-Pacific)
 * - AU915  (Australia)
 * - KR920  (Korea)
 * - IN865  (India)
 */
#define LORAWAN_REGION "EU868"

/**
 * LoRaWAN Communication Settings
 */
#define LORAWAN_TX_PORT 10              // Application port number (1-223)
#define LORAWAN_CONFIRMED_UPLINK 1      // 1 = confirmed, 0 = unconfirmed
#define LORAWAN_ADR_ENABLED 1           // Adaptive Data Rate (1 = enabled)
#define LORAWAN_MAX_JOIN_ATTEMPTS 3     // Max attempts to join network

//=============================================================================
// READING CONFIGURATION
//=============================================================================

/**
 * Reading Interval
 *
 * How often to take meter readings (in seconds)
 *
 * Common values:
 * - 900   (15 minutes)
 * - 1800  (30 minutes)
 * - 3600  (1 hour)    - Default
 * - 7200  (2 hours)
 * - 14400 (4 hours)
 * - 43200 (12 hours)
 * - 86400 (24 hours)
 */
#define READING_INTERVAL_SECONDS 3600

/**
 * Number of Digits on Meter Display
 *
 * Adjust based on your water meter type
 * Common values: 6, 7, or 8 digits
 */
#define NUM_DIGITS 8

//=============================================================================
// CAMERA CONFIGURATION
//=============================================================================

/**
 * Camera Sensor Type
 *
 * Supported sensors:
 * - HM0360 (default)
 * - OV5647
 * - IMX219
 */
#define CAMERA_SENSOR HM0360

/**
 * Camera Resolution
 */
#define CAMERA_WIDTH 640
#define CAMERA_HEIGHT 480

/**
 * Camera Frame Rate
 */
#define CAMERA_FPS 15

/**
 * Auto Exposure and White Balance
 */
#define CAMERA_AUTO_EXPOSURE 1
#define CAMERA_AUTO_WHITE_BALANCE 1

/**
 * Manual Camera Adjustments
 *
 * Set to 0 for automatic, or specify value:
 * Brightness: -100 to +100
 * Contrast:   -100 to +100
 */
#define CAMERA_BRIGHTNESS 0
#define CAMERA_CONTRAST 0

//=============================================================================
// DIGIT DETECTION CONFIGURATION
//=============================================================================

/**
 * Digit ROI (Region of Interest) Positions
 *
 * These define where each digit is located in the 640x480 camera image.
 * You must calibrate these for your specific water meter!
 *
 * Format: {x, y, width, height}
 * - x, y: Top-left corner coordinates
 * - width, height: ROI dimensions
 *
 * Calibration procedure:
 * 1. Capture a test image of your meter
 * 2. Use an image viewer to find pixel coordinates
 * 3. Update these values
 * 4. Rebuild and test
 */
#define DIGIT_0_ROI {100, 220, 40, 60}
#define DIGIT_1_ROI {150, 220, 40, 60}
#define DIGIT_2_ROI {200, 220, 40, 60}
#define DIGIT_3_ROI {250, 220, 40, 60}
#define DIGIT_4_ROI {300, 220, 40, 60}
#define DIGIT_5_ROI {350, 220, 40, 60}
#define DIGIT_6_ROI {400, 220, 40, 60}
#define DIGIT_7_ROI {450, 220, 40, 60}

/**
 * Image Preprocessing
 *
 * Enable/disable image preprocessing steps
 */
#define ENABLE_CONTRAST_ENHANCEMENT 1
#define ENABLE_NOISE_REDUCTION 0
#define ENABLE_QUALITY_VALIDATION 1

/**
 * Quality Thresholds
 */
#define MIN_IMAGE_QUALITY_SCORE 50  // 0-100 scale
#define MIN_CONFIDENCE_THRESHOLD 0.80  // 0.0-1.0 scale

//=============================================================================
// POWER MANAGEMENT CONFIGURATION
//=============================================================================

/**
 * Power Modes
 *
 * DEEP_SLEEP: Lowest power consumption (~10 µA)
 * SLEEP: Light sleep (~100 µA)
 * IDLE: CPU idle (~1 mA)
 */
#define POWER_MODE_BETWEEN_READINGS DEEP_SLEEP

/**
 * Battery Monitoring
 */
#define ENABLE_BATTERY_MONITORING 1
#define BATTERY_LOW_THRESHOLD 20  // Percentage (0-100)
#define BATTERY_CRITICAL_THRESHOLD 10  // Percentage (0-100)

/**
 * Battery Type and Capacity
 *
 * Used for battery life estimation
 */
#define BATTERY_CAPACITY_MAH 2000  // Typical Li-ion
#define BATTERY_VOLTAGE_MAX_MV 4200  // Fully charged
#define BATTERY_VOLTAGE_MIN_MV 3000  // Empty

//=============================================================================
// ML MODEL CONFIGURATION
//=============================================================================

/**
 * TensorFlow Lite Micro Settings
 */
#define TFLITE_TENSOR_ARENA_SIZE (100 * 1024)  // 100KB
#define TFLITE_ENABLE_NPU 1  // Use Ethos-U55 NPU acceleration

/**
 * Model Input/Output
 */
#define MODEL_INPUT_WIDTH 28
#define MODEL_INPUT_HEIGHT 28
#define MODEL_INPUT_CHANNELS 1  // Grayscale
#define MODEL_OUTPUT_CLASSES 10  // Digits 0-9

/**
 * Inference Settings
 */
#define MAX_INFERENCE_TIME_MS 50  // Timeout per digit
#define ENABLE_INFERENCE_TIMING 1  // Log inference times

//=============================================================================
// DEBUGGING AND LOGGING
//=============================================================================

/**
 * Debug Output
 *
 * Levels:
 * 0 = Errors only
 * 1 = Errors + Warnings
 * 2 = Errors + Warnings + Info
 * 3 = All (including debug)
 */
#define DEBUG_LEVEL 2

/**
 * Serial Console
 */
#define SERIAL_BAUD_RATE 921600
#define ENABLE_SERIAL_OUTPUT 1

/**
 * Module-specific Debug Flags
 */
#define DEBUG_CAMERA 1
#define DEBUG_ML_INFERENCE 1
#define DEBUG_LORAWAN 1
#define DEBUG_POWER_MGMT 1
#define DEBUG_DIGIT_DETECTION 1

/**
 * Performance Profiling
 */
#define ENABLE_TIMING_STATS 1
#define LOG_MEMORY_USAGE 1

//=============================================================================
// ADVANCED SETTINGS
//=============================================================================

/**
 * Watchdog Timer
 */
#define ENABLE_WATCHDOG 1
#define WATCHDOG_TIMEOUT_SECONDS 60

/**
 * Error Recovery
 */
#define MAX_CONSECUTIVE_ERRORS 3  // Reset after N consecutive failures
#define ENABLE_ERROR_RECOVERY 1

/**
 * Network Retry Policy
 */
#define LORAWAN_RETRY_DELAY_MS 5000
#define LORAWAN_MAX_RETRIES 3

/**
 * Data Buffering
 *
 * Store readings locally if network unavailable
 */
#define ENABLE_LOCAL_STORAGE 0
#define MAX_STORED_READINGS 10

//=============================================================================
// HARDWARE PIN ASSIGNMENTS (Advanced)
//=============================================================================

/**
 * UART Assignments
 */
#define LORAWAN_UART_ID 0
#define LORAWAN_UART_BAUD 115200

/**
 * GPIO Pins
 */
#define LED_STATUS_PIN 13
#define LED_ERROR_PIN 14
#define BUTTON_RESET_PIN 15

#endif // CONFIG_EXAMPLE_H
