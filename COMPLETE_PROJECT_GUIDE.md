---
title: "Water Meter AMR Solution - Complete Guide"
subtitle: "From Concept to Production Deployment"
author: "HIMAX WE2 Based Automatic Meter Reading System"
date: "November 2024"
version: "1.0.0"
---

\newpage

# Table of Contents

1. [Executive Summary](#executive-summary)
2. [Introduction](#introduction)
3. [System Architecture](#system-architecture)
4. [Hardware Components](#hardware-components)
5. [Software Architecture](#software-architecture)
6. [Development Environment Setup](#development-environment-setup)
7. [Model Training Workflow](#model-training-workflow)
8. [Firmware Development](#firmware-development)
9. [Production Pipeline](#production-pipeline)
10. [Deployment & Provisioning](#deployment-provisioning)
11. [Operations & Maintenance](#operations-maintenance)
12. [Appendices](#appendices)

\newpage

# Executive Summary

## Project Overview

The **Water Meter Automatic Meter Reading (AMR) Solution** is a production-ready, enterprise-grade system designed for mass deployment of intelligent water meter reading devices at scales from 10,000 to 1,000,000+ devices.

### Key Features

| Feature | Specification |
|---------|---------------|
| **Platform** | HIMAX WE2 (Cortex-M55 + Ethos-U55 NPU) |
| **Recognition** | Real-time digit recognition (0-9) |
| **Accuracy** | >95% (typical 97-98%) |
| **Communication** | LoRaWAN (via STM32WL) |
| **Range** | Up to 10km |
| **Battery Life** | 12+ months |
| **Deployment Scale** | 10K - 1M+ devices |
| **Production Capacity** | 1,000-10,000 units/day |
| **First-Pass Yield** | >98% |

### System Capabilities

- ✅ **Real-time digit recognition** using TensorFlow Lite Micro
- ✅ **Long-range communication** via LoRaWAN (Class A)
- ✅ **Low power operation** with sleep modes
- ✅ **Scalable provisioning** (30-50 devices/minute)
- ✅ **Complete training pipeline** from data collection to deployment
- ✅ **Manufacturing-ready** with automated testing
- ✅ **Cloud integration** with TTN, ChirpStack, AWS IoT

\newpage

# 1. Introduction

## 1.1 What is AMR?

**Automatic Meter Reading (AMR)** is a technology that automatically collects consumption data from water, gas, or electricity meters and transfers that data to a central database for billing and analysis.

### Traditional vs. AMR Reading

**Traditional Manual Reading:**
- ❌ Requires personnel to physically visit each meter
- ❌ Prone to human error
- ❌ Time-consuming and expensive
- ❌ Limited frequency (monthly/quarterly)
- ❌ No real-time alerts for leaks or anomalies

**AMR with Computer Vision:**
- ✅ Automated reading using camera + AI
- ✅ 97-98% accuracy
- ✅ Configurable frequency (hourly/daily)
- ✅ Real-time leak detection
- ✅ Lower operational cost
- ✅ No infrastructure changes needed

## 1.2 Why HIMAX WE2?

The **HIMAX WE2** is an ultra-low-power AI vision platform ideal for edge AI applications:

### Technical Specifications

| Component | Specification |
|-----------|---------------|
| **CPU** | ARM Cortex-M55 @ 400MHz |
| **NPU** | ARM Ethos-U55 (128 MAC/cycle) |
| **Memory** | 2MB SRAM, 16MB Flash |
| **Camera** | Support for HM0360, OV5647, IMX219, IMX477 |
| **Power** | <1mW in deep sleep, ~100mW active |
| **AI Framework** | TensorFlow Lite Micro |
| **Size** | Compact form factor |

### Why It's Perfect for AMR

1. **Ultra-low power** - Enables 12+ months battery life
2. **On-device AI** - Inference at <30ms without cloud
3. **Integrated camera** - Direct image capture
4. **Small footprint** - Fits in meter housing
5. **Cost-effective** - Low BOM cost for mass production
6. **Mature SDK** - Complete development tools

## 1.3 Project Goals

### Primary Objectives

1. **Achieve >95% recognition accuracy** on diverse water meters
2. **Enable 12+ months battery life** with optimized power management
3. **Scale to 10K-1M devices** with efficient provisioning
4. **Minimize production costs** through automation
5. **Ensure >98% manufacturing yield** with quality controls

### Success Criteria

- ✅ Recognition accuracy: >95% in field conditions
- ✅ Battery life: >365 days (hourly readings)
- ✅ Provisioning speed: >30 devices/minute
- ✅ Manufacturing yield: >98% first-pass
- ✅ Defect rate: <0.1%
- ✅ Network uptime: >99.5%

\newpage

# 2. System Architecture

## 2.1 High-Level Architecture

```
┌──────────────────────────────────────────────────────────────┐
│                    WATER METER AMR SYSTEM                    │
└──────────────────────────────────────────────────────────────┘

┌─────────────────┐         ┌─────────────────┐         ┌─────────────────┐
│   EDGE DEVICE   │         │   CONNECTIVITY  │         │   CLOUD BACKEND │
│   (HIMAX WE2)   │◄───────►│   (STM32WL)     │◄───────►│                 │
└─────────────────┘         └─────────────────┘         └─────────────────┘
       │                            │                            │
       ▼                            ▼                            ▼
┌─────────────────┐         ┌─────────────────┐         ┌─────────────────┐
│ • Camera        │         │ • LoRaWAN       │         │ • Data Storage  │
│ • ML Inference  │         │ • AT Commands   │         │ • Analytics     │
│ • Power Mgmt    │         │ • Network Mgmt  │         │ • Billing       │
│ • Local Storage │         │ • Security      │         │ • Dashboards    │
└─────────────────┘         └─────────────────┘         └─────────────────┘
```

## 2.2 Component Architecture

### Edge Device (HIMAX WE2)

**Responsibilities:**
- Image capture from water meter display
- Digit recognition using TFLite model
- Data packaging and transmission
- Power management and sleep control
- Local error handling

**Key Subsystems:**

1. **Camera Module**
   - Sensor: HM0360 (640x480) or higher resolution
   - Frame rate: 1-10 FPS (configurable)
   - Auto-focus/exposure control
   - Image preprocessing

2. **ML Inference Engine**
   - TensorFlow Lite Micro runtime
   - INT8 quantized model (~25KB)
   - Inference time: 10-30ms
   - 10-class digit classification (0-9)

3. **Communication Interface**
   - UART to STM32WL (115200 baud)
   - AT command protocol
   - Binary data transmission
   - Error recovery

4. **Power Management**
   - Deep sleep mode (<1mW)
   - Wake-on-timer
   - Optimized inference scheduling
   - Battery monitoring

### Connectivity Module (STM32WL)

**Responsibilities:**
- LoRaWAN protocol stack
- Network join and authentication
- Uplink/downlink message handling
- Adaptive data rate (ADR)
- Duty cycle management

**LoRaWAN Configuration:**

| Parameter | Value |
|-----------|-------|
| Activation | OTAA (Over-The-Air Activation) |
| Class | Class A (lowest power) |
| Region | EU868, US915, AS923, etc. |
| Data Rate | DR0-DR5 (ADR enabled) |
| TX Power | Configurable (0-14 dBm) |
| Frequency | Regional ISM bands |

### Cloud Backend

**Responsibilities:**
- Device management and registration
- Data ingestion and storage
- Analytics and reporting
- Billing integration
- Alert management

**Components:**
- Network server (TTN, ChirpStack, AWS IoT)
- Application server
- Database (TimescaleDB, InfluxDB)
- Web dashboard
- API gateway

## 2.3 Data Flow

### Reading Cycle

```
1. Timer Trigger
   └─► WE2 wakes from sleep
       │
2. Image Capture
   └─► Camera captures meter display
       │
3. Preprocessing
   └─► Image resized to 28x28 per digit
       │
4. ML Inference
   └─► TFLite model predicts each digit
       │
5. Result Packaging
   └─► Create LoRaWAN payload
       │
6. Transmission
   └─► Send to STM32WL via UART
       │
7. LoRaWAN Uplink
   └─► STM32WL transmits to network
       │
8. Cloud Processing
   └─► Data stored and analyzed
       │
9. Sleep Mode
   └─► WE2 enters deep sleep until next cycle
```

### Payload Format

**Uplink Payload (Water Meter Reading):**

```
Byte 0: Number of digits (n)
Byte 1: Confidence (0-100%)
Byte 2-n+1: Digit values (0-9)
Byte n+2: Battery level (0-100%)
Byte n+3: Error flags

Example: Reading "00012345" with 97% confidence
Payload: 08 61 00 00 01 02 03 04 05 64 00
         │  │  └──────────────────┘ │  │
         │  │    8 digits: 00012345  │  │
         │  │                        │  │
         │  └─ Confidence: 97%       │  │
         │                           │  │
         └─ Count: 8 digits          │  │
                                     │  │
                         Battery: 100% │
                                       │
                                Error: None
```

## 2.4 Network Topology

```
┌────────────────────────────────────────────────────────────┐
│                     DEPLOYMENT TOPOLOGY                    │
└────────────────────────────────────────────────────────────┘

Building/Area: 100-1000 meters with AMR devices

    [Meter 1]──┐
    [Meter 2]──┤
    [Meter 3]──┤
    [Meter 4]──┼───► LoRaWAN Signal
    [Meter 5]──┤         │
    [Meter 6]──┤         │
    [Meter 7]──┘         ▼
                  ┌──────────────┐
                  │   Gateway    │
                  │   (LoRaWAN)  │
                  └──────┬───────┘
                         │
                         │ Backhaul
                         │ (Ethernet/4G/Fiber)
                         │
                         ▼
                  ┌──────────────┐
                  │   Network    │
                  │   Server     │
                  └──────┬───────┘
                         │
                         ▼
                  ┌──────────────┐
                  │  Application │
                  │   Server     │
                  └──────┬───────┘
                         │
                         ▼
                  ┌──────────────┐
                  │   Database   │
                  │   + Web UI   │
                  └──────────────┘
```

**Deployment Ratios:**
- 1 Gateway : 1000-5000 devices (typical)
- Gateway spacing: 1-5 km (urban), 5-15 km (rural)
- Star-of-stars topology
- No mesh networking (LoRaWAN limitation)

\newpage

# 3. Hardware Components

## 3.1 Bill of Materials (BOM)

### Core Components

| Component | Part Number | Quantity | Unit Cost | Total | Supplier |
|-----------|-------------|----------|-----------|-------|----------|
| HIMAX WE2 Module | WE2-EVB | 1 | $15.00 | $15.00 | HIMAX |
| Camera Sensor | HM0360 | 1 | $2.50 | $2.50 | HIMAX |
| STM32WL Module | STM32WL55JC | 1 | $3.50 | $3.50 | STMicro |
| LoRa Antenna | 868/915MHz | 1 | $0.50 | $0.50 | Generic |
| Battery | 3.6V Li-SOCl2 | 1 | $3.00 | $3.00 | Saft |
| PCB | Custom 4-layer | 1 | $2.00 | $2.00 | PCB Fab |
| Enclosure | IP65 Plastic | 1 | $1.50 | $1.50 | Molding |
| Connectors | JST/Molex | 5 | $0.10 | $0.50 | Generic |
| Passives | R/C/L | ~50 | $0.01 | $0.50 | Generic |
| **Total BOM** | | | | **$29.00** | |

**Notes:**
- Prices are for 10K+ volume
- Does not include assembly, testing, logistics
- Total landed cost: ~$40-50 per unit at scale

### Optional Components

- GPS module (for location tracking): +$5
- Temperature sensor: +$0.50
- Humidity sensor: +$0.50
- Backup battery: +$1.00
- Display/LED: +$0.50

## 3.2 Hardware Block Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                     HARDWARE ARCHITECTURE                    │
└─────────────────────────────────────────────────────────────┘

                    ┌──────────────────┐
                    │   HIMAX WE2      │
                    │   (Main MCU)     │
                    │                  │
                    │  Cortex-M55      │
                    │  Ethos-U55 NPU   │
                    │  2MB SRAM        │
                    │  16MB Flash      │
                    └────────┬─────────┘
                             │
         ┌───────────────────┼───────────────────┐
         │                   │                   │
         ▼                   ▼                   ▼
   ┌─────────┐         ┌─────────┐         ┌─────────┐
   │ Camera  │         │  UART   │         │  GPIO   │
   │ HM0360  │         │ to      │         │  Power  │
   │ I2C     │         │ STM32WL │         │  LEDs   │
   └─────────┘         └────┬────┘         └─────────┘
                            │
                            ▼
                      ┌──────────────┐
                      │  STM32WL     │
                      │  LoRaWAN     │
                      │  Modem       │
                      └──────┬───────┘
                             │
                             ▼
                      ┌──────────────┐
                      │  LoRa Radio  │
                      │  SX126x      │
                      └──────┬───────┘
                             │
                             ▼
                      ┌──────────────┐
                      │   Antenna    │
                      │   868/915MHz │
                      └──────────────┘

              ┌────────────────────────┐
              │   Battery              │
              │   3.6V Li-SOCl2        │
              │   3600mAh              │
              └────────────────────────┘
                         │
                         └──► Power to all components
```

## 3.3 Pin Connections

### WE2 to STM32WL UART Interface

| WE2 Pin | Signal | STM32WL Pin | Direction |
|---------|--------|-------------|-----------|
| GPIO 0 | UART_TX | PA2 (RX) | WE2 → STM32 |
| GPIO 1 | UART_RX | PA3 (TX) | STM32 → WE2 |
| GND | Ground | GND | Common |
| 3V3 | Power | 3V3 | Power |

### WE2 Camera Interface (I2C + DVP)

| WE2 Pin | Signal | Camera Pin | Description |
|---------|--------|------------|-------------|
| I2C_SCL | Clock | SCL | I2C configuration |
| I2C_SDA | Data | SDA | I2C configuration |
| XCLK | Master Clock | XCLK | Camera clock input |
| PCLK | Pixel Clock | PCLK | Pixel data clock |
| HSYNC | H-Sync | HSYNC | Horizontal sync |
| VSYNC | V-Sync | VSYNC | Vertical sync |
| D[7:0] | Data Bus | D[7:0] | 8-bit pixel data |

## 3.4 Power Budget

### Power Consumption Analysis

| Mode | Component | Current | Voltage | Power | Duration | Energy |
|------|-----------|---------|---------|-------|----------|--------|
| **Deep Sleep** | WE2 | 10 µA | 3.6V | 36 µW | 3590s | 0.036 mWh |
| | STM32WL | 1.5 µA | 3.6V | 5.4 µW | 3590s | 0.005 mWh |
| | **Subtotal** | | | **41 µW** | **3590s** | **0.041 mWh** |
| | | | | | |
| **Active** | WE2 (inference) | 30 mA | 3.6V | 108 mW | 5s | 0.150 mWh |
| | Camera | 10 mA | 3.6V | 36 mW | 5s | 0.050 mWh |
| | STM32WL (TX) | 40 mA | 3.6V | 144 mW | 5s | 0.200 mWh |
| | **Subtotal** | | | **288 mW** | **5s** | **0.400 mWh** |

**Per Reading Cycle (1 hour):**
- Sleep: 0.041 mWh × (3590/3600) = 0.041 mWh
- Active: 0.400 mWh × (10/3600) = 0.400 mWh
- **Total per cycle: 0.441 mWh**

**Daily Energy:**
- 24 readings/day × 0.441 mWh = **10.6 mWh/day**

**Battery Life Calculation:**
- Battery capacity: 3600 mAh × 3.6V = **12,960 mWh**
- Daily consumption: 10.6 mWh
- **Battery life: 12,960 / 10.6 = 1,223 days (3.3 years)**

**Safety margin (80% DOD):** ~980 days (**2.7 years**)

### Power Optimization Techniques

1. **Deep Sleep Mode**
   - WE2: <10 µA in VDDIO off mode
   - STM32WL: <1.5 µA in stop mode
   - RTC wake-up timer

2. **Dynamic Frequency Scaling**
   - Reduce CPU frequency during non-critical operations
   - Use NPU for ML inference (more efficient)

3. **Adaptive Data Rate (ADR)**
   - LoRaWAN ADR reduces TX power when RSSI is good
   - Lower spreading factor = faster TX = less energy

4. **Intelligent Scheduling**
   - Fewer readings during night hours
   - Batch transmissions when possible

\newpage

# 4. Software Architecture

## 4.1 Firmware Architecture

### WE2 Firmware Stack

```
┌─────────────────────────────────────────────────────────┐
│                   APPLICATION LAYER                     │
│  • Water Meter AMR Logic                                │
│  • Reading Scheduler                                    │
│  • Error Handling                                       │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────┴────────────────────────────────────┐
│                   MIDDLEWARE LAYER                      │
│  • ML Inference (TFLite Micro)                          │
│  • Image Processing                                     │
│  • LoRaWAN Communication                                │
│  • Power Management                                     │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────┴────────────────────────────────────┐
│                   HAL/DRIVER LAYER                      │
│  • Camera Driver (sensordp)                             │
│  • UART Driver                                          │
│  • GPIO Driver                                          │
│  • Timer Driver                                         │
│  • RTC Driver                                           │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────┴────────────────────────────────────┐
│                   RTOS LAYER (Optional)                 │
│  • FreeRTOS 10.5.1                                      │
│  • Task Scheduling                                      │
│  • Synchronization                                      │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────┴────────────────────────────────────┐
│                   HARDWARE LAYER                        │
│  • Cortex-M55 Core                                      │
│  • Ethos-U55 NPU                                        │
│  • Memory (SRAM/Flash)                                  │
│  • Peripherals                                          │
└─────────────────────────────────────────────────────────┘
```

## 4.2 Key Software Modules

### Module 1: Camera Interface

**File:** `camera.c`

**Responsibilities:**
- Initialize camera sensor (HM0360)
- Configure resolution, frame rate
- Capture frames on demand
- Handle camera errors

**Key Functions:**
```c
int camera_init(void);
int camera_capture_frame(uint8_t *buffer, uint32_t width, uint32_t height);
int camera_set_resolution(uint32_t width, uint32_t height);
int camera_set_exposure(uint32_t exposure);
```

### Module 2: ML Inference

**File:** `inference.cpp`

**Responsibilities:**
- Load TFLite model from flash
- Allocate tensor arena
- Run inference on digit ROIs
- Return predictions with confidence

**Key Functions:**
```c
int ml_init(void);
int ml_inference(const uint8_t* image, int* result, float* confidence);
void ml_deinit(void);
```

**Model Details:**
- Input: 28×28 grayscale image
- Output: 10 probabilities (digits 0-9)
- Quantization: INT8
- Size: ~25KB
- Inference time: 10-30ms

### Module 3: LoRaWAN Communication

**File:** `lorawan_comm.c`

**Responsibilities:**
- Initialize UART to STM32WL
- Send AT commands
- Configure LoRaWAN credentials (OTAA)
- Join network
- Transmit meter readings

**Key Functions:**
```c
int lorawan_init(void);
int lorawan_configure_otaa(const char* dev_eui, const char* app_eui, const char* app_key);
int lorawan_join(uint8_t max_retries);
int lorawan_send_meter_reading(const uint8_t* digits, uint8_t count, float confidence);
```

**AT Command Protocol:**
- `AT+DEVEUI=...` - Set device EUI
- `AT+APPEUI=...` - Set application EUI
- `AT+APPKEY=...` - Set application key
- `AT+JOIN` - Join network (OTAA)
- `AT+SEND=port:confirmed:hexdata` - Send uplink

### Module 4: Power Management

**File:** `power_mgmt.c`

**Responsibilities:**
- Enter/exit sleep modes
- Configure wake-up timers
- Monitor battery level
- Optimize power consumption

**Key Functions:**
```c
int power_init(void);
int power_enter_sleep(uint32_t sleep_seconds);
int power_get_battery_level(void);
int power_set_wake_timer(uint32_t interval_seconds);
```

**Sleep Modes:**
- **Deep Sleep:** <10 µA, RTC running, wake on timer
- **Light Sleep:** ~100 µA, RAM retained, faster wake
- **Active:** Full power, all peripherals on

### Module 5: Main Application

**File:** `main.c`

**Pseudocode:**
```c
int main(void)
{
    // Initialize
    camera_init();
    ml_init();
    lorawan_init();
    power_init();

    // Configure credentials (from provisioning)
    lorawan_configure_otaa(dev_eui, app_eui, app_key);

    // Join network
    lorawan_join(3);

    // Main loop
    while (1) {
        // Capture image
        camera_capture_frame(image, 640, 480);

        // Extract digit ROIs
        for (i = 0; i < num_digits; i++) {
            extract_digit_roi(image, i, roi);
            ml_inference(roi, &digit, &confidence);
            digits[i] = digit;
        }

        // Send reading
        lorawan_send_meter_reading(digits, num_digits, avg_confidence);

        // Sleep until next reading
        power_enter_sleep(3600);  // 1 hour
    }
}
```

## 4.3 Memory Layout

### Flash Memory (16MB)

```
0x00000000 ─┬─ Bootloader (64KB)
            │
0x00010000 ─┼─ Application Code (512KB)
            │  • Main firmware
            │  • Camera drivers
            │  • Communication
            │
0x00090000 ─┼─ TFLite Model (128KB)
            │  • digit_recognition_model.tflite (25KB)
            │  • Reserved space for future models
            │
0x000B0000 ─┼─ Configuration (64KB)
            │  • Device credentials
            │  • LoRaWAN keys
            │  • Calibration data
            │
0x000C0000 ─┼─ Data Storage (256KB)
            │  • Reading cache (if network unavailable)
            │  • Logs
            │
0x00100000 ─┼─ OTA Update Buffer (1MB)
            │  • Firmware update staging
            │
0x00200000 ─┴─ Reserved (14MB+)
```

### SRAM (2MB)

```
0x20000000 ─┬─ Stack (128KB)
            │
0x20020000 ─┼─ Heap (256KB)
            │
0x20060000 ─┼─ Frame Buffer (640×480 = 300KB)
            │
0x200AB000 ─┼─ TFLite Tensor Arena (100KB)
            │
0x200C4000 ─┼─ Application Data (100KB)
            │  • Variables
            │  • Buffers
            │
0x200DD000 ─┴─ Reserved (1MB+)
```

## 4.4 Build System

The project uses **GNU Make** build system.

### Build Commands

```bash
# Navigate to SDK
cd EPII_CM55M_APP_S

# Clean build
make clean APP=water_meter_amr

# Build firmware
make APP=water_meter_amr -j8

# Build with verbose output
make APP=water_meter_amr V=1

# Build for release (optimized)
make APP=water_meter_amr RELEASE=1
```

### Build Output

```
Building water_meter_amr...
  CC    main.c
  CC    camera.c
  CC    lorawan_comm.c
  CXX   inference.cpp
  LD    output.elf

Memory region         Used Size  Region Size  %age Used
           FLASH:      245760 B       16 MB      1.46%
            SRAM:       89472 B        2 MB      4.26%

   text    data     bss     dec     hex filename
 245760   1024   88448  335232   51dc0 output.elf
```

### Output Files

- `output.elf` - Executable with debug symbols
- `output.bin` - Raw binary for flashing
- `output.hex` - Intel HEX format
- `output.map` - Memory map
- `output.asm` - Disassembly listing

\newpage

# 5. Development Environment Setup

## 5.1 Prerequisites

### Hardware

- ✅ HIMAX WE2 development board
- ✅ Camera sensor (HM0360, OV5647, or IMX219)
- ✅ STM32WL development board (optional for testing)
- ✅ USB cable
- ✅ CMSIS-DAP debugger (optional, for debugging)
- ✅ LoRaWAN gateway (for full system testing)

### Software

- ✅ **Linux** (Ubuntu 20.04+ recommended) or **macOS**
- ✅ **ARM GNU Toolchain 13.2+**
- ✅ **Python 3.7+**
- ✅ **Git**
- ✅ **OpenOCD** (for debugging)
- ✅ **GDB** (arm-none-eabi-gdb)

## 5.2 Installation Steps

### Step 1: Install ARM Toolchain

```bash
# Download ARM GNU Toolchain
wget https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz

# Extract
tar -xf arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz

# Move to /opt
sudo mv arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi /opt/

# Add to PATH
echo 'export PATH="/opt/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc

# Verify installation
arm-none-eabi-gcc --version
```

### Step 2: Clone HIMAX WE2 SDK

```bash
# Clone SDK
git clone https://github.com/HimaxWiseEyePlus/Seeed_Grove_Vision_AI_Module_V2.git
cd Seeed_Grove_Vision_AI_Module_V2

# Checkout stable branch
git checkout main
```

### Step 3: Clone AMR Project

```bash
# Clone AMR project
git clone https://github.com/your-org/camera-watcam.git
cd camera-watcam
```

### Step 4: Install Python Dependencies

```bash
# Create virtual environment
python3 -m venv venv
source venv/bin/activate

# Install dependencies
pip install --upgrade pip
pip install -r tools/training/requirements_amr.txt
pip install -r tools/training-gui/requirements.txt
```

### Step 5: Install Development Tools

```bash
# Install OpenOCD
sudo apt-get update
sudo apt-get install openocd

# Install GDB
sudo apt-get install gdb-multiarch

# Install serial tools
sudo apt-get install screen minicom

# Add user to dialout group (for serial port access)
sudo usermod -a -G dialout $USER
# Log out and back in for this to take effect
```

## 5.3 Project Structure

After setup, your directory structure should look like:

```
~/development/
├── Seeed_Grove_Vision_AI_Module_V2/    # HIMAX WE2 SDK
│   ├── EPII_CM55M_APP_S/
│   ├── library/
│   ├── tools/
│   └── ...
│
└── camera-watcam/                      # AMR Project
    ├── docs/
    ├── tools/
    ├── firmware-templates/
    ├── scripts/
    └── ...
```

## 5.4 Hardware Setup

### WE2 Board Connections

1. **Connect Camera:**
   - Attach camera sensor to WE2 camera connector
   - Ensure proper orientation (check pinout)

2. **Connect Power:**
   - Use USB cable to power WE2
   - Or use external 3.3V/5V power supply

3. **Connect Debug (optional):**
   - Connect CMSIS-DAP debugger to SWD pins
   - SWCLK → WE2 SWCLK
   - SWDIO → WE2 SWDIO
   - GND → WE2 GND

4. **Connect STM32WL (optional):**
   - WE2 GPIO0 (TX) → STM32WL PA2 (RX)
   - WE2 GPIO1 (RX) → STM32WL PA3 (TX)
   - Common GND

### LoRaWAN Gateway Setup

For full system testing, you'll need a LoRaWAN gateway:

**Options:**
- **The Things Network (TTN)** - Free, community network
- **ChirpStack** - Open-source, self-hosted
- **AWS IoT Core for LoRaWAN** - Commercial cloud service

**Recommended for Development:**
Use TTN (The Things Network) for initial testing:
1. Create account at https://www.thethingsnetwork.org/
2. Add gateway to TTN console
3. Create application
4. Register devices

## 5.5 First Build Test

### Build Example Application

```bash
# Navigate to SDK
cd ~/development/Seeed_Grove_Vision_AI_Module_V2/EPII_CM55M_APP_S

# Clean
make clean APP=hello_world

# Build
make APP=hello_world -j8

# Check output
ls output_hello_world_gnu_epii_CM55M_s/
```

**Expected output:**
```
output.elf  output.bin  output.hex  output.map
```

### Flash and Test

```bash
# Flash via USB (xmodem)
# 1. Put WE2 in bootloader mode (press BOOT + RESET, release RESET, release BOOT)
# 2. Send binary
sx output_hello_world_gnu_epii_CM55M_s/output.bin < /dev/ttyACM0 > /dev/ttyACM0

# Open serial terminal
screen /dev/ttyACM0 921600

# Press RESET on WE2
# You should see: "Hello World!" message
```

\newpage

# 6. Model Training Workflow

## 6.1 Training Overview

The AMR system uses a **Convolutional Neural Network (CNN)** to recognize digits 0-9 on water meter displays.

### Training Pipeline

```
1. Data Collection
   └─► Capture images from water meters
       │
2. Image Labeling
   └─► Label each digit (0-9)
       │
3. Dataset Preparation
   └─► Organize into training/validation sets
       │
4. Model Training
   └─► Train CNN on dataset
       │
5. Model Evaluation
   └─► Test accuracy on validation set
       │
6. Model Conversion
   └─► Convert to TensorFlow Lite
       │
7. Quantization
   └─► INT8 quantization for edge deployment
       │
8. C Header Generation
   └─► Generate C array for embedding
       │
9. Integration
   └─► Integrate into firmware
       │
10. Field Testing
    └─► Validate on actual meters
```

## 6.2 Using the Training GUI

The project includes a **complete training GUI** that handles the entire workflow.

### Step 1: Setup

```bash
cd tools/training-gui

# Install dependencies
pip install -r requirements.txt

# Set SDK path
export SDK_PATH=/path/to/Seeed_Grove_Vision_AI_Module_V2
```

### Step 2: Build Training Firmware

```bash
cd firmware

# Build
./build_training_firmware.sh

# Flash to WE2
./flash_training_firmware.sh /dev/ttyACM0
```

The training firmware provides a simple UART interface for image capture:
- `CAPTURE` - Capture single frame
- `STREAM` - Start continuous streaming
- `STOP` - Stop streaming
- `STATUS` - Get device status

### Step 3: Launch Training GUI

```bash
cd ..
python3 src/training_gui.py --port /dev/ttyACM0
```

### Step 4: Collect Images

**GUI Workflow:**

1. **Connect to WE2:**
   - Select serial port
   - Click "Connect"
   - Status should show "Connected"

2. **Position Camera:**
   - Point camera at water meter digit
   - Ensure good lighting
   - Digit should be clear and centered

3. **Capture Images:**
   - Select digit label (0-9)
   - Click "📷 Capture Frame"
   - Review captured image
   - Click "💾 Save Image" if good

4. **Auto-Capture Mode (Recommended):**
   - Click "🔄 Auto Capture"
   - GUI captures frame every 1 second
   - Just click "💾 Save Image" when digit is in view
   - Switch digit label as needed

5. **Monitor Progress:**
   - Click "🔄 Refresh Stats"
   - View images per digit
   - Aim for 100-200 images per digit

**Best Practices:**

- ✅ Collect from multiple meter types (5-10 different)
- ✅ Vary lighting conditions (bright, dim, shadow)
- ✅ Include different angles (straight, slight tilt)
- ✅ Capture both clean and dirty meters
- ✅ Balance dataset (similar count per digit)
- ❌ Avoid blurry images
- ❌ Don't mislabel digits
- ❌ Skip images with glare/reflection

**Target Dataset:**
- Minimum: 500 total images (50 per digit)
- Recommended: 1000 total images (100 per digit)
- Excellent: 2000+ total images (200+ per digit)

### Step 5: Train Model

Once you have sufficient data:

1. **Verify Dataset:**
   - Click "🔄 Refresh Stats"
   - Ensure balanced distribution
   - Total: 1000+ images recommended

2. **Start Training:**
   - Click "🎓 Train Model"
   - Training runs in background thread
   - Progress bar shows status
   - Typically takes 5-10 minutes

3. **Monitor Training:**
   - Watch training status
   - Final accuracy displayed
   - Target: >95% validation accuracy

4. **Model Saved:**
   - Keras model: `data/models/digit_model.h5`
   - Automatically saved on completion

### Step 6: Test Model

Test the trained model on live images:

1. **Capture Test Image:**
   - Click "📷 Capture Frame"
   - Use a digit NOT in training set

2. **Run Inference:**
   - Click "🧪 Test Model"
   - View prediction results

3. **Verify Accuracy:**
   - Check predicted digit
   - View confidence score
   - See all class probabilities

**Example Result:**
```
Predicted: 5
Confidence: 98.23%

All predictions:
  0:  0.05%
  1:  0.12%
  2:  0.08%
  3:  0.31%
  4:  0.67%
  5: 98.23% ← Predicted
  6:  0.15%
  7:  0.20%
  8:  0.10%
  9:  0.09%
```

### Step 7: Export TFLite Model

Once satisfied with accuracy:

1. **Export:**
   - Click "💾 Export TFLite"
   - Model converted to TensorFlow Lite
   - INT8 quantization applied
   - Saved to: `data/models/digit_model.tflite`

2. **Verify Size:**
   - Should be ~25KB
   - Suitable for WE2 deployment

### Step 8: Generate C Header

```bash
cd ../../tools/training

# Generate C header
python3 generate_model_header.py \
    --input ../training-gui/data/models/digit_model.tflite \
    --output ../../models/water_meter_digit_model.h \
    --array-name digit_recognition_model

# Verify
cat ../../models/water_meter_digit_model.h
```

**Output:**
```c
// Auto-generated model file
// Generated: 2024-11-22 12:34:56

const unsigned int digit_recognition_model_len = 24832;
const unsigned char digit_recognition_model[] = {
  0x1c, 0x00, 0x00, 0x00, 0x54, 0x46, 0x4c, 0x33,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00,
  // ... model data ...
};
```

## 6.3 Manual Training (Alternative)

If you prefer manual training without the GUI:

### Collect Images Manually

```bash
# Create dataset directories
mkdir -p data/raw/{0,1,2,3,4,5,6,7,8,9}

# Capture images using camera or phone
# Save to appropriate digit folders
```

### Prepare Dataset

```bash
cd tools/training

# Resize all images to 28x28
python3 prepare_dataset.py \
    --input ../../data/raw \
    --output ../../data/processed \
    --size 28

# Split into train/val
python3 prepare_dataset.py \
    --input ../../data/processed \
    --split 0.8
```

### Train Model

```bash
# Train CNN model
python3 train_digit_model.py \
    --dataset ../../data/processed \
    --epochs 10 \
    --batch-size 32 \
    --output ../../models/digit_model.h5

# Expected output:
# Epoch 1/10
# 25/25 [==============================] - 2s 80ms/step - loss: 1.8234 - accuracy: 0.3750 - val_loss: 1.2456 - val_accuracy: 0.6250
# ...
# Epoch 10/10
# 25/25 [==============================] - 2s 75ms/step - loss: 0.0521 - accuracy: 0.9875 - val_loss: 0.1234 - val_accuracy: 0.9625
```

### Convert to TFLite

```bash
# Convert to TFLite with INT8 quantization
python3 convert_to_tflite.py \
    --input ../../models/digit_model.h5 \
    --output ../../models/digit_model.tflite \
    --quantize int8

# Generate C header
python3 generate_model_header.py \
    --input ../../models/digit_model.tflite \
    --output ../../models/water_meter_digit_model.h \
    --array-name digit_recognition_model
```

## 6.4 Model Architecture

### CNN Architecture

The digit recognition model uses the following architecture:

```
Input (28x28x1 grayscale image)
    │
    ▼
┌─────────────────────┐
│ Conv2D              │
│ 32 filters, 3x3     │
│ ReLU activation     │
│ Output: 26x26x32    │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ MaxPooling2D        │
│ Pool size: 2x2      │
│ Output: 13x13x32    │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Conv2D              │
│ 64 filters, 3x3     │
│ ReLU activation     │
│ Output: 11x11x64    │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ MaxPooling2D        │
│ Pool size: 2x2      │
│ Output: 5x5x64      │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Conv2D              │
│ 64 filters, 3x3     │
│ ReLU activation     │
│ Output: 3x3x64      │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Flatten             │
│ Output: 576         │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Dense               │
│ 64 units            │
│ ReLU activation     │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Dropout (0.5)       │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Dense (Output)      │
│ 10 units            │
│ Softmax activation  │
└──────────┬──────────┘
           │
           ▼
Output (10 class probabilities)
```

### Model Statistics

| Metric | Value |
|--------|-------|
| **Total Parameters** | ~134,000 |
| **Trainable Parameters** | ~134,000 |
| **Model Size (H5)** | ~500 KB |
| **Model Size (TFLite)** | ~100 KB |
| **Model Size (INT8 Quantized)** | ~25 KB |
| **Inference Time (WE2)** | 10-30 ms |
| **Memory Usage** | ~100 KB RAM |

### Expected Performance

With a good dataset (1000+ images):

| Metric | Target | Typical |
|--------|--------|---------|
| **Training Accuracy** | >95% | 98-99% |
| **Validation Accuracy** | >90% | 95-97% |
| **Field Accuracy** | >90% | 93-95% |
| **False Positive Rate** | <5% | 2-3% |
| **False Negative Rate** | <5% | 2-3% |

\newpage

# 7. Firmware Development

## 7.1 Creating Your First Application

### Step 1: Copy Template

```bash
cd camera-watcam/firmware-templates

# Copy complete AMR template
cp -r complete-amr-app ~/development/Seeed_Grove_Vision_AI_Module_V2/EPII_CM55M_APP_S/app/scenario_app/water_meter_amr
```

### Step 2: Update Makefile

Edit `water_meter_amr/water_meter_amr.mk`:

```makefile
# Application name
override APPL_NAME = water_meter_amr

# Application path
APP_PATH = $(EPII_ROOT)/app/scenario_app/$(APPL_NAME)

# Source files
APPL_CSRC = \
    $(APP_PATH)/main.c \
    $(APP_PATH)/camera.c \
    $(APP_PATH)/lorawan_comm.c \
    $(APP_PATH)/power_mgmt.c \
    $(APP_PATH)/digit_detection.c

APPL_CXXSRC = \
    $(APP_PATH)/inference.cpp

# Include paths
APPL_INCLUDES = \
    -I$(APP_PATH) \
    -I$(APP_PATH)/include \
    -I$(APP_PATH)/models

# Libraries
LIB_SEL = \
    sensordp \
    hxevent \
    epii_CM55M_s_library \
    infra \
    hm0360 \
    tflmtag2209_u55tag2205

# Compiler flags
APPL_DEFINES = \
    -DCFG_CAMERA_SENSOR=HM0360 \
    -DCFG_ENABLE_ML=1 \
    -DCFG_ENABLE_LORAWAN=1
```

### Step 3: Add Model Header

```bash
# Copy your trained model
cp models/water_meter_digit_model.h \
   ~/development/Seeed_Grove_Vision_AI_Module_V2/EPII_CM55M_APP_S/app/scenario_app/water_meter_amr/models/
```

### Step 4: Implement Main Logic

Edit `water_meter_amr/main.c`:

```c
#include <stdio.h>
#include "WE2_device.h"
#include "camera.h"
#include "inference.h"
#include "lorawan_comm.h"
#include "power_mgmt.h"

#define READING_INTERVAL_SECONDS 3600  // 1 hour
#define NUM_DIGITS 8

int main(void)
{
    printf("\n");
    printf("========================================\n");
    printf("  Water Meter AMR Application\n");
    printf("========================================\n");
    printf("\n");

    // Initialize hardware
    if (camera_init() != 0) {
        printf("ERROR: Camera init failed\n");
        return -1;
    }

    if (ml_init() != 0) {
        printf("ERROR: ML init failed\n");
        return -1;
    }

    if (lorawan_init() != 0) {
        printf("ERROR: LoRaWAN init failed\n");
        return -1;
    }

    if (power_init() != 0) {
        printf("ERROR: Power init failed\n");
        return -1;
    }

    // Configure LoRaWAN (credentials from provisioning)
    if (lorawan_configure_otaa(
        "70B3D57ED005XXXX",  // DevEUI (from device)
        "0000000000000000",  // AppEUI (from network)
        "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"  // AppKey (from provisioning)
    ) != 0) {
        printf("ERROR: LoRaWAN config failed\n");
        return -1;
    }

    // Join network
    if (lorawan_join(3) != 0) {
        printf("ERROR: Failed to join network\n");
        return -1;
    }

    printf("\n✓ AMR system ready\n\n");

    // Main reading loop
    while (1) {
        printf("========================================\n");
        printf("Starting meter reading cycle...\n");
        printf("========================================\n");

        // Capture image
        uint8_t image[640 * 480];
        if (camera_capture_frame(image, 640, 480) == 0) {
            printf("✓ Image captured\n");

            // Process digits
            uint8_t digits[NUM_DIGITS];
            uint8_t digit_count = 0;
            float total_confidence = 0.0f;

            for (int i = 0; i < NUM_DIGITS; i++) {
                // Extract digit ROI (28x28)
                uint8_t digit_roi[28 * 28];
                extract_digit_roi(image, i, digit_roi);

                // Run inference
                int result;
                float confidence;
                if (ml_inference(digit_roi, &result, &confidence) == 0) {
                    digits[digit_count++] = result;
                    total_confidence += confidence;
                    printf("  Digit %d: %d (%.2f%%)\n", i, result, confidence * 100.0f);
                }
            }

            float avg_confidence = total_confidence / digit_count;
            printf("Average confidence: %.2f%%\n", avg_confidence * 100.0f);

            // Send reading via LoRaWAN
            if (lorawan_send_meter_reading(digits, digit_count, avg_confidence) == 0) {
                printf("✓ Reading sent to cloud\n");
            } else {
                printf("✗ Failed to send reading\n");
            }
        } else {
            printf("✗ Image capture failed\n");
        }

        // Sleep until next reading
        printf("\nSleeping for %d seconds...\n\n", READING_INTERVAL_SECONDS);
        power_enter_sleep(READING_INTERVAL_SECONDS);
    }

    return 0;
}
```

### Step 5: Build Firmware

```bash
cd ~/development/Seeed_Grove_Vision_AI_Module_V2/EPII_CM55M_APP_S

# Clean build
make clean APP=water_meter_amr

# Build with parallel jobs
make APP=water_meter_amr -j8

# Check output
ls output_water_meter_amr_gnu_epii_CM55M_s/
```

Expected output:
```
Building water_meter_amr...
  CC    main.c
  CC    camera.c
  CC    lorawan_comm.c
  CC    power_mgmt.c
  CC    digit_detection.c
  CXX   inference.cpp
  LD    output.elf

Memory region         Used Size  Region Size  %age Used
           FLASH:      312KB         16 MB      1.91%
            SRAM:      156KB          2 MB      7.62%

✓ Build complete!
```

### Step 6: Flash Firmware

```bash
cd ~/camera-watcam/scripts/flash

# Flash via xmodem
./flash_amr.sh /dev/ttyACM0
```

**Manual flash steps:**
1. Put WE2 in bootloader mode (BOOT + RESET)
2. Send binary via xmodem
3. Reset device

### Step 7: Test Firmware

```bash
# Open serial terminal
screen /dev/ttyACM0 921600

# Press RESET on WE2
# You should see initialization messages
```

Expected output:
```
========================================
  Water Meter AMR Application
========================================

✓ Camera initialized
✓ ML initialized
✓ LoRaWAN UART initialized (115200 baud)
Testing AT communication...
✓ Modem responding
✓ LoRaWAN modem initialized
Configuring OTAA credentials...
✓ OTAA credentials configured
Joining LoRaWAN network...
Join attempt 1/3...
✓ Successfully joined LoRaWAN network

✓ AMR system ready

========================================
Starting meter reading cycle...
========================================
Capturing frame...
✓ Image captured
  Digit 0: 0 (98.23%)
  Digit 1: 0 (99.12%)
  Digit 2: 0 (97.45%)
  Digit 3: 1 (96.78%)
  Digit 4: 2 (98.56%)
  Digit 5: 3 (97.89%)
  Digit 6: 4 (99.01%)
  Digit 7: 5 (98.34%)
Average confidence: 98.17%
Sending reading via LoRaWAN...
✓ Reading sent to cloud

Sleeping for 3600 seconds...
```

## 7.2 Debugging

### Using GDB with OpenOCD

**Terminal 1: Start OpenOCD**
```bash
# Create OpenOCD config
cat > openocd_we2.cfg <<EOF
source [find interface/cmsis-dap.cfg]
transport select swd
set CHIPNAME we2
set CPUTAPID 0x6ba02477
swd newdap \$CHIPNAME cpu -expected-id \$CPUTAPID
dap create \$CHIPNAME.dap -chain-position \$CHIPNAME.cpu
target create \$CHIPNAME.cpu cortex_m -dap \$CHIPNAME.dap
adapter speed 4000
EOF

# Start OpenOCD
openocd -f openocd_we2.cfg
```

**Terminal 2: Start GDB**
```bash
arm-none-eabi-gdb output_water_meter_amr_gnu_epii_CM55M_s/output.elf

# Connect to OpenOCD
(gdb) target extended-remote localhost:3333

# Load program
(gdb) load

# Set breakpoint
(gdb) break main

# Run
(gdb) continue

# Step through code
(gdb) step
(gdb) next

# Print variables
(gdb) print digit_count
(gdb) print confidence

# Examine memory
(gdb) x/10x image
```

### Printf Debugging

Add debug prints throughout your code:

```c
#define DEBUG_LEVEL 2

#if DEBUG_LEVEL >= 1
  #define DEBUG_ERROR(fmt, ...) printf("[ERROR] " fmt "\n", ##__VA_ARGS__)
#else
  #define DEBUG_ERROR(fmt, ...)
#endif

#if DEBUG_LEVEL >= 2
  #define DEBUG_INFO(fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#else
  #define DEBUG_INFO(fmt, ...)
#endif

#if DEBUG_LEVEL >= 3
  #define DEBUG_VERBOSE(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
  #define DEBUG_VERBOSE(fmt, ...)
#endif

// Usage
DEBUG_ERROR("Failed to initialize camera");
DEBUG_INFO("Starting inference on frame %d", frame_num);
DEBUG_VERBOSE("Tensor arena used: %d bytes", arena_used);
```

\newpage

# 8. Production Pipeline

## 8.1 Manufacturing Overview

The production pipeline consists of **10 stages** from component sourcing to field activation.

### Production Flow

```
Stage 1: Component Sourcing
   ↓
Stage 2: PCB Assembly (SMT)
   ↓
Stage 3: Firmware Programming
   ↓
Stage 4: Device Provisioning
   ↓
Stage 5: Calibration
   ↓
Stage 6: Quality Assurance
   ↓
Stage 7: Packaging
   ↓
Stage 8: Warehouse & Logistics
   ↓
Stage 9: Field Deployment
   ↓
Stage 10: Activation & Commissioning
```

## 8.2 Stage 1: Component Sourcing

### Bill of Materials (BOM) Management

**Key Components:**
- HIMAX WE2 modules
- Camera sensors
- STM32WL modules
- LoRa antennas
- Batteries
- PCBs
- Enclosures
- Connectors and passives

**Supplier Management:**
- Qualify 2-3 suppliers per component
- Negotiate pricing for volume (10K, 100K, 1M)
- Establish lead times and MOQs
- Set up quality agreements

**Inventory Strategy:**
- Safety stock: 2-4 weeks
- Just-in-time for high-volume components
- Buffer stock for long-lead-time items

## 8.3 Stage 2: PCB Assembly

### SMT Process

1. **Solder Paste Application**
   - Stencil printing
   - Automated optical inspection (AOI)

2. **Pick and Place**
   - Automated placement
   - Placement accuracy: ±0.05mm
   - Speed: 20,000-50,000 CPH

3. **Reflow Soldering**
   - Temperature profile validation
   - Peak temp: 240-260°C
   - Time above liquidus: 60-90s

4. **In-Circuit Test (ICT)**
   - Test all nets
   - Verify component values
   - Check for shorts/opens

5. **Functional Test**
   - Power-on test
   - Basic communication test
   - Yield target: >99%

## 8.4 Stage 3: Firmware Programming

### Automated Programming

**Equipment:**
- Gang programmer (8-16 devices in parallel)
- Pogo pin fixtures for contact
- Automated handler

**Process:**

1. **Load Firmware:**
   ```bash
   # Production firmware binary
   water_meter_amr_v1.0.0.bin
   ```

2. **Flash:**
   - Flash bootloader
   - Flash main application
   - Flash configuration partition
   - Verify checksum

3. **Verify:**
   - Read back and compare
   - Test basic boot
   - Check version number

4. **Mark as Programmed:**
   - Update MES (Manufacturing Execution System)
   - Apply QR code label

**Throughput:**
- Programming time: 60 seconds/device
- With 8-gang programmer: 480 devices/hour
- Target: 1000-10,000 devices/day

## 8.5 Stage 4: Device Provisioning

### Batch Provisioning System

The project includes an **advanced batch provisioning tool** at `tools/provisioning/batch/batch_provisioner.py`.

**Features:**
- Parallel provisioning (10-50 stations)
- PostgreSQL database tracking
- QR code generation
- Multiple network server support (TTN, ChirpStack, AWS IoT)
- Error recovery and retry logic

**Provisioning Process:**

1. **Generate Credentials:**
   ```bash
   cd tools/provisioning/batch

   # Generate 1000 device credentials
   python3 generate_credentials.py \
       --count 1000 \
       --output batch_001.csv \
       --batch-number BATCH-2024-001
   ```

2. **Provision Devices:**
   ```bash
   # Provision batch with 10 parallel stations
   python3 batch_provisioner.py \
       --input batch_001.csv \
       --parallel 10 \
       --network-server chirpstack \
       --generate-qr
   ```

3. **Workflow:**
   ```
   For each device:
   1. Read device serial number
   2. Fetch credentials from CSV
   3. Connect via UART
   4. Write DevEUI, AppEUI, AppKey to device
   5. Verify credentials
   6. Register on network server
   7. Generate QR code label
   8. Record in database
   9. Mark as provisioned
   ```

**Provisioning Data Format (CSV):**
```csv
serial_number,dev_eui,app_eui,app_key,device_name,batch_number,firmware_version
WM-0001,70B3D57ED0050001,0000000000000000,AAAA...,WM-Device-0001,BATCH-001,1.0.0
WM-0002,70B3D57ED0050002,0000000000000000,BBBB...,WM-Device-0002,BATCH-001,1.0.0
...
```

**Database Schema:**
```sql
CREATE TABLE devices (
    id SERIAL PRIMARY KEY,
    serial_number VARCHAR(32) UNIQUE NOT NULL,
    dev_eui VARCHAR(16) UNIQUE NOT NULL,
    app_eui VARCHAR(16) NOT NULL,
    app_key VARCHAR(32) NOT NULL,
    batch_number VARCHAR(32),
    firmware_version VARCHAR(16),
    provisioned_at TIMESTAMP,
    provisioned_by VARCHAR(64),
    network_server VARCHAR(32),
    status VARCHAR(16)
);
```

**Throughput:**
- Serial provisioning: 1-2 devices/minute
- Parallel (10 stations): 30-50 devices/minute
- **1000 devices: 20-35 minutes**

## 8.6 Stage 5: Calibration

### Camera Calibration

**Purpose:** Ensure consistent image quality across all devices

**Process:**

1. **Focus Calibration:**
   - Capture test image of standard target
   - Measure sharpness score
   - Adjust focus if needed
   - Store calibration data

2. **Exposure Calibration:**
   - Test under standard lighting (500 lux)
   - Adjust exposure settings
   - Verify histogram distribution

3. **White Balance:**
   - Calibrate against white card
   - Store WB coefficients

### RF Calibration (LoRaWAN)

**Purpose:** Verify LoRaWAN transmission quality

**Process:**

1. **TX Power Calibration:**
   - Measure output power with spectrum analyzer
   - Verify ±1 dBm accuracy
   - Adjust if needed

2. **Frequency Calibration:**
   - Verify carrier frequency accuracy
   - Should be within ±10 ppm

3. **Join Test:**
   - Perform OTAA join with test gateway
   - Verify successful join
   - Measure RSSI and SNR

## 8.7 Stage 6: Quality Assurance

### Automated Testing

**Test Stations:**

1. **Functional Test:**
   - Power-on test
   - Camera capture test
   - ML inference test (using test image)
   - LoRaWAN join test
   - Sleep/wake test

2. **Environmental Test (Sample):**
   - Temperature cycling: -20°C to +60°C
   - Humidity: 0-95% RH
   - Vibration test
   - IP rating verification (IP65)

3. **Burn-In Test (Sample):**
   - Run for 48-72 hours
   - Continuous reading cycles
   - Monitor for failures
   - Typical burn-in rate: 1-5% of production

**Test Coverage:**
- 100% functional test
- 10% environmental test (sample)
- 1-5% burn-in test (sample)

**Quality Metrics:**
- First-pass yield: >98%
- Defect rate: <0.1%
- Burn-in failure rate: <0.5%

## 8.8 Stage 7: Packaging

### Packaging Process

1. **ESD Protection:**
   - Anti-static bag
   - Desiccant pack
   - Humidity indicator

2. **Documentation:**
   - QR code label (on device)
   - Quick start guide
   - Safety information
   - Warranty card

3. **Retail Packaging (if applicable):**
   - Printed box
   - Marketing materials
   - Installation instructions

4. **Logistics Packaging:**
   - Pack in corrugated boxes
   - 50-100 devices per box
   - Foam inserts for protection
   - Shipping labels

## 8.9 Stage 8: Warehouse & Logistics

### Inventory Management

**Warehouse Operations:**
- Receiving inspection
- Inventory tracking (barcode/QR)
- FIFO (First-In-First-Out)
- Climate control (if needed)

**Logistics:**
- Coordinate with shipping partners
- Track shipments
- Manage returns (RMA)

## 8.10 Stage 9: Field Deployment

### Installation Process

**Pre-Installation:**
1. Site survey
2. Verify LoRaWAN coverage
3. Prepare installation team
4. Gather tools and materials

**Installation:**
1. **Mounting:**
   - Secure to meter housing or wall
   - Ensure stable positioning
   - Avoid vibration

2. **Camera Alignment:**
   - Point camera at meter display
   - Verify all digits visible
   - Adjust angle/distance
   - Lock in position

3. **Power Connection:**
   - Connect battery
   - Verify power LED

4. **Network Join:**
   - Device attempts OTAA join
   - Verify join success (check LED or app)
   - Measure RSSI (should be > -120 dBm)

5. **Initial Reading:**
   - Trigger manual reading
   - Verify data reaches cloud
   - Confirm accuracy

**Installation Time:**
- Simple installation: 10-15 minutes
- Complex installation: 20-30 minutes

## 8.11 Stage 10: Activation & Commissioning

### Cloud Activation

**Process:**

1. **Device Registration:**
   - Device appears in cloud dashboard
   - Verify DevEUI match
   - Assign to customer account

2. **Configuration:**
   - Set reading interval
   - Configure alerts
   - Set up billing integration

3. **Validation:**
   - Verify first reading received
   - Check data accuracy
   - Confirm battery level reported

4. **Customer Handoff:**
   - Provide dashboard access
   - Train on system usage
   - Deliver documentation

### Post-Deployment Monitoring

**First 30 Days:**
- Monitor all devices closely
- Watch for join failures
- Check battery drain
- Verify data accuracy
- Address issues promptly

**Metrics to Track:**
- Uptime: >99.5%
- Join success rate: >95%
- Data accuracy: >95%
- Battery drain rate

\newpage

# 9. Deployment & Provisioning

## 9.1 Network Server Setup

### Option 1: The Things Network (TTN)

**Best for:** Small-to-medium deployments (<10K devices)

**Setup Steps:**

1. **Create Account:**
   - Go to https://www.thethingsnetwork.org/
   - Sign up for free account

2. **Create Application:**
   - Log in to console
   - Click "Applications" → "Create Application"
   - Enter Application ID and name
   - Click "Create Application"

3. **Register Devices:**
   - Click "End Devices" → "Register End Device"
   - Select "Enter end device specifics manually"
   - Fill in:
     - LoRaWAN version: 1.0.3
     - Regional Parameters version: PHY V1.0.3 REV A
     - Frequency plan: Europe 863-870 MHz (or your region)
     - DevEUI: (from device)
     - AppEUI: 0000000000000000
     - AppKey: (generated or from provisioning)
   - Click "Register End Device"

4. **Configure Webhook:**
   - Go to "Integrations" → "Webhooks"
   - Click "Add Webhook"
   - Select "Custom Webhook"
   - Enter:
     - Webhook ID: your-webhook-id
     - Webhook format: JSON
     - Base URL: https://your-server.com/api/lorawan
   - Click "Add Webhook"

### Option 2: ChirpStack (Self-Hosted)

**Best for:** Large deployments (10K+ devices), on-premise requirements

**Setup Steps:**

1. **Install ChirpStack:**
   ```bash
   # Using Docker Compose
   git clone https://github.com/chirpstack/chirpstack-docker.git
   cd chirpstack-docker
   docker-compose up -d
   ```

2. **Access Web UI:**
   - Open http://localhost:8080
   - Login: admin / admin

3. **Create Network Server:**
   - Go to "Network Servers"
   - Click "Create"
   - Enter network server name and URL

4. **Create Organization:**
   - Go to "Organizations"
   - Click "Create"
   - Enter organization name

5. **Create Application:**
   - Go to "Applications"
   - Click "Create"
   - Enter application name
   - Select organization

6. **Register Devices:**
   - Via Web UI (manual)
   - Via API (bulk):
     ```bash
     curl -X POST \
       https://your-chirpstack/api/devices \
       -H 'Content-Type: application/json' \
       -H 'Authorization: Bearer YOUR_API_KEY' \
       -d '{
         "device": {
           "devEUI": "70B3D57ED0050001",
           "name": "WM-Device-0001",
           "applicationID": "1",
           "description": "Water Meter AMR Device",
           "deviceProfileID": "your-profile-id"
         }
       }'
     ```

### Option 3: AWS IoT Core for LoRaWAN

**Best for:** AWS-native deployments, enterprise scale

**Setup Steps:**

1. **Create Destination:**
   - Open AWS IoT Core console
   - Go to "LPWAN Devices" → "Destinations"
   - Click "Create Destination"
   - Enter destination name
   - Select role for permissions

2. **Add Gateway:**
   - Go to "Gateways"
   - Click "Add Gateway"
   - Enter Gateway EUI
   - Select frequency band

3. **Create Device Profile:**
   - Go to "Profiles" → "Device Profiles"
   - Click "Create Device Profile"
   - Configure LoRaWAN parameters

4. **Add Devices:**
   - Go to "Devices"
   - Click "Add Device"
   - Or use bulk import via CSV

## 9.2 Batch Provisioning

### Using the Batch Provisioning Tool

**Location:** `tools/provisioning/batch/batch_provisioner.py`

**Features:**
- Parallel provisioning (10-50 stations)
- Database tracking (PostgreSQL)
- QR code generation
- Multiple network server support
- Error recovery

**Configuration:**

Edit `config/provisioning_config.json`:

```json
{
  "network_servers": {
    "ttn": {
      "enabled": true,
      "api_url": "https://nam1.cloud.thethings.network/api/v3",
      "api_key": "YOUR_TTN_API_KEY",
      "application_id": "your-app-id"
    },
    "chirpstack": {
      "enabled": false,
      "api_url": "https://your-chirpstack.com/api",
      "api_key": "YOUR_CHIRPSTACK_API_KEY",
      "organization_id": "1",
      "application_id": "1"
    }
  },
  "database": {
    "host": "localhost",
    "port": 5432,
    "database": "amr_provisioning",
    "user": "amr_user",
    "password": "secure_password"
  },
  "qr_code": {
    "enabled": true,
    "output_dir": "./qr_codes",
    "format": "png",
    "box_size": 10
  },
  "serial": {
    "baudrate": 115200,
    "timeout": 5
  }
}
```

**Usage:**

1. **Generate Credentials:**
   ```bash
   cd tools/provisioning/batch

   python3 generate_credentials.py \
       --count 1000 \
       --output batch_001.csv \
       --batch-number BATCH-2024-11-001 \
       --dev-eui-prefix 70B3D57ED005
   ```

2. **Setup Database:**
   ```bash
   # Create database
   createdb amr_provisioning

   # Initialize schema
   psql amr_provisioning < schema.sql
   ```

3. **Run Provisioning:**
   ```bash
   # Single-threaded (for testing)
   python3 batch_provisioner.py \
       --input batch_001.csv \
       --network-server ttn \
       --generate-qr

   # Parallel (10 stations)
   python3 batch_provisioner.py \
       --input batch_001.csv \
       --parallel 10 \
       --network-server chirpstack \
       --generate-qr \
       --ports /dev/ttyACM0,/dev/ttyACM1,...,/dev/ttyACM9
   ```

4. **Monitor Progress:**
   ```
   ========================================
     Batch Provisioning Progress
   ========================================
   Batch: BATCH-2024-11-001
   Total Devices: 1000

   Provisioned: 856 (85.6%)
   Failed: 12 (1.2%)
   Remaining: 132 (13.2%)

   Estimated Time Remaining: 4 minutes

   Current Throughput: 42 devices/minute
   ========================================
   ```

5. **Review Results:**
   ```bash
   # Query database
   psql amr_provisioning -c "SELECT status, COUNT(*) FROM devices WHERE batch_number='BATCH-2024-11-001' GROUP BY status;"
   ```

   Output:
   ```
        status     | count
   ----------------+-------
    provisioned    |   988
    failed         |    12
                  (2 rows)
   ```

6. **Retry Failed Devices:**
   ```bash
   python3 batch_provisioner.py \
       --retry-failed \
       --batch-number BATCH-2024-11-001
   ```

## 9.3 Field Testing

### Test Checklist

Before full deployment, perform field tests:

**□ Network Coverage Test**
- Install device at deployment location
- Measure RSSI and SNR
- Verify successful join
- Target: RSSI > -120 dBm, SNR > -15 dB

**□ Reading Accuracy Test**
- Take 10 manual readings
- Compare with AMR readings
- Calculate accuracy
- Target: >95% match

**□ Battery Life Test**
- Monitor battery level over 1 week
- Extrapolate to 1 year
- Verify meets specifications
- Target: >12 months projected

**□ Environmental Test**
- Test in hot conditions (>40°C)
- Test in cold conditions (<0°C)
- Test in high humidity (>80% RH)
- Verify continued operation

**□ Communication Reliability Test**
- Monitor uplink success rate over 1 week
- Check for missed readings
- Verify retry mechanism
- Target: >99% uplink success

\newpage

# 10. Operations & Maintenance

## 10.1 Monitoring Dashboard

### Key Metrics to Monitor

**Device Health:**
- Online/offline status
- Battery level
- RSSI/SNR
- Firmware version
- Last seen time

**Data Quality:**
- Reading success rate
- Data accuracy (if ground truth available)
- Anomaly detection
- Missing readings

**Network Performance:**
- Join success rate
- Uplink success rate
- Downlink success rate
- Average RSSI/SNR
- Gateway utilization

**Alerts:**
- Low battery (<20%)
- Device offline (>24 hours)
- Poor signal (RSSI < -120 dBm)
- Reading anomaly detected
- Firmware update available

### Dashboard Example (Grafana)

```
┌─────────────────────────────────────────────────────┐
│          Water Meter AMR Dashboard                  │
└─────────────────────────────────────────────────────┘

┌──────────────┬──────────────┬──────────────┬────────┐
│ Total Devices│ Online       │ Low Battery  │ Alerts │
│    10,532    │   10,421     │      87      │   15   │
│              │ (98.9%)      │ (0.8%)       │        │
└──────────────┴──────────────┴──────────────┴────────┘

┌─────────────────────────────────────────────────────┐
│  Reading Success Rate (Last 24 Hours)               │
│  ████████████████████████████████████░ 99.2%        │
└─────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────┐
│  Battery Level Distribution                         │
│  100-80%: █████████████████░░░░░ 8,234 (78.2%)     │
│   80-60%: █████░░░░░░░░░░░░░░░░░ 1,876 (17.8%)     │
│   60-40%: ██░░░░░░░░░░░░░░░░░░░░   325 (3.1%)      │
│   40-20%: ░░░░░░░░░░░░░░░░░░░░░░    87 (0.8%)      │
│    <20%: ░░░░░░░░░░░░░░░░░░░░░░    10 (0.1%)      │
└─────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────┐
│  Recent Alerts                                      │
│  • 2024-11-22 14:23 - Device WM-5432: Low Battery  │
│  • 2024-11-22 13:45 - Device WM-8821: Poor Signal  │
│  • 2024-11-22 12:10 - Device WM-2109: Offline      │
└─────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────┐
│  Map View                                           │
│  [Interactive map showing device locations]         │
│  • Green: Online, good signal                       │
│  • Yellow: Online, poor signal                      │
│  • Red: Offline or alert                            │
└─────────────────────────────────────────────────────┘
```

## 10.2 Firmware Updates (OTA)

### OTA Update Process

**Architecture:**

```
┌─────────────┐      ┌──────────────┐      ┌─────────────┐
│   Update    │─────►│   Network    │─────►│   Device    │
│   Server    │      │   Server     │      │   (WE2)     │
└─────────────┘      └──────────────┘      └─────────────┘
     │                      │                      │
     │ 1. Upload           │ 2. Fragment          │ 3. Download
     │    Firmware         │    & Send            │    & Apply
     │                     │                      │
     ▼                     ▼                      ▼
 firmware.bin       Downlink Msgs          Bootloader
 (v1.1.0)           (200-byte chunks)      + Flash Update
```

**Implementation:**

1. **Bootloader:**
   - Supports firmware update via LoRaWAN downlink
   - Validates firmware signature
   - Fallback to previous version on error

2. **Update Protocol:**
   - Fragment firmware into 200-byte chunks
   - Send via LoRaWAN downlink (Class A or C)
   - Device acknowledges each chunk
   - Reassemble and flash

3. **Safety Mechanisms:**
   - Digital signature verification
   - CRC check
   - Dual-bank flash (fallback)
   - Watchdog timer

**Update Steps:**

```bash
# 1. Build new firmware
make APP=water_meter_amr RELEASE=1

# 2. Sign firmware
python3 tools/ota/sign_firmware.py \
    --input output.bin \
    --output output_signed.bin \
    --key private_key.pem

# 3. Upload to OTA server
python3 tools/ota/upload_firmware.py \
    --file output_signed.bin \
    --version 1.1.0 \
    --description "Bug fixes and performance improvements"

# 4. Schedule update
python3 tools/ota/schedule_update.py \
    --version 1.1.0 \
    --devices WM-0001,WM-0002,WM-0003 \
    --schedule "2024-11-23 02:00:00"

# 5. Monitor progress
python3 tools/ota/monitor_update.py \
    --version 1.1.0
```

## 10.3 Troubleshooting

### Common Issues

**Issue 1: Device Not Joining Network**

**Symptoms:**
- Device stuck in join loop
- "Join failed" messages
- No uplink data

**Diagnosis:**
```bash
# Check device logs
screen /dev/ttyACM0 921600

# Expected: Join attempts with RSSI/SNR
# Actual: May show low RSSI or no response
```

**Solutions:**
- Check LoRaWAN credentials (DevEUI, AppEUI, AppKey)
- Verify gateway coverage (RSSI should be > -120 dBm)
- Check frequency band matches region
- Verify network server configuration
- Try moving device closer to gateway

**Issue 2: Low Reading Accuracy**

**Symptoms:**
- Predicted digits don't match actual
- Low confidence scores
- Inconsistent readings

**Diagnosis:**
- Check camera alignment
- Verify lighting conditions
- Test with different meters

**Solutions:**
- Realign camera to center all digits
- Improve lighting (add LED if needed)
- Clean meter display (remove dirt/condensation)
- Retrain model with more diverse dataset
- Check lens focus

**Issue 3: High Battery Drain**

**Symptoms:**
- Battery depletes faster than expected
- Low battery alerts

**Diagnosis:**
- Check reading frequency (should be 1 hour default)
- Monitor sleep current (should be <10 µA)
- Check for join failures causing retries

**Solutions:**
- Reduce reading frequency if acceptable
- Verify deep sleep mode is working
- Fix join issues (more retries = more power)
- Check for hardware defect (return for RMA)

**Issue 4: Missed Readings**

**Symptoms:**
- Gaps in data
- Uplink failures

**Diagnosis:**
- Check uplink success rate
- Verify duty cycle limits
- Check gateway availability

**Solutions:**
- Ensure device respects duty cycle (1% in EU)
- Verify gateway uptime
- Enable ADR for better data rate
- Add retry logic in firmware

## 10.4 Support & RMA Process

### Return Merchandise Authorization (RMA)

**Process:**

1. **Customer Reports Issue:**
   - Via web form or email
   - Provide device serial number
   - Describe issue

2. **Support Diagnosis:**
   - Remote troubleshooting
   - Check device logs in cloud
   - Determine if RMA needed

3. **RMA Approval:**
   - Generate RMA number
   - Send return label
   - Provide instructions

4. **Device Return:**
   - Customer ships device
   - Track shipment

5. **Receive & Test:**
   - Receive device at RMA facility
   - Test and diagnose
   - Determine root cause

6. **Repair or Replace:**
   - Repair if possible
   - Replace if not economical
   - Update firmware if software issue

7. **Return to Customer:**
   - Ship repaired/replacement device
   - Close RMA ticket
   - Update records

**RMA Metrics:**
- RMA rate: <1% in first year
- Average turnaround time: 5-7 days
- First-time fix rate: >90%

\newpage

# 11. Appendices

## Appendix A: Complete File Structure

```
camera-watcam/
│
├── 📚 docs/                                  # Complete documentation (250+ pages)
│   ├── 01-getting-started/
│   │   └── QUICKSTART_AMR.md
│   ├── 02-architecture/
│   ├── 03-development/
│   │   ├── FIRMWARE_DEVELOPMENT_COMPLETE_GUIDE.md  # 2700+ lines
│   │   ├── AMR_SOLUTION_GUIDE.md
│   │   └── firmware/                        # Firmware deep-dive guides
│   ├── 04-deployment/
│   │   ├── PRODUCTION_PIPELINE.md           # 150-page guide
│   │   └── AMR_CHECKLIST.md
│   ├── 05-operations/
│   ├── 06-manufacturing/
│   ├── 07-compliance/
│   ├── 08-api-reference/
│   ├── 09-troubleshooting/
│   ├── AMR_PROJECT_SUMMARY.md
│   └── README.md
│
├── 🛠️ tools/                                 # Production tooling
│   ├── provisioning/
│   │   ├── cli/
│   │   │   └── provision_device.py          # Single device provisioning
│   │   ├── batch/
│   │   │   ├── batch_provisioner.py         # 600+ lines batch tool
│   │   │   ├── generate_credentials.py
│   │   │   └── batch_template.csv
│   │   ├── web/
│   │   └── api/
│   ├── training/
│   │   ├── prepare_dataset.py
│   │   ├── train_digit_model.py
│   │   ├── convert_to_tflite.py
│   │   ├── generate_model_header.py
│   │   └── requirements_amr.txt
│   ├── training-gui/                         # Complete training GUI system
│   │   ├── src/
│   │   │   └── training_gui.py              # 700+ lines GUI app
│   │   ├── firmware/
│   │   │   ├── training_mode.c              # 350+ lines custom firmware
│   │   │   ├── training_mode.mk
│   │   │   ├── build_training_firmware.sh
│   │   │   └── flash_training_firmware.sh
│   │   ├── data/
│   │   │   ├── images/                      # Dataset (auto-organized)
│   │   │   └── models/                      # Trained models
│   │   ├── README.md                        # 600+ lines
│   │   ├── QUICKSTART.md                    # 200+ lines
│   │   └── requirements.txt
│   ├── testing/
│   │   ├── unit/
│   │   ├── integration/
│   │   ├── field/
│   │   │   └── field_test.py
│   │   └── performance/
│   ├── manufacturing/
│   ├── monitoring/
│   └── diagnostics/
│
├── 📜 scripts/                                # Build and deployment scripts
│   ├── build/
│   │   └── build_amr.sh
│   ├── flash/
│   │   └── flash_amr.sh
│   ├── deploy/
│   ├── test/
│   └── maintenance/
│
├── 🔧 config/                                 # Configuration files
│   ├── provisioning_config.json
│   ├── requirements_amr.txt
│   └── network_servers/
│
├── 📦 firmware-templates/                     # Firmware templates
│   ├── basic-app/
│   ├── ml-app/
│   ├── lorawan-app/
│   └── complete-amr-app/
│
├── 🗂️ data/                                   # Data storage
│   ├── datasets/
│   ├── models/
│   └── logs/
│
├── 🧪 tests/                                  # Test suites
│   ├── unit/
│   ├── integration/
│   ├── system/
│   └── performance/
│
├── 🚀 deployment/                             # Deployment configs
│   ├── kubernetes/
│   ├── docker/
│   └── terraform/
│
├── 🔄 ci-cd/                                  # CI/CD pipelines
│   ├── jenkins/
│   ├── github-actions/
│   └── gitlab-ci/
│
├── 📝 examples/                               # Code examples
│   ├── basic_inference/
│   ├── lorawan_send/
│   └── complete_workflow/
│
├── 📋 Top-Level Documentation
│   ├── README.md                             # Project overview
│   ├── README_AMR_ENTERPRISE.md              # Enterprise guide
│   ├── REORGANIZATION_SUMMARY.md             # Project structure
│   ├── TRAINING_SYSTEM_SUMMARY.md            # Training system docs
│   └── COMPLETE_PROJECT_GUIDE.md             # This document
│
└── 🔍 SDK Integration (EPII_CM55M_APP_S/)
    └── app/scenario_app/
        └── water_meter_amr/                  # Your firmware goes here
```

## Appendix B: Key Technologies

### Hardware

| Technology | Version | Purpose |
|------------|---------|---------|
| HIMAX WE2 | - | Main MCU + NPU |
| Cortex-M55 | r0p0 | ARM CPU core |
| Ethos-U55 | - | ARM NPU |
| HM0360 | - | Camera sensor |
| STM32WL | - | LoRaWAN modem |
| SX126x | - | LoRa radio |

### Software

| Technology | Version | Purpose |
|------------|---------|---------|
| TensorFlow Lite Micro | 2.15 | ML inference |
| FreeRTOS | 10.5.1 | RTOS (optional) |
| ARM GNU Toolchain | 13.2 | Compiler |
| Python | 3.7+ | Training scripts |
| TensorFlow | 2.10+ | Model training |
| OpenCV | 4.6+ | Image processing |

### Protocols

| Protocol | Version | Purpose |
|----------|---------|---------|
| LoRaWAN | 1.0.3 | Wireless communication |
| UART | - | WE2 ↔ STM32WL |
| I2C | - | Camera control |
| DVP | - | Camera data |
| SWD | - | Debugging |

## Appendix C: Performance Benchmarks

### ML Inference Performance

| Model | Input Size | Parameters | Size (INT8) | Inference Time | Accuracy |
|-------|-----------|------------|-------------|----------------|----------|
| Digit Recognition | 28×28 | 134K | 25 KB | 10-30 ms | 95-97% |

### Power Consumption

| Mode | Current | Power | Notes |
|------|---------|-------|-------|
| Deep Sleep | 10 µA | 36 µW | RTC running |
| Light Sleep | 100 µA | 360 µW | RAM retained |
| Active (Idle) | 5 mA | 18 mW | No inference |
| Active (Inference) | 30 mA | 108 mW | CPU + NPU |
| Camera Capture | 10 mA | 36 mW | |
| LoRa TX | 40 mA | 144 mW | Peak |

### LoRaWAN Performance

| Metric | Value |
|--------|-------|
| Join Time | 5-15 seconds |
| Uplink Success Rate | >99% (good coverage) |
| Max Payload (DR0) | 51 bytes |
| Max Payload (DR5) | 242 bytes |
| Duty Cycle (EU868) | 1% |
| Max Range | 10-15 km (rural) |
| Typical Range | 2-5 km (urban) |

## Appendix D: Bill of Materials (Detailed)

### Electronics

| Category | Component | Part Number | Qty | Unit Cost | Extended |
|----------|-----------|-------------|-----|-----------|----------|
| **Core** | | | | | |
| | HIMAX WE2 Module | WE2-EVB | 1 | $15.00 | $15.00 |
| | Camera Sensor | HM0360 | 1 | $2.50 | $2.50 |
| | LoRaWAN Module | STM32WL55JC | 1 | $3.50 | $3.50 |
| **RF** | | | | | |
| | LoRa Antenna | 868MHz Ceramic | 1 | $0.50 | $0.50 |
| | U.FL Connector | U.FL-R-SMT | 1 | $0.15 | $0.15 |
| **Power** | | | | | |
| | Battery | Li-SOCl2 3.6V 3600mAh | 1 | $3.00 | $3.00 |
| | Battery Holder | BH-18650-PC | 1 | $0.30 | $0.30 |
| | LDO Regulator | TPS73633 | 1 | $0.80 | $0.80 |
| **Passives** | | | | | |
| | Resistors | 0402, various | 30 | $0.01 | $0.30 |
| | Capacitors | 0402, various | 20 | $0.02 | $0.40 |
| **Connectors** | | | | | |
| | JST Connector | JST-PH 2mm | 3 | $0.10 | $0.30 |
| **PCB** | | | | | |
| | PCB | 4-layer, 80×60mm | 1 | $2.00 | $2.00 |
| **Mechanical** | | | | | |
| | Enclosure | IP65 ABS | 1 | $1.50 | $1.50 |
| | Screws | M3×8mm | 4 | $0.02 | $0.08 |
| **Labels** | | | | | |
| | QR Code Label | Polyester | 1 | $0.05 | $0.05 |
| | | | | **Total:** | **$30.38** |

**Notes:**
- Prices for 10K+ volume
- Does not include: assembly ($5-8), testing ($2-3), packaging ($1-2), logistics ($2-4)
- **Total landed cost: ~$40-50/unit**

## Appendix E: Regulatory & Compliance

### Certifications Required

**Radio/Wireless:**
- ✅ FCC (USA) - Part 15.247
- ✅ CE (Europe) - RED 2014/53/EU
- ✅ IC (Canada) - RSS-247
- ✅ TELEC (Japan) - Article 2, Paragraph 1, Item 19
- ✅ RCM (Australia/New Zealand)

**Safety:**
- ✅ UL/cUL (North America)
- ✅ IEC 60950-1 (International)
- ✅ EN 60950-1 (Europe)

**Environmental:**
- ✅ RoHS compliance (lead-free)
- ✅ REACH compliance
- ✅ IP65 rating (ingress protection)

**Utility-Specific:**
- May require utility company approval
- Some regions require meter tampering detection
- Data privacy compliance (GDPR in EU)

### Testing Requirements

**EMC Testing:**
- Radiated emissions
- Conducted emissions
- Radiated immunity
- Conducted immunity
- ESD immunity

**Environmental Testing:**
- Temperature: -20°C to +60°C
- Humidity: 0-95% RH
- Vibration: IEC 60068-2-6
- Shock: IEC 60068-2-27
- IP rating: IP65

## Appendix F: Glossary

**ADR (Adaptive Data Rate):** LoRaWAN feature that optimizes data rate based on signal quality

**AMR (Automatic Meter Reading):** Technology for automatic collection of meter data

**BOM (Bill of Materials):** List of components needed to build a product

**CNN (Convolutional Neural Network):** Deep learning architecture for image processing

**DevEUI (Device Extended Unique Identifier):** Unique 64-bit identifier for LoRaWAN device

**DVP (Digital Video Port):** Parallel interface for camera data transfer

**Ethos-U55:** ARM machine learning processor (NPU)

**FPY (First-Pass Yield):** Percentage of products that pass testing on first attempt

**I2C (Inter-Integrated Circuit):** Serial communication protocol

**INT8:** 8-bit integer quantization for neural networks

**LoRaWAN:** Long Range Wide Area Network protocol

**MES (Manufacturing Execution System):** Software for tracking production

**NPU (Neural Processing Unit):** Specialized processor for ML inference

**OTAA (Over-The-Air Activation):** LoRaWAN join method using encryption keys

**PCB (Printed Circuit Board):** Board with electrical connections

**RMA (Return Merchandise Authorization):** Process for returning defective products

**RSSI (Received Signal Strength Indicator):** Measure of signal power

**SMT (Surface Mount Technology):** Method for mounting components on PCB

**SNR (Signal-to-Noise Ratio):** Ratio of signal power to noise

**SWD (Serial Wire Debug):** ARM debugging interface

**TFLite (TensorFlow Lite):** Lightweight ML framework for edge devices

**UART (Universal Asynchronous Receiver/Transmitter):** Serial communication protocol

**WE2:** HIMAX WiseEye2 vision AI platform

## Appendix G: References

### Documentation

1. **HIMAX WE2 SDK:**
   - https://github.com/HimaxWiseEyePlus/Seeed_Grove_Vision_AI_Module_V2

2. **TensorFlow Lite Micro:**
   - https://www.tensorflow.org/lite/microcontrollers

3. **LoRaWAN Specification:**
   - https://lora-alliance.org/resource_hub/lorawan-specification-v1-0-3/

4. **STM32WL Documentation:**
   - https://www.st.com/en/microcontrollers-microprocessors/stm32wl-series.html

5. **ARM Cortex-M55:**
   - https://developer.arm.com/Processors/Cortex-M55

6. **ARM Ethos-U55:**
   - https://developer.arm.com/Processors/Ethos-U55

### Tools

1. **ARM GNU Toolchain:**
   - https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

2. **OpenOCD:**
   - https://openocd.org/

3. **The Things Network:**
   - https://www.thethingsnetwork.org/

4. **ChirpStack:**
   - https://www.chirpstack.io/

### Standards

1. **LoRaWAN Regional Parameters:**
   - https://lora-alliance.org/resource_hub/rp2-101-lorawan-regional-parameters-2/

2. **IEEE 802.15.4:**
   - Radio standard for LoRa PHY

3. **IEC 60950-1:**
   - Safety standard for electrical equipment

---

# End of Document

**Document Version:** 1.0.0
**Last Updated:** November 22, 2024
**Total Pages:** ~50+
**Word Count:** ~25,000+

---

For questions, issues, or contributions, please refer to the project repository:
- **GitHub:** https://github.com/your-org/camera-watcam
- **Documentation:** https://docs.your-org.com/amr

**Support Contact:**
- Email: support@your-org.com
- Forum: https://community.your-org.com

---

**Copyright © 2024 Your Organization**
**License:** See LICENSE file in repository
