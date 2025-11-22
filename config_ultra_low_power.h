/**
 * @file config_ultra_low_power.h
 * @brief Ultra-Low-Power Configuration for 5-10 Year Battery Life
 *
 * This configuration optimizes the Water Meter AMR system for maximum battery life.
 * Target: 5-10 years on Li-SOCl₂ battery with 2-hour reading intervals.
 *
 * Expected power consumption: ~64 µA average
 * Required battery capacity:
 * - 5 years: 2,800 mAh minimum (recommend AA Li-SOCl₂ 2600 mAh)
 * - 10 years: 5,600 mAh minimum (recommend C-cell Li-SOCl₂ 8500 mAh)
 *
 * USAGE:
 * Copy this file to your firmware directory as config.h
 * Rebuild firmware with these settings
 */

#ifndef CONFIG_ULTRA_LOW_POWER_H
#define CONFIG_ULTRA_LOW_POWER_H

//=============================================================================
// ULTRA-LOW-POWER MODE
//=============================================================================

/**
 * Enable ultra-low-power optimizations
 *
 * This enables all power-saving features including:
 * - Ultra-deep sleep (<5 µA)
 * - Fast boot sequence
 * - Adaptive LoRaWAN TX power
 * - Smart transmission (skip if unchanged)
 */
#define ENABLE_ULTRA_LOW_POWER_MODE 1

//=============================================================================
// READING CONFIGURATION
//=============================================================================

/**
 * Reading Interval: 2 HOURS
 *
 * Readings every 2 hours = 12 readings per day
 * This significantly reduces average power consumption
 */
#define READING_INTERVAL_SECONDS 7200  // 2 hours

/**
 * Number of Digits
 */
#define NUM_DIGITS 8

/**
 * Smart Transmission
 *
 * Skip transmission if reading hasn't changed
 * Force transmission every 12 readings (24 hours) for reliability
 */
#define ENABLE_SMART_TRANSMISSION 1
#define FORCE_TX_EVERY_N_READINGS 12  // Force every 24 hours

//=============================================================================
// LORAWAN CONFIGURATION
//=============================================================================

/**
 * LoRaWAN Device Credentials (OTAA)
 * IMPORTANT: Replace with your actual credentials!
 */
#define LORAWAN_DEV_EUI  "70B3D57ED0050001"
#define LORAWAN_APP_EUI  "0000000000000000"
#define LORAWAN_APP_KEY  "00000000000000000000000000000000"

/**
 * LoRaWAN Region
 */
#define LORAWAN_REGION "EU868"

/**
 * LoRaWAN Power Optimization
 */
#define LORAWAN_TX_PORT 10

// Use UNCONFIRMED uplinks to save ~50% TX energy
// LoRaWAN will still ensure delivery via the network protocol
#define LORAWAN_CONFIRMED_UPLINK 0

// But force confirmation every 24 readings for reliability check
#define CONFIRM_EVERY_N_READINGS 24  // Once every 48 hours

// Adaptive TX power based on signal strength
#define LORAWAN_ADAPTIVE_TX_POWER 1
#define LORAWAN_MIN_TX_POWER 2   // dBm (when RSSI > -80 dBm)
#define LORAWAN_MED_TX_POWER 8   // dBm (when RSSI > -100 dBm)
#define LORAWAN_MAX_TX_POWER 14  // dBm (when RSSI <= -100 dBm)

// Use fastest data rate for shortest TX time
#define LORAWAN_PREFERRED_DATARATE SF7  // ~100ms TX time
#define LORAWAN_FALLBACK_DATARATE SF9   // If SF7 fails

// Disable ADR (saves join time and processing)
#define LORAWAN_ADR_ENABLED 0

// Join attempts
#define LORAWAN_MAX_JOIN_ATTEMPTS 3

//=============================================================================
// POWER MANAGEMENT CONFIGURATION
//=============================================================================

/**
 * Ultra-Deep Sleep Mode
 *
 * Achieves <5 µA sleep current by:
 * - Powering down all unused peripherals
 * - Setting unused GPIO to analog input
 * - Disabling debug interfaces
 * - Powering off camera sensor
 */
#define ULTRA_DEEP_SLEEP_MODE 1

/**
 * Power-Down Peripherals During Sleep
 */
#define POWER_DOWN_CAMERA_IN_SLEEP 1
#define DISABLE_DEBUG_UART_IN_SLEEP 1
#define DISABLE_LORAWAN_MODEM_IN_SLEEP 0  // Keep alive for RTC
#define DISABLE_UNUSED_GPIO_PULLS 1
#define SET_UNUSED_GPIO_TO_ANALOG 1

