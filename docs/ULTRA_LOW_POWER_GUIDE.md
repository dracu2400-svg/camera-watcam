# Ultra-Low-Power Configuration Guide

**Achieving 5-10 Years Battery Life for Water Meter AMR**

Version: 1.0.0
Last Updated: 2024-11-22

---

## 📊 Power Consumption Analysis

### Current System (1-hour reading interval)

| Mode | Current | Duration (per hour) | Energy |
|------|---------|---------------------|---------|
| Deep sleep | 10 µA | 3,590 sec | 9.97 µAh |
| Wake + init | 50 mA | 2 sec | 27.78 µAh |
| Camera capture | 80 mA | 1 sec | 22.22 µAh |
| ML inference (8 digits) | 100 mA | 2 sec | 55.56 µAh |
| LoRaWAN TX | 120 mA | 5 sec | 166.67 µAh |
| **Total per hour** | - | 3,600 sec | **282.20 µAh** |
| **Average current** | **~282 µA** | - | - |

**Battery life with 2000 mAh:** 2000 mAh ÷ 0.282 mA = **7,092 hours = 295 days ≈ 10 months**

---

## 🎯 Target: 2-Hour Reading Interval

### Power Consumption (2-hour intervals)

| Mode | Current | Duration (per 2 hours) | Energy |
|------|---------|------------------------|---------|
| Deep sleep | 10 µA | 7,190 sec | 19.97 µAh |
| Wake + init | 50 mA | 2 sec | 27.78 µAh |
| Camera capture | 80 mA | 1 sec | 22.22 µAh |
| ML inference | 100 mA | 2 sec | 55.56 µAh |
| LoRaWAN TX | 120 mA | 5 sec | 166.67 µAh |
| **Total per 2 hours** | - | 7,200 sec | **292.20 µAh** |
| **Average current** | **~146 µA** | - | - |

**Battery life with 2000 mAh:** 2000 mAh ÷ 0.146 mA = **13,699 hours = 571 days ≈ 1.6 years**

**Configuration:**
```c
#define READING_INTERVAL_SECONDS 7200  // 2 hours
```

---

## 🚀 Ultra-Low-Power Optimizations

### Optimization Strategy

To achieve 5-10 years battery life, we need to:
1. **Minimize sleep current** (target: <5 µA)
2. **Reduce active time** (faster wake-to-sleep)
3. **Optimize LoRaWAN** (fewer transmissions)
4. **Use high-capacity batteries** (Li-SOCl₂)

### 1. Minimize Sleep Current

#### Hardware Optimizations

**Target: <5 µA deep sleep**

```c
// In power_mgmt.c - Enhanced deep sleep
void power_ultra_deep_sleep(uint32_t sleep_seconds)
{
    // Disable all unused peripherals
    hx_drv_uart_deinit(LORAWAN_UART_ID);
    hx_drv_uart_deinit(DEBUG_UART_ID);

    // Power down camera sensor
    hx_drv_cis_power_off();

    // Disable unused GPIO pull-ups/pull-downs
    hx_drv_gpio_disable_all_pulls();

    // Set unused pins to analog input (lowest power)
    hx_drv_gpio_set_unused_to_analog();

    // Disable debug interface
    hx_drv_debug_disable();

    // Configure RTC wakeup
    hx_drv_pmu_set_rtc_wakeup(sleep_seconds);

    // Enter deepest sleep mode
    hx_drv_pmu_enter_ultra_deep_sleep();
}
```

**Expected sleep current:** **3-5 µA** (vs 10 µA standard)

#### Circuit Optimizations

1. **Remove LED indicators** during sleep (if present)
2. **Add battery disconnect switch** for storage
3. **Use ultra-low quiescent current regulators** (<1 µA)
4. **Disable LoRaWAN modem during sleep** (if possible)

### 2. Reduce Active Time

#### Fast Boot Configuration

```c
// Optimize boot sequence for minimal active time
#define ENABLE_FAST_BOOT 1
#define SKIP_BOOTLOADER_DELAY 1
#define MINIMIZE_UART_OUTPUT 1

// Skip non-essential initializations
#define SKIP_USB_INIT 1
#define SKIP_I2C_SCAN 1
```

