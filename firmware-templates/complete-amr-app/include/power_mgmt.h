/**
 * @file power_mgmt.h
 * @brief Power management module header
 */

#ifndef POWER_MGMT_H
#define POWER_MGMT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Power modes
 */
typedef enum {
    POWER_MODE_ACTIVE,      // Full power
    POWER_MODE_IDLE,        // CPU idle, peripherals active
    POWER_MODE_SLEEP,       // Light sleep
    POWER_MODE_DEEP_SLEEP   // Deep sleep
} power_mode_t;

/**
 * @brief Power statistics structure
 */
typedef struct {
    power_mode_t mode;
    int32_t battery_voltage_mv;
    int8_t battery_percentage;
    uint32_t uptime_seconds;
} power_stats_t;

/**
 * @brief Initialize power management
 *
 * @return 0 on success, -1 on failure
 */
int power_init(void);

/**
 * @brief Enter sleep mode for specified duration
 *
 * @param sleep_seconds Duration to sleep in seconds
 * @return 0 on success, -1 on failure
 */
int power_enter_sleep(uint32_t sleep_seconds);

/**
 * @brief Set power mode
 *
 * @param mode Target power mode
 * @return 0 on success, -1 on failure
 */
int power_set_mode(power_mode_t mode);

/**
 * @brief Get current power mode
 *
 * @return Current power mode
 */
power_mode_t power_get_mode(void);

/**
 * @brief Get battery voltage
 *
 * @return Battery voltage in millivolts, or -1 on error
 */
int32_t power_get_battery_voltage(void);

/**
 * @brief Get battery percentage
 *
 * @return Battery percentage (0-100), or -1 on error
 */
int8_t power_get_battery_percentage(void);

/**
 * @brief Estimate remaining battery life in hours
 *
 * @param reading_interval_sec Interval between readings in seconds
 * @return Estimated hours remaining, or -1 on error
 */
int32_t power_estimate_battery_life(uint32_t reading_interval_sec);

/**
 * @brief Get power statistics
 *
 * @param stats Output power statistics structure
 * @return 0 on success, -1 on failure
 */
int power_get_stats(power_stats_t* stats);

/**
 * @brief Enable/disable power monitoring
 *
 * @param enable 1 to enable, 0 to disable
 */
void power_set_monitoring(uint8_t enable);

/**
 * @brief Deinitialize power management
 */
void power_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // POWER_MGMT_H
