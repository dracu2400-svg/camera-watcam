/**
 * @file power_mgmt.c
 * @brief Power management module for battery-powered operation
 *
 * This module handles power states, sleep modes, and battery monitoring
 * to maximize battery life in the water meter AMR application.
 */

#include <stdio.h>
#include <stdint.h>
#include "power_mgmt.h"

// HIMAX power management driver
#include "hx_drv_pmu.h"
#include "hx_drv_timer.h"

// Power state
static power_mode_t current_mode = POWER_MODE_ACTIVE;

/**
 * @brief Initialize power management
 *
 * @return 0 on success, -1 on failure
 */
int power_init(void)
{
    printf("Initializing power management...\n");

    // Initialize PMU (Power Management Unit)
    hx_drv_pmu_init();

    // Configure wake-up sources
    hx_drv_pmu_set_wakeup_event(PMU_WAKEUP_RTC);  // Wake from RTC timer

    // Set initial power mode
    current_mode = POWER_MODE_ACTIVE;

    printf("✓ Power management initialized\n");
    printf("  Wake sources: RTC timer\n");

    return 0;
}

/**
 * @brief Enter sleep mode for specified duration
 *
 * @param sleep_seconds Duration to sleep in seconds
 * @return 0 on success, -1 on failure
 */
int power_enter_sleep(uint32_t sleep_seconds)
{
    if (sleep_seconds == 0) {
        printf("WARNING: Sleep duration is 0, skipping sleep\n");
        return 0;
    }

    printf("Entering deep sleep mode for %lu seconds...\n", sleep_seconds);

    // Configure RTC wake-up timer
    hx_drv_pmu_set_rtc_wakeup(sleep_seconds);

    // Save current state
    current_mode = POWER_MODE_DEEP_SLEEP;

    // Disable unused peripherals to save power
    // (Camera, UART, etc. will be re-initialized on wake)

    // Enter deep sleep
    // Current consumption in deep sleep: ~10 µA
    hx_drv_pmu_enter_deep_sleep();

    // System wakes up here
    printf("Woke up from deep sleep\n");

    // Restore active mode
    current_mode = POWER_MODE_ACTIVE;

    // Re-enable peripherals
    // (Handled by application layer - camera_init, lorawan_init, etc.)

    return 0;
}

/**
 * @brief Set power mode
 *
 * @param mode Target power mode
 * @return 0 on success, -1 on failure
 */
int power_set_mode(power_mode_t mode)
{
    printf("Changing power mode: %d -> %d\n", current_mode, mode);

    switch (mode) {
        case POWER_MODE_ACTIVE:
            // Full power, all peripherals enabled
            hx_drv_pmu_set_mode(PMU_MODE_ACTIVE);
            break;

        case POWER_MODE_IDLE:
            // CPU in low-power idle, peripherals running
            hx_drv_pmu_set_mode(PMU_MODE_IDLE);
            break;

        case POWER_MODE_SLEEP:
            // Light sleep, can wake quickly
            hx_drv_pmu_set_mode(PMU_MODE_SLEEP);
            break;

        case POWER_MODE_DEEP_SLEEP:
            // Deep sleep, lowest power consumption
            // Use power_enter_sleep() instead for controlled sleep
            printf("WARNING: Use power_enter_sleep() for deep sleep\n");
            return -1;

        default:
            printf("ERROR: Invalid power mode %d\n", mode);
            return -1;
    }

    current_mode = mode;
    return 0;
}

/**
 * @brief Get current power mode
 *
 * @return Current power mode
 */
power_mode_t power_get_mode(void)
{
    return current_mode;
}

/**
 * @brief Get battery voltage
 *
 * @return Battery voltage in millivolts, or -1 on error
 */
int32_t power_get_battery_voltage(void)
{
    // Read battery voltage from ADC
    uint32_t adc_value = hx_drv_pmu_get_battery_voltage();

    // Convert ADC value to millivolts
    // Assuming 12-bit ADC with 3.3V reference and voltage divider
    int32_t voltage_mv = (adc_value * 3300) / 4096;

    // Apply voltage divider correction (if applicable)
    // For example, if using 2:1 divider: voltage_mv *= 2;

    return voltage_mv;
}

/**
 * @brief Get battery percentage
 *
 * @return Battery percentage (0-100), or -1 on error
 */
int8_t power_get_battery_percentage(void)
{
    int32_t voltage_mv = power_get_battery_voltage();

    if (voltage_mv < 0) {
        return -1;
    }

    // Calculate percentage based on voltage
    // Typical Li-ion discharge curve:
    // 4200mV = 100%
    // 3700mV = 50%
    // 3300mV = 10%
    // 3000mV = 0%

    const int32_t V_MAX = 4200;
    const int32_t V_MIN = 3000;

    if (voltage_mv >= V_MAX) {
        return 100;
    }

    if (voltage_mv <= V_MIN) {
        return 0;
    }

    // Linear approximation
    int8_t percentage = ((voltage_mv - V_MIN) * 100) / (V_MAX - V_MIN);

    return percentage;
}

/**
 * @brief Estimate remaining battery life in hours
 *
 * @param reading_interval_sec Interval between readings in seconds
 * @return Estimated hours remaining, or -1 on error
 */
int32_t power_estimate_battery_life(uint32_t reading_interval_sec)
{
    int8_t battery_pct = power_get_battery_percentage();

    if (battery_pct < 0) {
        return -1;
    }

    // Estimate based on typical consumption
    // Assumptions:
    // - Battery capacity: 2000 mAh
    // - Deep sleep current: 10 µA
    // - Active current during reading: 100 mA for 10 seconds
    // - Reading interval: reading_interval_sec

    const float BATTERY_CAPACITY_MAH = 2000.0f;
    const float SLEEP_CURRENT_MA = 0.01f;  // 10 µA
    const float ACTIVE_CURRENT_MA = 100.0f;
    const float ACTIVE_TIME_SEC = 10.0f;

    // Calculate average current
    float sleep_time_sec = reading_interval_sec - ACTIVE_TIME_SEC;
    float avg_current_ma = (SLEEP_CURRENT_MA * sleep_time_sec +
                           ACTIVE_CURRENT_MA * ACTIVE_TIME_SEC) / reading_interval_sec;

    // Calculate remaining capacity
    float remaining_capacity_mah = BATTERY_CAPACITY_MAH * (battery_pct / 100.0f);

    // Calculate hours remaining
    int32_t hours_remaining = (int32_t)(remaining_capacity_mah / avg_current_ma);

    return hours_remaining;
}

/**
 * @brief Get power statistics
 *
 * @param stats Output power statistics structure
 * @return 0 on success, -1 on failure
 */
int power_get_stats(power_stats_t* stats)
{
    if (stats == NULL) {
        return -1;
    }

    stats->mode = current_mode;
    stats->battery_voltage_mv = power_get_battery_voltage();
    stats->battery_percentage = power_get_battery_percentage();
    stats->uptime_seconds = hx_drv_timer_get_ms() / 1000;

    return 0;
}

/**
 * @brief Enable/disable power monitoring
 *
 * @param enable 1 to enable, 0 to disable
 */
void power_set_monitoring(uint8_t enable)
{
    if (enable) {
        printf("Power monitoring enabled\n");
        // Enable periodic battery monitoring
    } else {
        printf("Power monitoring disabled\n");
        // Disable periodic battery monitoring
    }
}

/**
 * @brief Deinitialize power management
 */
void power_deinit(void)
{
    hx_drv_pmu_deinit();
    printf("Power management deinitialized\n");
}