#### Optimized Reading Sequence

```c
// Streamlined reading cycle
int fast_reading_cycle(void)
{
    uint32_t start = hx_drv_timer_get_ms();

    // 1. Quick camera warm-up (200ms instead of 2s)
    camera_quick_init();

    // 2. Single-shot capture (no preview)
    camera_capture_fast(image);

    // 3. Parallel processing while transmitting
    // (Start LoRaWAN TX while doing inference)

    uint32_t end = hx_drv_timer_get_ms();
    printf("Reading cycle: %lu ms\n", end - start);

    return 0;
}
```

**Target active time:** **5 seconds** (vs 10 seconds)

### 3. Optimize LoRaWAN Transmission

#### Reduce TX Power When Possible

```c
// Adaptive TX power based on RSSI
void lorawan_optimize_tx_power(void)
{
    int16_t rssi = lorawan_get_rssi();

    if (rssi > -80) {
        // Good signal, use minimal TX power
        lorawan_set_tx_power(2);  // 2 dBm
    } else if (rssi > -100) {
        // Medium signal
        lorawan_set_tx_power(8);  // 8 dBm
    } else {
        // Weak signal, use max power
        lorawan_set_tx_power(14); // 14 dBm
    }
}
```

#### Use Unconfirmed Uplinks

```c
// Unconfirmed uplinks save ~50% TX energy
#define LORAWAN_CONFIRMED_UPLINK 0  // Disable confirmations

// But send confirmation every 24 readings for reliability
#define CONFIRM_EVERY_N_READINGS 24
```

#### Optimize Data Rate

```c
// Use highest data rate possible (less air time)
#define LORAWAN_DATARATE SF7  // Fastest (vs SF12 slowest)
// SF7: ~100ms TX time
// SF12: ~2000ms TX time
```

### 4. Additional Power Savings

#### Reduce Image Resolution

```c
// Use lower resolution for faster processing
#define CAMERA_WIDTH_ULTRA_LOW_POWER 320
#define CAMERA_HEIGHT_ULTRA_LOW_POWER 240

// Smaller image = faster capture & processing
// Still sufficient for digit recognition
```

#### Skip Readings if No Change

```c
// Only transmit if reading changed
static uint32_t last_reading = 0;

if (current_reading == last_reading) {
    // Skip transmission, reading unchanged
    skip_count++;

    // But force transmission every 24 hours
    if (skip_count >= 12) {  // 12 * 2h = 24h
        force_transmission = 1;
    }
}
```

---

## 📊 Ultra-Low-Power Configuration

### Optimized Power Profile (2-hour intervals)

| Mode | Current | Duration | Energy |
|------|---------|----------|---------|
| **Ultra deep sleep** | **5 µA** | 7,195 sec | **9.99 µAh** |
| Quick wake + init | 50 mA | 0.5 sec | 6.94 µAh |
| Fast camera capture | 80 mA | 0.5 sec | 11.11 µAh |
| ML inference | 100 mA | 2 sec | 55.56 µAh |
| LoRaWAN TX (SF7, low power) | 80 mA | 2 sec | 44.44 µAh |
| **Total per 2 hours** | - | **7,200 sec** | **128.04 µAh** |
| **Average current** | **~64 µA** | - | - |

### Battery Life Calculations

#### With 2000 mAh Battery
```
Battery life = 2000 mAh ÷ 0.064 mA
             = 31,250 hours
             = 1,302 days
             = 3.6 years
```

#### With 8500 mAh Battery (Li-SOCl₂)
```
Battery life = 8500 mAh ÷ 0.064 mA
             = 132,813 hours
             = 5,534 days
             = 15.2 years
```

#### With 19000 mAh Battery (D-cell Li-SOCl₂)
```
Battery life = 19000 mAh ÷ 0.064 mA
             = 296,875 hours
             = 12,370 days
             = 33.9 years
```

---

## 🔋 Battery Selection Guide

### For 5-Year Operation (Target: 64 µA average)

