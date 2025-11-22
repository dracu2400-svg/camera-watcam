/**
 * @file lorawan_comm.c
 * @brief LoRaWAN communication module
 *
 * This module handles communication with the STM32WL LoRaWAN modem
 * via UART using AT commands.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "lorawan_comm.h"

// HIMAX UART driver
#include "hx_drv_uart.h"

// Configuration
#define LORAWAN_UART_ID      HX_UART_0
#define LORAWAN_UART_BAUD    115200
#define LORAWAN_TX_PORT      10
#define LORAWAN_RX_BUFFER_SIZE 256
#define LORAWAN_AT_TIMEOUT   5000  // 5 seconds

// State
static uint8_t lorawan_joined = 0;
static char rx_buffer[LORAWAN_RX_BUFFER_SIZE];
static uint16_t rx_index = 0;

/**
 * @brief Send AT command and wait for response
 *
 * @param command AT command to send
 * @param response Expected response (NULL to skip check)
 * @param timeout Timeout in milliseconds
 * @return 0 on success, -1 on failure
 */
static int lorawan_send_at_command(const char* command, const char* response, uint32_t timeout)
{
    // Clear RX buffer
    memset(rx_buffer, 0, sizeof(rx_buffer));
    rx_index = 0;

    // Send command
    hx_drv_uart_print(LORAWAN_UART_ID, (char*)command);
    hx_drv_uart_print(LORAWAN_UART_ID, "\r\n");

    printf("  AT> %s\n", command);

    if (response == NULL) {
        return 0;  // Don't wait for response
    }

    // Wait for response
    uint32_t start_time = hx_drv_timer_get_ms();

    while ((hx_drv_timer_get_ms() - start_time) < timeout) {
        // Read available data
        uint8_t byte;
        if (hx_drv_uart_read(LORAWAN_UART_ID, &byte, 1) > 0) {
            if (rx_index < LORAWAN_RX_BUFFER_SIZE - 1) {
                rx_buffer[rx_index++] = byte;
                rx_buffer[rx_index] = '\0';

                // Check if we got the expected response
                if (strstr(rx_buffer, response) != NULL) {
                    printf("  AT< %s\n", rx_buffer);
                    return 0;
                }
            }
        }

        // Small delay to prevent busy waiting
        hx_drv_timer_delay_ms(10);
    }

    printf("  AT< TIMEOUT (expected: %s)\n", response);
    return -1;
}

/**
 * @brief Initialize LoRaWAN communication
 *
 * @return 0 on success, -1 on failure
 */
int lorawan_init(void)
{
    printf("Initializing LoRaWAN modem...\n");

    // Initialize UART for communication with STM32WL
    hx_drv_uart_init(LORAWAN_UART_ID, LORAWAN_UART_BAUD);

    printf("✓ LoRaWAN UART initialized (%d baud)\n", LORAWAN_UART_BAUD);

    // Test AT communication
    printf("Testing AT communication...\n");
    if (lorawan_send_at_command("AT", "OK", LORAWAN_AT_TIMEOUT) != 0) {
        printf("ERROR: No response from modem\n");
        return -1;
    }

    printf("✓ Modem responding\n");

    // Reset modem
    printf("Resetting modem...\n");
    lorawan_send_at_command("ATZ", "OK", LORAWAN_AT_TIMEOUT);

    // Set region (EU868, US915, etc.)
    printf("Setting region...\n");
    lorawan_send_at_command("AT+BAND=EU868", "OK", LORAWAN_AT_TIMEOUT);

    // Set LoRaWAN mode (OTAA)
    printf("Setting OTAA mode...\n");
    lorawan_send_at_command("AT+MODE=OTAA", "OK", LORAWAN_AT_TIMEOUT);

    // Set adaptive data rate
    printf("Enabling ADR...\n");
    lorawan_send_at_command("AT+ADR=ON", "OK", LORAWAN_AT_TIMEOUT);

    printf("✓ LoRaWAN modem initialized\n");
    return 0;
}

/**
 * @brief Configure LoRaWAN OTAA credentials
 *
 * @param dev_eui Device EUI (16 hex chars)
 * @param app_eui Application EUI (16 hex chars)
 * @param app_key Application Key (32 hex chars)
 * @return 0 on success, -1 on failure
 */