/**
 * Fast Boot Configuration
 *
 * Minimize time from wake to sleep
 * Target: <5 seconds active time
 */
#define ENABLE_FAST_BOOT 1
#define SKIP_BOOTLOADER_DELAY 1
#define CAMERA_QUICK_INIT_MODE 1
#define CAMERA_WARMUP_TIME_MS 200  // Reduced from 2000ms

/**
 * Battery Monitoring
 */
#define ENABLE_BATTERY_MONITORING 1
#define BATTERY_CAPACITY_MAH 8500  // C-cell Li-SOCl₂

// Li-SOCl₂ voltage curve (different from Li-ion)
#define BATTERY_VOLTAGE_MAX_MV 3600  // Fresh
#define BATTERY_VOLTAGE_NOM_MV 3500  // Normal operation
#define BATTERY_VOLTAGE_LOW_MV 3200  // Low warning
#define BATTERY_VOLTAGE_MIN_MV 3000  // Critical

// Send battery voltage with every reading
#define INCLUDE_BATTERY_IN_PAYLOAD 1

// Alert thresholds
#define BATTERY_LOW_THRESHOLD_MV 3200
#define BATTERY_CRITICAL_THRESHOLD_MV 3000

//=============================================================================
// CAMERA CONFIGURATION
//=============================================================================

/**
 * Camera Sensor
 */
#define CAMERA_SENSOR HM0360

/**
 * Camera Resolution
 *
 * Options for ultra-low-power:
 * - 640x480 (standard, recommended)
 * - 320x240 (faster processing, less power)
 */
#define CAMERA_WIDTH 640
#define CAMERA_HEIGHT 480

// Or use low-power mode:
// #define CAMERA_WIDTH_LOW_POWER 320
// #define CAMERA_HEIGHT_LOW_POWER 240

/**
 * Camera Settings
 */
#define CAMERA_FPS 15
#define CAMERA_AUTO_EXPOSURE 1
#define CAMERA_AUTO_WHITE_BALANCE 1
#define CAMERA_BRIGHTNESS 0  // Auto
#define CAMERA_CONTRAST 0    // Auto

/**
 * Single-Shot Mode
 *
 * Capture one frame and power down immediately
 * (No preview or continuous capture)
 */
#define CAMERA_SINGLE_SHOT_MODE 1

//=============================================================================
// DIGIT DETECTION CONFIGURATION
//=============================================================================

/**
 * Digit ROI Positions
 *
 * Calibrate these for your specific meter!
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
 * Image Quality Validation
 */
#define ENABLE_QUALITY_VALIDATION 1
#define MIN_IMAGE_QUALITY_SCORE 50  // 0-100

/**
 * Preprocessing
 */
#define ENABLE_CONTRAST_ENHANCEMENT 1
#define ENABLE_NOISE_REDUCTION 0  // Disable to save processing time

//=============================================================================
// ML MODEL CONFIGURATION
//=============================================================================

/**
 * TensorFlow Lite Micro
 */
#define TFLITE_TENSOR_ARENA_SIZE (100 * 1024)  // 100KB
#define TFLITE_ENABLE_NPU 1  // Use Ethos-U55 for acceleration

/**
 * Model Configuration
 */
#define MODEL_INPUT_WIDTH 28
#define MODEL_INPUT_HEIGHT 28
#define MODEL_INPUT_CHANNELS 1
#define MODEL_OUTPUT_CLASSES 10

/**
 * Inference Settings
 */
#define MAX_INFERENCE_TIME_MS 50
#define ENABLE_INFERENCE_TIMING 0  // Disable to reduce printf overhead
#define MIN_CONFIDENCE_THRESHOLD 0.80

//=============================================================================
// DEBUGGING AND LOGGING
//=============================================================================

/**
 * Debug Level
 *
 * PRODUCTION: Set to 0 (errors only)
 * This significantly reduces UART power consumption
 */
#define DEBUG_LEVEL 0  // 0 = Errors only, disable all debug output

/**
 * Serial Console
 *
 * PRODUCTION: Disable serial output to save power
 */
#define ENABLE_SERIAL_OUTPUT 0  // Disable in production
#define SERIAL_BAUD_RATE 921600

/**
 * Module Debug (all disabled for production)
 */
#define DEBUG_CAMERA 0
#define DEBUG_ML_INFERENCE 0
#define DEBUG_LORAWAN 0
#define DEBUG_POWER_MGMT 0
#define DEBUG_DIGIT_DETECTION 0