**Required capacity:**
```
5 years × 365 days × 24 hours × 0.064 mA = 2,803 mAh
```

**Recommended batteries:**

#### Option 1: AA Li-SOCl₂ (Lithium Thionyl Chloride)

**Model:** Saft LS14500 or Tadiran SL-760
- **Capacity:** 2600-2900 mAh
- **Voltage:** 3.6V
- **Temperature:** -60°C to +85°C
- **Self-discharge:** <1% per year
- **Expected life:** **5-6 years**
- **Cost:** ~$8-12 per cell
- **Configuration:** 1 cell (3.6V direct)

**Pros:**
- ✅ Perfect capacity for 5 years
- ✅ Extremely low self-discharge
- ✅ Wide temperature range
- ✅ No protection circuit needed
- ✅ Compact size (AA)

**Cons:**
- ❌ Non-rechargeable
- ❌ Lower peak current capability
- ❌ Must be replaced after life

#### Option 2: C-cell Li-SOCl₂

**Model:** Saft LS26500 or Tadiran SL-2770
- **Capacity:** 7700-8500 mAh
- **Voltage:** 3.6V
- **Temperature:** -60°C to +85°C
- **Expected life:** **10+ years**
- **Cost:** ~$15-20 per cell
- **Configuration:** 1 cell

**Pros:**
- ✅ 10+ year operation
- ✅ High capacity margin
- ✅ Excellent for outdoor installations

**Cons:**
- ❌ Larger size
- ❌ Higher cost

#### Option 3: Dual AA Li-SOCl₂ (High Peak Current)

**Model:** Saft LS14500 + HPC1520 (hybrid)
- **Capacity:** 2600 mAh (primary) + capacitor
- **Voltage:** 3.6V
- **Peak current:** Up to 1.5A
- **Expected life:** **5 years**

**Pros:**
- ✅ Better peak current handling
- ✅ Suitable for LoRaWAN bursts
- ✅ Longer life under pulse loads

---

### For 10-Year Operation (Target: 64 µA average)

**Required capacity:**
```
10 years × 365 days × 24 hours × 0.064 mA = 5,606 mAh
```

**Recommended batteries:**

#### Option 1: D-cell Li-SOCl₂

**Model:** Saft LS33600 or Tadiran SL-2780
- **Capacity:** 17000-19000 mAh
- **Voltage:** 3.6V
- **Expected life:** **15-20 years** (with margin)
- **Cost:** ~$25-35 per cell
- **Size:** D-cell (34mm × 61mm)

**Pros:**
- ✅ 15-20 year operation
- ✅ Huge capacity margin
- ✅ Excellent reliability
- ✅ Best for long-term outdoor deployment

**Cons:**
- ❌ Large size
- ❌ Higher cost
- ❌ Overkill for some applications

#### Option 2: 2× C-cell Li-SOCl₂ (Series)

**Configuration:** 2× Saft LS26500 in parallel
- **Total capacity:** 15400-17000 mAh
- **Voltage:** 3.6V
- **Expected life:** **15+ years**
- **Cost:** ~$30-40 total

---

## ⚙️ Configuration for Ultra-Low-Power Mode

### config_ultra_low_power.h

