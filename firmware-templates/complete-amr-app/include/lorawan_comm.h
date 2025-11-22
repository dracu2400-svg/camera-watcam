/**
 * @file lorawan_comm.h
 * @brief LoRaWAN communication module header
 */

#ifndef LORAWAN_COMM_H
#define LORAWAN_COMM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize LoRaWAN communication
 *
 * @return 0 on success, -1 on failure
 */
int lorawan_init(void);

/**
 * @brief Configure LoRaWAN OTAA credentials
 *
 * @param dev_eui Device EUI (16 hex chars)
 * @param app_eui Application EUI (16 hex chars)
 * @param app_key Application Key (32 hex chars)
 * @return 0 on success, -1 on failure
 */
int lorawan_configure_otaa(const char* dev_eui, const char* app_eui, const char* app_key);

/**
 * @brief Join LoRaWAN network
 *
 * @param max_attempts Maximum join attempts
 * @return 0 on success, -1 on failure
 */
int lorawan_join(uint8_t max_attempts);

/**
 * @brief Send meter reading via LoRaWAN
 *
 * @param digits Array of digit values (0-9)
 * @param count Number of digits
 * @param confidence Average confidence score (0.0-1.0)
 * @return 0 on success, -1 on failure
 */
int lorawan_send_meter_reading(const uint8_t* digits, uint8_t count, float confidence);

/**
 * @brief Get LoRaWAN modem status
 *
 * @param rssi Output RSSI value (dBm)
 * @param snr Output SNR value (dB)
 * @return 0 on success, -1 on failure
 */
int lorawan_get_status(int16_t* rssi, int8_t* snr);

/**
 * @brief Check if joined to network
 *
 * @return 1 if joined, 0 if not joined
 */
int lorawan_is_joined(void);

/**
 * @brief Deinitialize LoRaWAN
 */
void lorawan_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // LORAWAN_COMM_H