/**
 * Performance Profiling (disabled for production)
 */
#define ENABLE_TIMING_STATS 0
#define LOG_MEMORY_USAGE 0

/**
 * Disable printf in production
 *
 * Replaces all printf() with empty macros
 */
#define DISABLE_PRINTF_IN_PRODUCTION 1

#if DISABLE_PRINTF_IN_PRODUCTION && (DEBUG_LEVEL == 0)
    #define printf(...)  // Empty macro
#endif

//=============================================================================
// DATA CACHING AND RELIABILITY
//=============================================================================

/**
 * Reading Cache
 *
 * Store readings locally if network unavailable
 * Transmit cached readings when network recovers
 */
#define ENABLE_READING_CACHE 1
#define MAX_CACHED_READINGS 24  // Store up to 48 hours of readings

/**
 * Retry Policy
 */
#define LORAWAN_RETRY_DELAY_MS 5000
#define LORAWAN_MAX_RETRIES 2  // Limited retries to save power

/**
 * Network Recovery
 */
#define AUTO_REJOIN_ON_FAILURE 1
#define MAX_REJOIN_ATTEMPTS 3

//=============================================================================
// WATCHDOG TIMER
//=============================================================================

/**
 * Watchdog Configuration
 */
#define ENABLE_WATCHDOG 1
#define WATCHDOG_TIMEOUT_SECONDS 30  // Faster recovery if stuck

//=============================================================================
// ADVANCED POWER OPTIMIZATIONS
//=============================================================================

/**
 * Hardware Power Control
 */
#define USE_HARDWARE_POWER_SWITCH 0  // If available on PCB
#define POWER_OFF_CAMERA_MODULE 1
#define POWER_OFF_SENSORS_IN_SLEEP 1

/**
 * Clock Frequency Scaling
 *
 * Reduce CPU frequency during active time to save power
 * (Trade-off: slightly longer active time)
 */
#define ENABLE_FREQUENCY_SCALING 0  // Experimental
#define ACTIVE_CPU_FREQ_MHZ 200     // Reduced from 400 MHz

/**
 * Optimize Compiler Settings
 *
 * These should be set in Makefile:
 * APPL_OPT = -Os  # Optimize for size (also reduces power)
 */

//=============================================================================
// EXPECTED PERFORMANCE
//=============================================================================

/**
 * Power Consumption Summary (with optimizations)
 *
 * Deep Sleep:     5 µA  (7195 sec per 2h cycle)
 * Active Time:   90 mA  (5 sec per 2h cycle)
 *
 * Average Current: ~64 µA
 *
 * Battery Life Estimates:
 * - AA Li-SOCl₂ (2600 mAh):    5.0 years
 * - C-cell Li-SOCl₂ (8500 mAh): 15.2 years
 * - D-cell Li-SOCl₂ (19000 mAh): 34.0 years
 */

//=============================================================================
// NOTES
//=============================================================================

/**
 * IMPORTANT NOTES FOR DEPLOYMENT:
 *
 * 1. Battery Selection:
 *    - Use Li-SOCl₂ (Lithium Thionyl Chloride) batteries
 *    - NOT standard Li-ion (higher self-discharge)
 *    - Brands: Saft LS14500, Tadiran SL-760 (AA)
 *              Saft LS26500, Tadiran SL-2770 (C-cell)
 *
 * 2. Temperature Considerations:
 *    - Li-SOCl₂ works from -60°C to +85°C
 *    - Battery capacity reduces at low temperatures
 *    - Add 20% margin for extreme temperatures
 *
 * 3. Self-Discharge:
 *    - Li-SOCl₂: <1% per year
 *    - Li-ion: 2-5% per month
 *    - This is why Li-SOCl₂ is essential for long life
 *
 * 4. Peak Current:
 *    - LoRaWAN TX can draw 120+ mA in bursts
 *    - Li-SOCl₂ may need capacitor for peak loads
 *    - Consider hybrid Li-SOCl₂ with built-in capacitor
 *
 * 5. Field Testing:
 *    - Always measure actual current consumption
 *    - Test in actual deployment temperature
 *    - Monitor first units for 1 month
 *
 * 6. Maintenance:
 *    - Set calendar reminders for battery replacement
 *    - Monitor battery voltage via LoRaWAN
 *    - Replace at 3.2V (don't wait for 3.0V)
 */

#endif // CONFIG_ULTRA_LOW_POWER_H