```c
/**
 * Ultra-Low-Power Configuration
 * Target: 5-10 year battery life
 */

//=============================================================================
// READING CONFIGURATION
//=============================================================================

// Reading interval: 2 hours
#define READING_INTERVAL_SECONDS 7200  // 2 hours = 7200 seconds

// Number of digits
#define NUM_DIGITS 8

//=============================================================================
// POWER OPTIMIZATION
//=============================================================================

// Enable ultra-low-power mode
#define ENABLE_ULTRA_LOW_POWER_MODE 1

// Target sleep current: <5 µA
#define ULTRA_DEEP_SLEEP_MODE 1
#define DISABLE_DEBUG_UART_IN_SLEEP 1
#define POWER_DOWN_CAMERA_IN_SLEEP 1
#define DISABLE_UNUSED_PERIPHERALS 1

// Fast boot for minimal active time
#define ENABLE_FAST_BOOT 1
#define SKIP_BOOTLOADER_DELAY 1
#define MINIMIZE_UART_OUTPUT 1

// Reduce active time
#define CAMERA_QUICK_INIT_MODE 1
#define CAMERA_WARMUP_TIME_MS 200  // Reduced from 2000ms

//=============================================================================
// LORAWAN OPTIMIZATION
//=============================================================================

// Use unconfirmed uplinks (saves 50% TX energy)
#define LORAWAN_CONFIRMED_UPLINK 0

// But confirm every N readings for reliability
#define CONFIRM_EVERY_N_READINGS 24  // Once per 48 hours

// Adaptive TX power
#define LORAWAN_ADAPTIVE_TX_POWER 1
#define LORAWAN_MIN_TX_POWER 2   // dBm (when signal is good)
#define LORAWAN_MAX_TX_POWER 14  // dBm (when signal is weak)

// Use fastest data rate possible
#define LORAWAN_PREFERRED_DATARATE SF7  // Fastest
#define LORAWAN_FALLBACK_DATARATE SF9   // If SF7 fails

// Disable ADR to save join time
#define LORAWAN_ADR_ENABLED 0

//=============================================================================
// CAMERA OPTIMIZATION
//=============================================================================

// Use lower resolution for faster processing
#define CAMERA_WIDTH_LOW_POWER 320
#define CAMERA_HEIGHT_LOW_POWER 240

// Or keep standard resolution but optimize capture
#define CAMERA_WIDTH 640
#define CAMERA_HEIGHT 480
#define CAMERA_SINGLE_SHOT_MODE 1  // No preview frames

//=============================================================================
// SMART TRANSMISSION
//=============================================================================

// Skip transmission if reading unchanged
#define ENABLE_SMART_TRANSMISSION 1
#define FORCE_TX_EVERY_N_READINGS 12  // Force TX every 24 hours

// Cache readings if network unavailable
#define ENABLE_READING_CACHE 1
#define MAX_CACHED_READINGS 24  // Cache up to 48 hours

//=============================================================================
// BATTERY MONITORING
//=============================================================================

// Monitor battery for low voltage
#define ENABLE_BATTERY_MONITORING 1
#define BATTERY_LOW_VOLTAGE_MV 3200
#define BATTERY_CRITICAL_VOLTAGE_MV 3000

// Send battery status with every reading
#define INCLUDE_BATTERY_IN_PAYLOAD 1

//=============================================================================
// DEBUG (Disable in production)
//=============================================================================

// Minimal debug output to save power
#define DEBUG_LEVEL 0  // 0 = Errors only
#define ENABLE_SERIAL_OUTPUT 0  // Disable serial in production
#define DISABLE_PRINTF_IN_PRODUCTION 1

//=============================================================================
// WATCHDOG
//=============================================================================

// Shorter watchdog for faster recovery
#define WATCHDOG_TIMEOUT_SECONDS 30

```

---

## 📈 Expected Battery Life Summary

### Standard Configuration (1-hour intervals)

| Battery Type | Capacity | Expected Life |
|--------------|----------|---------------|
| Li-ion 2000mAh | 2000 mAh | ~10 months |
| Li-ion 3000mAh | 3000 mAh | ~15 months |

### Optimized Configuration (2-hour intervals, optimizations)

| Battery Type | Capacity | Average Current | Expected Life |
|--------------|----------|-----------------|---------------|
| **AA Li-SOCl₂** | **2600 mAh** | **64 µA** | **~5 years** ✅ |
| **C-cell Li-SOCl₂** | **8500 mAh** | **64 µA** | **~15 years** ✅ |
| **D-cell Li-SOCl₂** | **19000 mAh** | **64 µA** | **~34 years** ✅ |

### Further Optimized (4-hour intervals)

| Battery Type | Capacity | Average Current | Expected Life |
|--------------|----------|-----------------|---------------|
| AA Li-SOCl₂ | 2600 mAh | **38 µA** | **~8 years** |
| C-cell Li-SOCl₂ | 8500 mAh | **38 µA** | **~25 years** |
| D-cell Li-SOCl₂ | 19000 mAh | **38 µA** | **~57 years** |