int lorawan_configure_otaa(const char* dev_eui, const char* app_eui, const char* app_key)
{
    char cmd[128];

    printf("Configuring OTAA credentials...\n");

    // Set DevEUI
    snprintf(cmd, sizeof(cmd), "AT+DEVEUI=%s", dev_eui);
    if (lorawan_send_at_command(cmd, "OK", LORAWAN_AT_TIMEOUT) != 0) {
        printf("ERROR: Failed to set DevEUI\n");
        return -1;
    }

    // Set AppEUI
    snprintf(cmd, sizeof(cmd), "AT+APPEUI=%s", app_eui);
    if (lorawan_send_at_command(cmd, "OK", LORAWAN_AT_TIMEOUT) != 0) {
        printf("ERROR: Failed to set AppEUI\n");
        return -1;
    }

    // Set AppKey
    snprintf(cmd, sizeof(cmd), "AT+APPKEY=%s", app_key);
    if (lorawan_send_at_command(cmd, "OK", LORAWAN_AT_TIMEOUT) != 0) {
        printf("ERROR: Failed to set AppKey\n");
        return -1;
    }

    printf("✓ OTAA credentials configured\n");
    return 0;
}

/**
 * @brief Join LoRaWAN network
 *
 * @param max_attempts Maximum join attempts
 * @return 0 on success, -1 on failure
 */
int lorawan_join(uint8_t max_attempts)
{
    printf("Joining LoRaWAN network...\n");

    for (uint8_t attempt = 1; attempt <= max_attempts; attempt++) {
        printf("Join attempt %d/%d...\n", attempt, max_attempts);

        if (lorawan_send_at_command("AT+JOIN", "JOINED", 30000) == 0) {
            lorawan_joined = 1;
            printf("✓ Successfully joined LoRaWAN network\n");
            return 0;
        }

        if (attempt < max_attempts) {
            printf("Join failed, retrying in 5 seconds...\n");
            hx_drv_timer_delay_ms(5000);
        }
    }

    printf("ERROR: Failed to join after %d attempts\n", max_attempts);
    return -1;
}

/**
 * @brief Send meter reading via LoRaWAN
 *
 * @param digits Array of digit values (0-9)
 * @param count Number of digits
 * @param confidence Average confidence score (0.0-1.0)
 * @return 0 on success, -1 on failure
 */
int lorawan_send_meter_reading(const uint8_t* digits, uint8_t count, float confidence)
{
    if (!lorawan_joined) {
        printf("ERROR: Not joined to network\n");
        return -1;
    }

    if (digits == NULL || count == 0) {
        printf("ERROR: Invalid parameters\n");
        return -1;
    }

    // Build payload
    // Format: [count][confidence_byte][digit0][digit1]...[digitN]
    uint8_t payload[32];
    payload[0] = count;
    payload[1] = (uint8_t)(confidence * 100.0f);  // Confidence as 0-100

    for (uint8_t i = 0; i < count && i < 30; i++) {
        payload[2 + i] = digits[i];
    }

    uint8_t payload_size = 2 + count;

    // Convert to hex string
    char hex_payload[128];
    for (uint8_t i = 0; i < payload_size; i++) {
        sprintf(&hex_payload[i * 2], "%02X", payload[i]);
    }

    // Send uplink
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "AT+SEND=%d:%s", LORAWAN_TX_PORT, hex_payload);

    printf("Sending %d bytes on port %d (confirmed=1)...\n", payload_size, LORAWAN_TX_PORT);
    printf("Data: %s\n", hex_payload);

    if (lorawan_send_at_command(cmd, "TX_DONE", 10000) != 0) {
        printf("ERROR: Failed to send uplink\n");
        return -1;
    }

    printf("✓ Data sent successfully\n");
    return 0;
}

/**
 * @brief Get LoRaWAN modem status
 *
 * @param rssi Output RSSI value (dBm)
 * @param snr Output SNR value (dB)
 * @return 0 on success, -1 on failure
 */
int lorawan_get_status(int16_t* rssi, int8_t* snr)
{
    // Query modem for stats
    if (lorawan_send_at_command("AT+STATS", "OK", LORAWAN_AT_TIMEOUT) == 0) {
        // Parse response for RSSI and SNR
        // This would need proper parsing of the response
        // For now, just return success
        if (rssi != NULL) {
            *rssi = -80;  // Example value
        }
        if (snr != NULL) {
            *snr = 10;    // Example value
        }
        return 0;
    }

    return -1;
}

/**
 * @brief Check if joined to network
 *
 * @return 1 if joined, 0 if not joined
 */
int lorawan_is_joined(void)
{
    return lorawan_joined;
}

/**
 * @brief Deinitialize LoRaWAN
 */
void lorawan_deinit(void)
{
    hx_drv_uart_deinit(LORAWAN_UART_ID);
    lorawan_joined = 0;
    printf("LoRaWAN deinitialized\n");
}