---

## 🛠️ Implementation Guide

### Step 1: Update Firmware Configuration

```bash
# Copy ultra-low-power config
cp config_ultra_low_power.h firmware-templates/complete-amr-app/config/config.h

# Edit main.c to use new interval
# Change: #define READING_INTERVAL_SECONDS 7200
```

### Step 2: Enable Power Optimizations

Add to `power_mgmt.c`:

```c
// Enable all power-saving features
void power_enable_ultra_low_power(void)
{
    // 1. Disable debug UART
    hx_drv_uart_deinit(DEBUG_UART_ID);

    // 2. Set all unused pins to analog input
    for (int pin = 0; pin < 64; pin++) {
        if (!is_pin_used(pin)) {
            hx_drv_gpio_set_analog(pin);
        }
    }

    // 3. Disable pull resistors
    hx_drv_gpio_disable_all_pulls();

    // 4. Power down camera between readings
    hx_drv_cis_power_off();

    // 5. Configure LoRaWAN modem for low power
    lorawan_set_low_power_mode(1);
}
```

### Step 3: Optimize LoRaWAN

Add to `lorawan_comm.c`:

```c
// Adaptive TX power
int lorawan_adaptive_tx_power(void)
{
    int16_t rssi;
    lorawan_get_status(&rssi, NULL);

    if (rssi > -80) {
        lorawan_send_at_command("AT+TXP=2", "OK", 1000);  // 2 dBm
    } else if (rssi > -100) {
        lorawan_send_at_command("AT+TXP=8", "OK", 1000);  // 8 dBm
    } else {
        lorawan_send_at_command("AT+TXP=14", "OK", 1000); // 14 dBm
    }

    return 0;
}
```

### Step 4: Select and Install Battery

**For 5-year operation:**
- **Recommended:** Saft LS14500 (AA Li-SOCl₂, 2600 mAh)
- **Alternative:** Tadiran SL-760 (AA Li-SOCl₂, 2400 mAh)

**For 10-year operation:**
- **Recommended:** Saft LS26500 (C-cell Li-SOCl₂, 8500 mAh)
- **Alternative:** Saft LS33600 (D-cell Li-SOCl₂, 19000 mAh)

### Step 5: Test Power Consumption

```bash
# Measure sleep current
# Expected: <5 µA

# Measure active current during reading
# Expected: ~80-100 mA for 5 seconds

# Calculate actual average
# Target: <70 µA for 5-year life
```

---

## ✅ Checklist for Ultra-Low-Power Deployment

### Hardware

- [ ] Use Li-SOCl₂ battery (AA, C, or D-cell)
- [ ] Remove/disable LED indicators
- [ ] Add battery connector with polarity protection
- [ ] Use ultra-low quiescent LDO regulator
- [ ] Disable unused circuits on PCB

### Firmware

- [ ] Set READING_INTERVAL_SECONDS to 7200 (2 hours)
- [ ] Enable ULTRA_DEEP_SLEEP_MODE
- [ ] Disable debug UART in production
- [ ] Enable LORAWAN_ADAPTIVE_TX_POWER
- [ ] Set LORAWAN_CONFIRMED_UPLINK to 0
- [ ] Enable ENABLE_SMART_TRANSMISSION
- [ ] Minimize printf() statements

### Testing

- [ ] Measure sleep current (<5 µA target)
- [ ] Measure active time (<5 seconds target)
- [ ] Test LoRaWAN reliability (>99% success)
- [ ] Verify battery voltage monitoring
- [ ] Test in actual deployment temperature range

### Deployment

- [ ] Label device with battery install date
- [ ] Record expected replacement date
- [ ] Set up battery low voltage alerts
- [ ] Monitor first month for issues

---

## 📞 Support

For questions about ultra-low-power configuration:
- See [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md)
- See [Power Management Module](firmware-templates/complete-amr-app/src/power_mgmt.c)

---

**Achieve 5-10 years battery life with proper configuration and battery selection!** ⚡🔋
