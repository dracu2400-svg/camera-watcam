# Complete Firmware Development Guide for Water Meter AMR

**Target Platform**: HIMAX WE2 (Cortex-M55) + STM32WL (Cortex-M4)
**IDE**: Command-line build system (Make)
**Toolchain**: ARM GNU Toolchain 13.2
**RTOS**: FreeRTOS 10.5.1
**ML Framework**: TensorFlow Lite Micro

**Document Version**: 1.0.0
**Last Updated**: 2024-11-22
**Audience**: Firmware developers, embedded engineers

---

## Table of Contents

1. [Firmware Architecture Overview](#firmware-architecture-overview)
2. [Development Environment Setup](#development-environment-setup)
3. [Project Structure Deep Dive](#project-structure-deep-dive)
4. [Creating Your First Application](#creating-your-first-application)
5. [Camera Integration](#camera-integration)
6. [ML Model Integration](#ml-model-integration)
7. [LoRaWAN Communication](#lorawan-communication)
8. [Power Management](#power-management)
9. [Build System](#build-system)
10. [Debugging and Testing](#debugging-and-testing)
11. [Advanced Topics](#advanced-topics)
12. [Best Practices](#best-practices)

---

## Firmware Architecture Overview

### System Architecture Diagram

```
┌────────────────────────────────────────────────────────────┐
│                    WE2 FIRMWARE STACK                      │
└────────────────────────────────────────────────────────────┘

Application Layer:
┌─────────────────────────────────────────────────────────┐
│  Water Meter AMR Application                             │
│  ├── Main Loop (main.c)                                 │
│  ├── Digit Recognition (recognition.c)                  │
│  ├── UART Communication (uart_comm.c)                   │
│  └── Configuration (config.h)                           │
└────────────┬────────────────────────────────────────────┘
             │
Middleware Layer:
┌────────────▼────────────────────────────────────────────┐
│  ├── TensorFlow Lite Micro (inference engine)           │
│  ├── Image Processing (preprocessing)                   │
│  ├── FreeRTOS (task scheduling)                        │
│  └── Communication Protocols (UART, I2C, SPI)          │
└────────────┬────────────────────────────────────────────┘
             │
Hardware Abstraction Layer (HAL):
┌────────────▼────────────────────────────────────────────┐
│  ├── Camera Driver (HM0360/OV5647/IMX219)              │
│  ├── UART Driver                                        │
│  ├── Timer Driver                                       │
│  ├── PMU (Power Management Unit)                       │
│  └── Flash Driver                                       │
└────────────┬────────────────────────────────────────────┘
             │
Hardware:
┌────────────▼────────────────────────────────────────────┐
│  HIMAX WE2 (Cortex-M55)                                │
│  ├── CPU: Dual Cortex-M55 @ 400MHz                     │
│  ├── Memory: 2MB SRAM, 16MB Flash                      │
│  ├── NPU: Ethos-U55 (for ML acceleration)             │
│  └── Peripherals: Camera, UART, I2C, SPI, Timer       │
└─────────────────────────────────────────────────────────┘

External Communication:
┌─────────────────────────────────────────────────────────┐
│  WE2 ◄──UART──► STM32WL ◄──LoRaWAN──► Network Server   │
└─────────────────────────────────────────────────────────┘
```

### Memory Map

```
Flash Memory Layout (16MB):
┌────────────────────────────────────┐ 0x00000000
│  Bootloader (256KB)                │
├────────────────────────────────────┤ 0x00040000
│  Main Firmware (768KB)             │
│  - Application Code                │
│  - Libraries                       │
│  - RTOS                            │
├────────────────────────────────────┤ 0x00100000
│  Reserved (256KB)                  │
├────────────────────────────────────┤ 0x00140000
│  ML Models (2MB)                   │
│  - Model 1: 0x00200000            │
│  - Model 2: 0x00280000            │
│  - Model 3: 0x0032A000            │
├────────────────────────────────────┤ 0x00340000
│  Configuration/Calibration (64KB)  │
├────────────────────────────────────┤ 0x00350000
│  User Data/Logs (256KB)           │
├────────────────────────────────────┤ 0x00390000
│  Reserved for OTA (7MB)           │
└────────────────────────────────────┘ 0x01000000

SRAM Memory Layout (2MB):
┌────────────────────────────────────┐ 0x30000000
│  Bootloader RAM (16KB)             │
├────────────────────────────────────┤ 0x30004000
│  Application Stack (256KB)         │
│  - Main Stack                      │
│  - FreeRTOS Stacks                │
├────────────────────────────────────┤ 0x30044000
│  Application Heap (512KB)          │
│  - Dynamic Memory                  │
│  - Tensor Arena (100KB)           │
├────────────────────────────────────┤ 0x300C4000
│  Frame Buffers (1MB)               │
│  - Raw Image Buffer                │
│  - Processed Image                │
├────────────────────────────────────┤ 0x301C4000
│  Global Variables (32KB)           │
├────────────────────────────────────┤ 0x301CC000
│  Reserved/DMA (200KB)              │
└────────────────────────────────────┘ 0x30200000
```

---

## Development Environment Setup

### 1. Install ARM GNU Toolchain

```bash
# Download ARM GCC 13.2
cd ~
wget https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz

# Extract
tar -xvf arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz

# Add to PATH (add to ~/.bashrc for persistence)
export PATH="$HOME/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-eabi/bin/:$PATH"

# Verify installation
arm-none-eabi-gcc --version
# Should show: arm-none-eabi-gcc (Arm GNU Toolchain 13.2.rel1) 13.2.1
```

### 2. Install Build Tools

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y make git python3 python3-pip

# Install Python dependencies
pip3 install pyserial tqdm

# Install flash tools
cd camera-watcam
pip3 install -r xmodem/requirements.txt
```

### 3. Install Development Tools (Optional but Recommended)

```bash
# Code editor with C/C++ support
# VS Code with extensions:
# - C/C++ (Microsoft)
# - Cortex-Debug
# - ARM Assembly

# Serial monitor
sudo apt install -y minicom screen

# Debugger (for SWD debugging)
sudo apt install -y openocd gdb-multiarch

# Code formatting
sudo apt install -y clang-format
```

### 4. Clone and Setup Repository

```bash
# Clone repository
git clone <your-repo-url> camera-watcam
cd camera-watcam

# Verify structure
ls EPII_CM55M_APP_S/
# Should see: app, board, device, drivers, library, etc.
```

---

## Project Structure Deep Dive

### WE2 SDK Directory Structure

```
EPII_CM55M_APP_S/
│
├── app/                              # Application code
│   └── scenario_app/                # Scenario applications
│       ├── allon_sensor_tflm/       # Basic ML example
│       ├── tflm_fd_fm/              # Face detection example
│       └── water_meter_amr/         # ⭐ OUR AMR APPLICATION
│
├── board/                            # Board support packages
│   └── epii_evb/                    # EVB board definitions
│       ├── epii_evb.mk              # Board makefile
│       ├── epii_init.c              # Board initialization
│       └── pinmux_init.c            # Pin multiplexing
│
├── device/                           # Device drivers
│   ├── clib/                        # C library
│   └── hal/                         # Hardware abstraction layer
│
├── drivers/                          # Peripheral drivers
│   ├── inc/                         # Driver headers
│   ├── src/                         # Driver implementations
│   │   ├── hx_drv_uart.c           # UART driver
│   │   ├── hx_drv_timer.c          # Timer driver
│   │   ├── hx_drv_gpio.c           # GPIO driver
│   │   └── hx_drv_pmu.c            # Power management
│   └── mk_cfg/                      # Driver configurations
│
├── external/                         # External peripherals
│   └── cis/                         # Camera image sensors
│       ├── hm0360_revb/             # HM0360 camera
│       ├── hm11b1/                  # HM11B1 camera
│       ├── ov5647/                  # OV5647 (Pi Camera)
│       ├── imx219/                  # IMX219 (Pi Camera v2)
│       └── imx477/                  # IMX477 (Pi HQ Camera)
│
├── library/                          # Middleware libraries
│   ├── inference/                   # ML inference
│   │   └── tflmtag2209_u55tag2205/ # TFLite Micro + Ethos-U55
│   ├── sensordp/                    # Sensor datapath
│   ├── img_proc/                    # Image processing
│   ├── cmsis_nn/                    # CMSIS-NN (optimized ML)
│   ├── cmsis_dsp/                   # CMSIS-DSP (signal processing)
│   └── pwrmgmt/                     # Power management
│
├── os/                               # Operating system
│   └── rtos2_freertos/              # FreeRTOS 10.5.1
│       └── 10.5.1/
│           ├── Source/              # FreeRTOS source
│           ├── portable/            # Port for Cortex-M55
│           └── include/             # FreeRTOS headers
│
├── linker_script/                    # Linker scripts
│   ├── gcc/                         # GCC linker scripts
│   └── arm/                         # ARM linker scripts
│
├── middleware/                       # Additional middleware
│   └── fatfs/                       # FAT filesystem
│
├── obj_*/                            # Build output (generated)
│   └── gnu_epii_evb_WLCSP65/       # Build artifacts
│       ├── *.o                      # Object files
│       ├── *.elf                    # Executable
│       ├── *.map                    # Memory map
│       └── *.bin                    # Binary
│
└── makefile                          # ⭐ Top-level makefile
```

### Your Application Structure

```
EPII_CM55M_APP_S/app/scenario_app/water_meter_amr/
│
├── main.c                            # ⭐ Application entry point
├── water_meter_amr.c                # ⭐ Main application logic
├── water_meter_amr.h                # Public interface
├── common_config.h                  # ⭐ Configuration
│
├── recognition/                      # Digit recognition module
│   ├── digit_recognition.c
│   ├── digit_recognition.h
│   ├── preprocessing.c              # Image preprocessing
│   └── preprocessing.h
│
├── communication/                    # Communication module
│   ├── uart_comm.c                  # UART to STM32WL
│   ├── uart_comm.h
│   └── protocol.h                   # Protocol definitions
│
├── power/                            # Power management
│   ├── power_mgmt.c
│   └── power_mgmt.h
│
├── models/                           # ML models
│   └── water_meter_digit_model.h   # ⭐ Model data
│
├── cis_sensor/                      # Camera sensor config
│   └── cis_hm0360/
│       ├── cisdp_cfg.h             # Sensor configuration
│       └── cisdp_sensor.c          # Sensor driver
│
├── water_meter_amr.mk               # ⭐ Application makefile
└── drv_user_defined.mk             # ⭐ Driver configuration
```

---

## Creating Your First Application

### Step 1: Create Application Directory

```bash
cd EPII_CM55M_APP_S/app/scenario_app
mkdir -p my_first_app
cd my_first_app
```

### Step 2: Create main.c

**File**: `EPII_CM55M_APP_S/app/scenario_app/my_first_app/main.c`

```c
#include <stdio.h>
#include <string.h>
#include "WE2_device.h"
#include "hx_drv_timer.h"
#include "hx_drv_uart.h"
#include "hx_drv_gpio.h"

/**
 * @brief Main application entry point
 *
 * This is where your application starts execution.
 * Initialize hardware, setup peripherals, and run main loop.
 */
int main(void)
{
    /* Print startup message */
    printf("\n");
    printf("========================================\n");
    printf("  My First WE2 Application\n");
    printf("  Build: %s %s\n", __DATE__, __TIME__);
    printf("========================================\n");
    printf("\n");

    /* Initialize hardware */
    printf("Initializing hardware...\n");

    /* Initialize UART (already done by bootloader, but can reconfigure) */
    HX_DRV_UART_CONFIG uart_cfg;
    uart_cfg.uart_baudrate = UART_BR_921600;
    uart_cfg.uart_databits = UART_DATA_8BITS;
    uart_cfg.uart_stopbits = UART_STOPBIT_1;
    uart_cfg.uart_parity = UART_PARITY_NONE;
    uart_cfg.uart_flowctrl = UART_FLOWCTRL_NONE;

    if (hx_drv_uart_init(USE_DW_UART_0, &uart_cfg) != HX_DRV_LIB_PASS) {
        printf("ERROR: UART init failed\n");
        return -1;
    }

    printf("✓ UART initialized\n");

    /* Initialize GPIO for LED */
    hx_drv_gpio_init(GPIO_0);  // Assuming GPIO 0 is an LED
    hx_drv_gpio_set_output(GPIO_0, GPIO_OUT_LOW);
    printf("✓ GPIO initialized\n");

    /* Initialize timer */
    hx_drv_timer_init();
    printf("✓ Timer initialized\n");

    printf("\n");
    printf("System initialized successfully!\n");
    printf("Entering main loop...\n");
    printf("\n");

    /* Main loop */
    uint32_t loop_count = 0;
    uint32_t last_blink_time = 0;
    uint8_t led_state = 0;

    while (1) {
        uint32_t current_time = hx_drv_timer_get_ms();

        /* Blink LED every 1 second */
        if ((current_time - last_blink_time) >= 1000) {
            led_state = !led_state;
            hx_drv_gpio_set_output(GPIO_0, led_state ? GPIO_OUT_HIGH : GPIO_OUT_LOW);

            printf("[%lu] Loop iteration: %lu, LED: %s\n",
                   current_time,
                   loop_count,
                   led_state ? "ON" : "OFF");

            last_blink_time = current_time;
            loop_count++;
        }

        /* Small delay to prevent busy loop */
        hx_drv_timer_delay_ms(10);
    }

    return 0;
}
```

### Step 3: Create Application Makefile

**File**: `EPII_CM55M_APP_S/app/scenario_app/my_first_app/my_first_app.mk`

```makefile
##
## Application Makefile for My First App
##

# Define application name
APPL_DEFINES += -DMY_FIRST_APP

# Application source files
APPL_CSRC += \
    $(APP_PATH)/main.c

# Application C++ source files (if any)
APPL_CXXSRC +=

# Application include paths
APPL_INCLUDES += \
    -I$(APP_PATH)

# Select libraries to include
# LIB_SEL: Select middleware libraries
# Options: tflmtag2209_u55tag2205 (TFLite Micro)
#          cmsis_nn (CMSIS-NN for ML)
#          cmsis_dsp (CMSIS-DSP for signal processing)
#          sensordp (Sensor datapath)
#          img_proc (Image processing)

# For this simple app, we don't need ML libraries
# LIB_SEL += tflmtag2209_u55tag2205

# Camera sensor support (if needed)
# CIS_SUPPORT_INAPP = cis_hm0360
# CIS_SUPPORT_INAPP = cis_imx219
# CIS_SUPPORT_INAPP = cis_ov5647

# Memory configuration
# Use default linker script
# override LINKER_SCRIPT_FILE := $(SCENARIO_APP_ROOT)/$(APP_TYPE)/my_app.ld
```

### Step 4: Create Driver Configuration

**File**: `EPII_CM55M_APP_S/app/scenario_app/my_first_app/drv_user_defined.mk`

```makefile
##
## Driver Configuration
##

# Include default driver configuration
include $(DRIVERS)/mk_cfg/drv_user_defined.mk

# Enable specific drivers
# UART is enabled by default

# If you need additional peripherals, uncomment:
# APPL_DEFINES += -DUSE_I2C
# APPL_DEFINES += -DUSE_SPI
# APPL_DEFINES += -DUSE_GPIO
```

### Step 5: Update Top-Level Makefile

**File**: `EPII_CM55M_APP_S/makefile`

Edit the makefile to set your application:

```makefile
# Set APP_TYPE to your application name
APP_TYPE = my_first_app
```

### Step 6: Build the Application

```bash
cd EPII_CM55M_APP_S

# Clean previous build
make clean

# Build
make -j$(nproc)

# Output will be in:
# obj_epii_evb_icv30_bdv10/gnu_epii_evb_WLCSP65/EPII_CM55M_gnu_epii_evb_WLCSP65_s.elf
```

### Step 7: Generate Firmware Image

```bash
cd ../we2_image_gen_local

# Copy ELF file
cp ../EPII_CM55M_APP_S/obj_epii_evb_icv30_bdv10/gnu_epii_evb_WLCSP65/EPII_CM55M_gnu_epii_evb_WLCSP65_s.elf input_case1_secboot/

# Generate image
./we2_local_image_gen project_case1_blp_wlcsp.json

# Output: output_case1_sec_wlcsp/output.img
```

### Step 8: Flash to Device

```bash
# Using xmodem script
cd ..
python3 xmodem/xmodem_send.py \
    --port /dev/ttyACM0 \
    --baudrate 921600 \
    --protocol xmodem \
    --file we2_image_gen_local/output_case1_sec_wlcsp/output.img

# Press RESET button on device
```

### Step 9: Monitor Output

```bash
# Using minicom
minicom -D /dev/ttyACM0 -b 921600

# Or using screen
screen /dev/ttyACM0 921600

# You should see:
# ========================================
#   My First WE2 Application
#   Build: Nov 22 2024 12:00:00
# ========================================
#
# Initializing hardware...
# ✓ UART initialized
# ✓ GPIO initialized
# ✓ Timer initialized
#
# System initialized successfully!
# Entering main loop...
#
# [1000] Loop iteration: 0, LED: ON
# [2000] Loop iteration: 1, LED: OFF
# ...
```

---

## Camera Integration

### Camera Architecture

```
Camera Hardware:
┌─────────────┐
│  Camera     │
│  Sensor     │──I2C──► Configuration
│  (HM0360)   │
│             │──MIPI──► Image Data
└─────────────┘

WE2 Camera Subsystem:
┌────────────────────────────────────┐
│  Sensor Datapath (SENSORDP)        │
│  ├── Image Capture                 │
│  ├── Format Conversion             │
│  ├── Resize/Crop                   │
│  └── Frame Buffer Management       │
└──────────┬─────────────────────────┘
           │
           ▼
    ┌──────────────┐
    │ Frame Buffer │
    │  (Raw Image) │
    └──────┬───────┘
           │
           ▼
    ┌──────────────┐
    │ Preprocessing│
    │ (Your Code)  │
    └──────┬───────┘
           │
           ▼
    ┌──────────────┐
    │ ML Inference │
    └──────────────┘
```

### Camera Integration Example

**File**: `camera_app/camera_capture.c`

```c
#include <stdio.h>
#include <string.h>
#include "WE2_device.h"
#include "hx_drv_timer.h"
#include "sensordp_sensor_api.h"
#include "cisdp_sensor.h"

/* Camera configuration */
#define CAMERA_WIDTH    640
#define CAMERA_HEIGHT   480
#define CAMERA_FORMAT   SENSORDP_STREAM_RAW8  // 8-bit grayscale

/* Frame buffer */
static uint8_t frame_buffer[CAMERA_WIDTH * CAMERA_HEIGHT];

/**
 * @brief Initialize camera sensor
 */
int camera_init(void)
{
    SENSORDP_STATUS_E ret;

    printf("Initializing camera sensor...\n");

    /* Initialize sensor datapath */
    ret = sensordp_init();
    if (ret != SENSORDP_STATUS_OK) {
        printf("ERROR: sensordp_init failed: %d\n", ret);
        return -1;
    }

    /* Configure sensor */
    HX_CIS_SensorSetting_t sensor_cfg;
    sensor_cfg.sensor_width = CAMERA_WIDTH;
    sensor_cfg.sensor_height = CAMERA_HEIGHT;
    sensor_cfg.sensor_framerate = 15;  // 15 FPS
    sensor_cfg.sensor_type = HX_CIS_SENSOR_TYPE_HM0360;

    ret = sensordp_set_sensorctrl_cfg(&sensor_cfg,
                                       SENSORDP_STREAM_RAW8,
                                       SENSORDP_CFG_TYPE_SENSOR_STRM);
    if (ret != SENSORDP_STATUS_OK) {
        printf("ERROR: sensordp_set_sensorctrl_cfg failed: %d\n", ret);
        return -1;
    }

    /* Start sensor streaming */
    ret = sensordp_start_capture();
    if (ret != SENSORDP_STATUS_OK) {
        printf("ERROR: sensordp_start_capture failed: %d\n", ret);
        return -1;
    }

    printf("✓ Camera initialized successfully\n");
    return 0;
}

/**
 * @brief Capture a frame from camera
 *
 * @param buffer Output buffer for image data
 * @param width  Image width
 * @param height Image height
 * @return 0 on success, negative on error
 */
int camera_capture_frame(uint8_t *buffer, uint32_t width, uint32_t height)
{
    SENSORDP_STATUS_E ret;
    sensordp_frame_t frame_data;

    /* Wait for frame ready */
    ret = sensordp_get_frame(&frame_data, 1000);  // 1 second timeout
    if (ret != SENSORDP_STATUS_OK) {
        printf("ERROR: sensordp_get_frame failed: %d\n", ret);
        return -1;
    }

    /* Copy frame data to output buffer */
    if (frame_data.width != width || frame_data.height != height) {
        printf("ERROR: Frame size mismatch (expected %dx%d, got %dx%d)\n",
               width, height, frame_data.width, frame_data.height);
        return -1;
    }

    memcpy(buffer, frame_data.raw_data, width * height);

    /* Release frame */
    sensordp_release_frame(&frame_data);

    return 0;
}

/**
 * @brief Main function demonstrating camera usage
 */
int main(void)
{
    printf("\n");
    printf("========================================\n");
    printf("  Camera Capture Example\n");
    printf("========================================\n");
    printf("\n");

    /* Initialize camera */
    if (camera_init() != 0) {
        printf("Camera initialization failed!\n");
        return -1;
    }

    /* Capture loop */
    uint32_t frame_count = 0;

    while (1) {
        printf("Capturing frame %lu...\n", frame_count);

        /* Capture frame */
        if (camera_capture_frame(frame_buffer, CAMERA_WIDTH, CAMERA_HEIGHT) == 0) {
            printf("✓ Frame %lu captured successfully\n", frame_count);

            /* Process frame here */
            // Example: Calculate average brightness
            uint32_t sum = 0;
            for (int i = 0; i < CAMERA_WIDTH * CAMERA_HEIGHT; i++) {
                sum += frame_buffer[i];
            }
            uint8_t avg_brightness = sum / (CAMERA_WIDTH * CAMERA_HEIGHT);
            printf("  Average brightness: %u\n", avg_brightness);

            frame_count++;
        } else {
            printf("✗ Frame capture failed\n");
        }

        /* Wait before next capture */
        hx_drv_timer_delay_ms(1000);  // 1 second delay
    }

    return 0;
}
```

### Camera Configuration for Different Sensors

**For HM0360 (default)**:
```makefile
# In your .mk file
CIS_SUPPORT_INAPP = cis_hm0360
```

**For Raspberry Pi Camera v2 (IMX219)**:
```makefile
CIS_SUPPORT_INAPP = cis_imx219
```

**For Raspberry Pi HQ Camera (IMX477)**:
```makefile
CIS_SUPPORT_INAPP = cis_imx477
```

---

## ML Model Integration

### ML Integration Architecture

```
Training (PC/Cloud):
┌─────────────────┐
│  TensorFlow/    │
│  Keras Model    │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  TFLite         │
│  Converter      │
│  (INT8 Quant)   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  .tflite file   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  C Header Gen   │
│  (xxd/script)   │
└────────┬────────┘
         │
         ▼

Deployment (WE2):
┌─────────────────┐
│  Model Data     │
│  (C array)      │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  TFLite Micro   │
│  Interpreter    │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Inference      │
│  Result         │
└─────────────────┘
```

### Step-by-Step ML Integration

#### Step 1: Train and Convert Model

See `tools/training/` for complete training pipeline.

#### Step 2: Generate C Header from TFLite

```bash
cd tools/training
python3 generate_model_header.py \
    --input ../../models/water_meter_digit_model.tflite \
    --output ../../models/water_meter_digit_model.h \
    --array-name digit_recognition_model
```

Output `water_meter_digit_model.h`:
```c
const unsigned int digit_recognition_model_len = 28456;
const unsigned char digit_recognition_model[] = {
  0x1c, 0x00, 0x00, 0x00, 0x54, 0x46, 0x4c, 0x33,
  // ... model data ...
};
```

#### Step 3: Copy Model to Application

```bash
cp models/water_meter_digit_model.h \
   EPII_CM55M_APP_S/app/scenario_app/water_meter_amr/models/
```

#### Step 4: Create Inference Code

**File**: `ml_app/inference.cpp`

```cpp
#include <stdio.h>
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Include model
#include "models/water_meter_digit_model.h"

// Tensor arena size (adjust based on your model)
constexpr int kTensorArenaSize = 100 * 1024;  // 100KB
static uint8_t tensor_arena[kTensorArenaSize];

// Global pointers
static const tflite::Model* model = nullptr;
static tflite::MicroInterpreter* interpreter = nullptr;
static TfLiteTensor* input_tensor = nullptr;
static TfLiteTensor* output_tensor = nullptr;

/**
 * @brief Initialize TensorFlow Lite Micro
 *
 * @return 0 on success, -1 on failure
 */
int ml_init(void)
{
    printf("Initializing TensorFlow Lite Micro...\n");

    // Load model
    model = tflite::GetModel(digit_recognition_model);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        printf("ERROR: Model schema version %lu != TFLite schema %d\n",
               model->version(), TFLITE_SCHEMA_VERSION);
        return -1;
    }

    // Setup operation resolver
    // Add only the operations your model uses
    static tflite::MicroMutableOpResolver<6> micro_op_resolver;
    micro_op_resolver.AddConv2D();
    micro_op_resolver.AddMaxPool2D();
    micro_op_resolver.AddReshape();
    micro_op_resolver.AddFullyConnected();
    micro_op_resolver.AddSoftmax();
    micro_op_resolver.AddQuantize();

    // Build interpreter
    static tflite::MicroInterpreter static_interpreter(
        model, micro_op_resolver, tensor_arena, kTensorArenaSize);
    interpreter = &static_interpreter;

    // Allocate tensors
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        printf("ERROR: AllocateTensors() failed\n");
        return -1;
    }

    // Get input and output tensors
    input_tensor = interpreter->input(0);
    output_tensor = interpreter->output(0);

    printf("✓ TFLite initialized successfully\n");
    printf("  Model size: %u bytes\n", digit_recognition_model_len);
    printf("  Input:  %d x %d x %d (%s)\n",
           input_tensor->dims->data[1],
           input_tensor->dims->data[2],
           input_tensor->dims->data[3],
           input_tensor->type == kTfLiteInt8 ? "INT8" : "FLOAT32");
    printf("  Output: %d classes (%s)\n",
           output_tensor->dims->data[1],
           output_tensor->type == kTfLiteInt8 ? "INT8" : "FLOAT32");
    printf("  Arena used: %zu / %d bytes\n",
           interpreter->arena_used_bytes(), kTensorArenaSize);

    return 0;
}

/**
 * @brief Run inference on input image
 *
 * @param image Input image data (28x28 grayscale)
 * @param result Output class (0-9 for digits)
 * @param confidence Output confidence (0.0-1.0)
 * @return 0 on success
 */
int ml_inference(const uint8_t* image, int* result, float* confidence)
{
    // Copy image to input tensor (convert to INT8 if needed)
    int8_t* input_data = input_tensor->data.int8;
    for (int i = 0; i < 28 * 28; i++) {
        // Convert uint8 [0, 255] to int8 [-128, 127]
        input_data[i] = (int8_t)(image[i] - 128);
    }

    // Run inference
    uint32_t start_time = hx_drv_timer_get_ms();

    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        printf("ERROR: Invoke() failed\n");
        return -1;
    }

    uint32_t end_time = hx_drv_timer_get_ms();
    uint32_t inference_time = end_time - start_time;

    // Get output
    int8_t* output_data = output_tensor->data.int8;

    // Find class with highest score
    int max_idx = 0;
    int8_t max_val = output_data[0];
    for (int i = 1; i < 10; i++) {
        if (output_data[i] > max_val) {
            max_val = output_data[i];
            max_idx = i;
        }
    }

    *result = max_idx;
    // Convert INT8 score to confidence [0.0, 1.0]
    *confidence = (max_val + 128) / 255.0f;

    printf("  Inference time: %lu ms\n", inference_time);
    printf("  Predicted class: %d (confidence: %.2f%%)\n",
           *result, *confidence * 100.0f);

    return 0;
}

/**
 * @brief Main function demonstrating ML inference
 */
int main(void)
{
    printf("\n");
    printf("========================================\n");
    printf("  ML Inference Example\n");
    printf("========================================\n");
    printf("\n");

    // Initialize ML
    if (ml_init() != 0) {
        printf("ML initialization failed!\n");
        return -1;
    }

    // Example: Create a test image (all zeros = digit '0' hopefully)
    uint8_t test_image[28 * 28];
    memset(test_image, 0, sizeof(test_image));

    // Run inference
    printf("\nRunning inference on test image...\n");
    int predicted_class;
    float confidence;

    if (ml_inference(test_image, &predicted_class, &confidence) == 0) {
        printf("\n✓ Inference successful\n");
        printf("  Predicted digit: %d\n", predicted_class);
        printf("  Confidence: %.2f%%\n", confidence * 100.0f);
    } else {
        printf("\n✗ Inference failed\n");
    }

    return 0;
}
```

#### Step 5: Update Makefile to Include TFLite

**File**: `ml_app/ml_app.mk`

```makefile
# Enable TensorFlow Lite Micro library
LIB_SEL += tflmtag2209_u55tag2205

# Add C++ source files
APPL_CXXSRC += \
    $(APP_PATH)/inference.cpp

# Add model include path
APPL_INCLUDES += \
    -I$(APP_PATH)/models
```

---

## LoRaWAN Communication

### Architecture Overview

The WE2 device communicates with the STM32WL LoRaWAN modem via UART using AT commands.

```
┌─────────────────┐              ┌─────────────────┐
│   HIMAX WE2     │   UART       │    STM32WL      │
│                 │ ◄──────────► │  LoRaWAN Modem  │
│  Application    │  115200 baud │                 │
└─────────────────┘              └─────────────────┘
        │                                │
        │ Camera + ML                    │ AT Commands
        │ Processing                     │ LoRaWAN Protocol
        │                                │
        ▼                                ▼
  Digit Recognition              Network Server
                                (TTN/ChirpStack/AWS)
```

### UART Communication Setup

#### Hardware Connections

| WE2 Pin | STM32WL Pin | Signal |
|---------|-------------|--------|
| GPIO 0  | PA2 (TX)    | WE2 RX |
| GPIO 1  | PA3 (RX)    | WE2 TX |
| GND     | GND         | Ground |

#### UART Initialization Code

**File**: `lorawan_comm.c`

```c
#include "hx_drv_uart.h"
#include <stdio.h>
#include <string.h>

#define LORAWAN_UART        USE_DW_UART_0
#define LORAWAN_BAUDRATE    UART_BR_115200
#define AT_TIMEOUT_MS       5000
#define RX_BUFFER_SIZE      512

static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint32_t rx_index = 0;

/**
 * @brief Initialize UART for LoRaWAN modem communication
 *
 * @return 0 on success, -1 on failure
 */
int lorawan_uart_init(void)
{
    HX_DRV_UART_CONFIG uart_cfg;

    // Configure UART
    uart_cfg.uart_baudrate = LORAWAN_BAUDRATE;
    uart_cfg.uart_protocol = UART_PROTOCOL_8N1;
    uart_cfg.uart_fifo = UART_FIFO_ENABLE;

    // Initialize UART
    if (hx_drv_uart_init(LORAWAN_UART, &uart_cfg) != HX_DRV_LIB_PASS) {
        printf("ERROR: UART init failed\n");
        return -1;
    }

    printf("✓ LoRaWAN UART initialized (115200 baud)\n");
    return 0;
}

/**
 * @brief Send AT command to modem
 *
 * @param cmd AT command string (without \r\n)
 * @return 0 on success
 */
int lorawan_send_at_command(const char* cmd)
{
    char at_cmd[256];
    snprintf(at_cmd, sizeof(at_cmd), "%s\r\n", cmd);

    uint32_t len = strlen(at_cmd);

    printf("TX: %s", at_cmd);

    // Send command
    for (uint32_t i = 0; i < len; i++) {
        hx_drv_uart_write(LORAWAN_UART, at_cmd[i]);
    }

    return 0;
}

/**
 * @brief Read response from modem
 *
 * @param response Buffer to store response
 * @param max_len Maximum buffer size
 * @param timeout_ms Timeout in milliseconds
 * @return Number of bytes read, or -1 on timeout
 */
int lorawan_read_response(char* response, uint32_t max_len, uint32_t timeout_ms)
{
    uint32_t start_time = hx_drv_timer_get_ms();
    uint32_t index = 0;

    memset(response, 0, max_len);

    while ((hx_drv_timer_get_ms() - start_time) < timeout_ms) {
        uint8_t byte;

        if (hx_drv_uart_read(LORAWAN_UART, &byte) == HX_DRV_LIB_PASS) {
            if (index < max_len - 1) {
                response[index++] = byte;

                // Check for end of response (OK or ERROR)
                if (strstr(response, "OK\r\n") || strstr(response, "ERROR\r\n")) {
                    response[index] = '\0';
                    printf("RX: %s", response);
                    return index;
                }
            }
        }

        hx_drv_timer_delay_ms(10);
    }

    printf("ERROR: Response timeout\n");
    return -1;
}

/**
 * @brief Wait for specific response pattern
 *
 * @param expected Expected string in response
 * @param timeout_ms Timeout in milliseconds
 * @return 0 if found, -1 on timeout or error
 */
int lorawan_wait_for_response(const char* expected, uint32_t timeout_ms)
{
    char response[512];

    int len = lorawan_read_response(response, sizeof(response), timeout_ms);
    if (len < 0) {
        return -1;
    }

    if (strstr(response, expected) != NULL) {
        return 0;
    }

    printf("ERROR: Expected '%s' not found in response\n", expected);
    return -1;
}
```

### LoRaWAN Initialization

```c
/**
 * @brief Initialize LoRaWAN modem
 *
 * @return 0 on success, -1 on failure
 */
int lorawan_init(void)
{
    printf("\n");
    printf("========================================\n");
    printf("  Initializing LoRaWAN Modem\n");
    printf("========================================\n");
    printf("\n");

    // Initialize UART
    if (lorawan_uart_init() != 0) {
        return -1;
    }

    // Wait for modem to boot
    hx_drv_timer_delay_ms(2000);

    // Test communication
    printf("Testing AT communication...\n");
    lorawan_send_at_command("AT");
    if (lorawan_wait_for_response("OK", AT_TIMEOUT_MS) != 0) {
        printf("ERROR: Modem not responding\n");
        return -1;
    }
    printf("✓ Modem responding\n");

    // Get firmware version
    printf("Getting modem firmware version...\n");
    lorawan_send_at_command("AT+VER?");
    lorawan_wait_for_response("OK", AT_TIMEOUT_MS);

    // Reset modem
    printf("Resetting modem...\n");
    lorawan_send_at_command("AT+RESET");
    hx_drv_timer_delay_ms(3000);

    printf("✓ LoRaWAN modem initialized\n");
    return 0;
}
```

### OTAA Configuration and Join

```c
/**
 * @brief Configure LoRaWAN credentials (OTAA)
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
    lorawan_send_at_command(cmd);
    if (lorawan_wait_for_response("OK", AT_TIMEOUT_MS) != 0) {
        return -1;
    }

    // Set AppEUI
    snprintf(cmd, sizeof(cmd), "AT+APPEUI=%s", app_eui);
    lorawan_send_at_command(cmd);
    if (lorawan_wait_for_response("OK", AT_TIMEOUT_MS) != 0) {
        return -1;
    }

    // Set AppKey
    snprintf(cmd, sizeof(cmd), "AT+APPKEY=%s", app_key);
    lorawan_send_at_command(cmd);
    if (lorawan_wait_for_response("OK", AT_TIMEOUT_MS) != 0) {
        return -1;
    }

    // Set activation mode to OTAA
    lorawan_send_at_command("AT+MODE=0");  // 0 = OTAA
    if (lorawan_wait_for_response("OK", AT_TIMEOUT_MS) != 0) {
        return -1;
    }

    printf("✓ OTAA credentials configured\n");
    return 0;
}

/**
 * @brief Join LoRaWAN network using OTAA
 *
 * @param max_retries Maximum join attempts
 * @return 0 on success, -1 on failure
 */
int lorawan_join(uint8_t max_retries)
{
    printf("Joining LoRaWAN network...\n");

    for (uint8_t retry = 0; retry < max_retries; retry++) {
        printf("Join attempt %d/%d...\n", retry + 1, max_retries);

        // Send join command
        lorawan_send_at_command("AT+JOIN");

        // Wait for join response (can take up to 30 seconds)
        if (lorawan_wait_for_response("JOINED", 30000) == 0) {
            printf("✓ Successfully joined LoRaWAN network\n");
            return 0;
        }

        printf("Join failed, retrying in 5 seconds...\n");
        hx_drv_timer_delay_ms(5000);
    }

    printf("ERROR: Failed to join network after %d attempts\n", max_retries);
    return -1;
}
```

### Sending Data

```c
/**
 * @brief Send data payload via LoRaWAN
 *
 * @param port LoRaWAN port (1-223)
 * @param data Data buffer
 * @param len Data length (max 51 bytes for DR0, up to 242 for DR5)
 * @param confirmed Confirmed (1) or unconfirmed (0) message
 * @return 0 on success, -1 on failure
 */
int lorawan_send_data(uint8_t port, const uint8_t* data, uint8_t len, uint8_t confirmed)
{
    char cmd[512];
    char hex_data[512];

    // Convert data to hex string
    for (uint8_t i = 0; i < len; i++) {
        snprintf(&hex_data[i * 2], 3, "%02X", data[i]);
    }
    hex_data[len * 2] = '\0';

    // Build AT command
    // Format: AT+SEND=<port>:<confirmed>:<hex_data>
    snprintf(cmd, sizeof(cmd), "AT+SEND=%d:%d:%s", port, confirmed, hex_data);

    printf("Sending %d bytes on port %d (confirmed=%d)...\n", len, port, confirmed);
    printf("Data: %s\n", hex_data);

    lorawan_send_at_command(cmd);

    if (lorawan_wait_for_response("OK", AT_TIMEOUT_MS) != 0) {
        return -1;
    }

    // Wait for TX confirmation
    if (lorawan_wait_for_response("TX_DONE", 10000) != 0) {
        printf("ERROR: TX timeout\n");
        return -1;
    }

    printf("✓ Data sent successfully\n");
    return 0;
}

/**
 * @brief Send digit recognition result
 *
 * @param digits Array of recognized digits
 * @param count Number of digits
 * @param confidence Average confidence
 * @return 0 on success
 */
int lorawan_send_meter_reading(const uint8_t* digits, uint8_t count, float confidence)
{
    // Payload format:
    // Byte 0: Number of digits
    // Byte 1: Confidence (0-100)
    // Byte 2-N: Digit values (0-9)

    uint8_t payload[32];
    payload[0] = count;
    payload[1] = (uint8_t)(confidence * 100.0f);

    for (uint8_t i = 0; i < count; i++) {
        payload[2 + i] = digits[i];
    }

    uint8_t payload_len = 2 + count;

    // Send on port 10, confirmed message
    return lorawan_send_data(10, payload, payload_len, 1);
}
```

### Complete AMR Application with LoRaWAN

```c
/**
 * @brief Main AMR application with LoRaWAN
 */
int main(void)
{
    printf("\n");
    printf("========================================\n");
    printf("  Water Meter AMR Application\n");
    printf("========================================\n");
    printf("\n");

    // Initialize LoRaWAN
    if (lorawan_init() != 0) {
        printf("LoRaWAN init failed!\n");
        return -1;
    }

    // Configure credentials (from provisioning)
    if (lorawan_configure_otaa(
        "70B3D57ED005XXXX",  // DevEUI
        "0000000000000000",  // AppEUI
        "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"  // AppKey
    ) != 0) {
        printf("Configuration failed!\n");
        return -1;
    }

    // Join network
    if (lorawan_join(3) != 0) {
        printf("Join failed!\n");
        return -1;
    }

    // Initialize camera
    if (camera_init() != 0) {
        printf("Camera init failed!\n");
        return -1;
    }

    // Initialize ML
    if (ml_init() != 0) {
        printf("ML init failed!\n");
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

            // Process digits (simplified - would need digit segmentation)
            uint8_t digits[8];
            uint8_t digit_count = 0;
            float total_confidence = 0.0f;

            // For each digit position in meter
            for (int i = 0; i < 8; i++) {
                // Extract digit ROI and resize to 28x28
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

            // Send reading via LoRaWAN
            if (lorawan_send_meter_reading(digits, digit_count, avg_confidence) == 0) {
                printf("✓ Reading sent to cloud\n");
            } else {
                printf("✗ Failed to send reading\n");
            }
        }

        // Sleep for 1 hour (configurable)
        printf("\nSleeping for 1 hour...\n\n");
        hx_drv_timer_delay_ms(3600000);
    }

    return 0;
}
```

### Power-Optimized LoRaWAN Usage

```c
/**
 * @brief Configure LoRaWAN for low power operation
 */
int lorawan_configure_low_power(void)
{
    // Set adaptive data rate
    lorawan_send_at_command("AT+ADR=1");
    lorawan_wait_for_response("OK", AT_TIMEOUT_MS);

    // Set duty cycle limits
    lorawan_send_at_command("AT+DC=1");
    lorawan_wait_for_response("OK", AT_TIMEOUT_MS);

    // Set transmit power to lowest necessary
    lorawan_send_at_command("AT+TXP=0");  // 0 = maximum power reduction
    lorawan_wait_for_response("OK", AT_TIMEOUT_MS);

    // Enable auto-sleep mode
    lorawan_send_at_command("AT+SLEEP=1");
    lorawan_wait_for_response("OK", AT_TIMEOUT_MS);

    printf("✓ Low power mode configured\n");
    return 0;
}
```

### AT Command Reference

Common AT commands for STM32WL LoRaWAN modem:

| Command | Description | Example |
|---------|-------------|---------|
| `AT` | Test communication | `AT` → `OK` |
| `AT+RESET` | Reset modem | `AT+RESET` |
| `AT+VER?` | Get firmware version | `AT+VER?` → `1.2.0` |
| `AT+DEVEUI=<eui>` | Set Device EUI | `AT+DEVEUI=70B3D57ED005XXXX` |
| `AT+APPEUI=<eui>` | Set Application EUI | `AT+APPEUI=0000000000000000` |
| `AT+APPKEY=<key>` | Set Application Key | `AT+APPKEY=XXXX...` |
| `AT+MODE=<mode>` | Set activation (0=OTAA, 1=ABP) | `AT+MODE=0` |
| `AT+JOIN` | Join network | `AT+JOIN` → `JOINED` |
| `AT+SEND=<port>:<conf>:<data>` | Send data | `AT+SEND=10:1:48656C6C6F` |
| `AT+ADR=<0/1>` | Adaptive Data Rate | `AT+ADR=1` |
| `AT+DR=<rate>` | Set data rate (0-5) | `AT+DR=0` |
| `AT+TXP=<power>` | Set TX power | `AT+TXP=0` |
| `AT+CLASS=<A/B/C>` | Set device class | `AT+CLASS=A` |

---

## Build System

### Build System Overview

The HIMAX WE2 SDK uses a GNU Make-based build system with a hierarchical structure:

```
Build System Hierarchy
┌─────────────────────────────────────────┐
│  Top-Level Makefile                     │
│  (EPII_CM55M_APP_S/makefile)            │
└───────────┬─────────────────────────────┘
            │
            ├─► options.mk (build options)
            ├─► rules.mk (build rules)
            │
            ▼
┌─────────────────────────────────────────┐
│  Scenario Application Makefile          │
│  (scenario_app/<app>/<app>.mk)          │
└───────────┬─────────────────────────────┘
            │
            ├─► APPL_CSRC (C sources)
            ├─► APPL_CXXSRC (C++ sources)
            ├─► APPL_ASMSRC (Assembly sources)
            ├─► APPL_INCLUDES (Include paths)
            ├─► LIB_SEL (Library selection)
            │
            ▼
┌─────────────────────────────────────────┐
│  Library Makefiles                      │
│  (library/<lib>/<lib>.mk)               │
└───────────┬─────────────────────────────┘
            │
            ▼
┌─────────────────────────────────────────┐
│  Compiled Objects & Linking             │
│  → output.elf                           │
└─────────────────────────────────────────┘
```

### Key Makefile Variables

#### Application Configuration

```makefile
# Application name and path
APPL_NAME = water_meter_amr
APP_PATH = $(EPII_ROOT)/app/scenario_app/$(APPL_NAME)

# Toolchain selection
TOOLCHAIN = arm
TOOLCHAIN_VER = 13.2

# Target configuration
CORTEX_M = 55
EPII_CORE = EPII_CM55M_S
```

#### Source Files

```makefile
# C source files
APPL_CSRC = \
    $(APP_PATH)/main.c \
    $(APP_PATH)/camera.c \
    $(APP_PATH)/lorawan_comm.c \
    $(APP_PATH)/power_mgmt.c

# C++ source files (for ML)
APPL_CXXSRC = \
    $(APP_PATH)/inference.cpp

# Assembly source files (if needed)
APPL_ASMSRC =
```

#### Include Paths

```makefile
# Include directories
APPL_INCLUDES = \
    -I$(APP_PATH) \
    -I$(APP_PATH)/include \
    -I$(APP_PATH)/models \
    -I$(EPII_ROOT)/library/camera \
    -I$(EPII_ROOT)/library/drivers
```

#### Library Selection

```makefile
# Core libraries (always needed)
LIB_SEL = \
    sensordp \
    hxevent \
    httpd \
    mqtt \
    mbedtls \
    epii_CM55M_s_library \
    infra

# Camera sensor library
LIB_SEL += hm0360

# TensorFlow Lite Micro library
LIB_SEL += tflmtag2209_u55tag2205
```

#### Compiler Flags

```makefile
# C compiler flags
APPL_DEFINES = \
    -DCFG_CAMERA_SENSOR=HM0360 \
    -DCFG_ENABLE_ML=1 \
    -DCFG_ENABLE_LORAWAN=1 \
    -DDEBUG_LEVEL=2

# Optimization level
APPL_OPT = -O2

# Additional C flags
APPL_CFLAGS = \
    -Wall \
    -Wno-unused-function \
    -ffunction-sections \
    -fdata-sections

# C++ specific flags
APPL_CXXFLAGS = \
    -std=c++11 \
    -fno-rtti \
    -fno-exceptions \
    -fno-threadsafe-statics
```

#### Linker Configuration

```makefile
# Linker script
LINKER_SCRIPT = $(EPII_ROOT)/linker/epii_cm55m_s.ld

# Linker flags
APPL_LDFLAGS = \
    -Wl,--gc-sections \
    -Wl,--print-memory-usage \
    -Wl,-Map=$(OUT_DIR)/output.map

# Libraries to link
APPL_LIBS = \
    -lm \
    -lc \
    -lgcc
```

### Complete Application Makefile Example

**File**: `EPII_CM55M_APP_S/app/scenario_app/water_meter_amr/water_meter_amr.mk`

```makefile
##
# Water Meter AMR Application Makefile
##

# Application name
override APPL_NAME = water_meter_amr

# Application path
APP_PATH = $(EPII_ROOT)/app/scenario_app/$(APPL_NAME)

##
# Source Files
##

# C source files
APPL_CSRC = \
    $(APP_PATH)/main.c \
    $(APP_PATH)/camera.c \
    $(APP_PATH)/lorawan_comm.c \
    $(APP_PATH)/power_mgmt.c \
    $(APP_PATH)/digit_detection.c

# C++ source files (for TensorFlow Lite)
APPL_CXXSRC = \
    $(APP_PATH)/inference.cpp

# Assembly source files
APPL_ASMSRC =

##
# Include Paths
##

APPL_INCLUDES = \
    -I$(APP_PATH) \
    -I$(APP_PATH)/include \
    -I$(APP_PATH)/models \
    -I$(EPII_ROOT)/library/camera \
    -I$(EPII_ROOT)/library/drivers/inc \
    -I$(EPII_ROOT)/library/common/inc

##
# Library Selection
##

# Core libraries
LIB_SEL = \
    sensordp \
    hxevent \
    httpd \
    mqtt \
    mbedtls \
    epii_CM55M_s_library \
    infra

# Camera sensor
LIB_SEL += hm0360

# TensorFlow Lite Micro
LIB_SEL += tflmtag2209_u55tag2205

##
# Compiler Defines
##

APPL_DEFINES = \
    -DCFG_CAMERA_SENSOR=HM0360 \
    -DCFG_ENABLE_ML=1 \
    -DCFG_ENABLE_LORAWAN=1 \
    -DDEBUG_LEVEL=2 \
    -DFREERTOS_ENABLE=1

##
# Compiler Flags
##

# Optimization
APPL_OPT = -O2

# C flags
APPL_CFLAGS = \
    -Wall \
    -Wextra \
    -Wno-unused-parameter \
    -Wno-unused-function \
    -ffunction-sections \
    -fdata-sections \
    -fstack-usage

# C++ flags
APPL_CXXFLAGS = \
    -std=c++11 \
    -fno-rtti \
    -fno-exceptions \
    -fno-threadsafe-statics \
    -fno-use-cxa-atexit

##
# Linker Flags
##

APPL_LDFLAGS = \
    -Wl,--gc-sections \
    -Wl,--print-memory-usage \
    -Wl,-Map=$(OUT_DIR)/output.map \
    -Wl,--cref

APPL_LIBS = \
    -lm \
    -lc \
    -lgcc \
    -lstdc++

##
# Build Targets (optional custom targets)
##

.PHONY: flash-app
flash-app:
	@echo "Flashing application..."
	@openocd -f interface/cmsis-dap.cfg \
	         -f target/cortex_m.cfg \
	         -c "program $(OUT_DIR)/output.elf verify reset exit"

.PHONY: debug-app
debug-app:
	@echo "Starting debug session..."
	@arm-none-eabi-gdb $(OUT_DIR)/output.elf \
	    -ex "target remote localhost:3333"
```

### Build Process Step-by-Step

#### 1. **Preprocessing**

```bash
# Expand macros and includes
arm-none-eabi-gcc -E main.c \
    -I./include \
    -DCFG_CAMERA_SENSOR=HM0360 \
    -o main.i
```

#### 2. **Compilation**

```bash
# Compile C to object files
arm-none-eabi-gcc -c main.c \
    -mcpu=cortex-m55 \
    -mthumb \
    -O2 \
    -Wall \
    -I./include \
    -o main.o

# Compile C++ to object files
arm-none-eabi-g++ -c inference.cpp \
    -mcpu=cortex-m55 \
    -mthumb \
    -O2 \
    -std=c++11 \
    -fno-rtti \
    -fno-exceptions \
    -o inference.o
```

#### 3. **Assembly**

```bash
# Assemble .s files
arm-none-eabi-as startup.s \
    -mcpu=cortex-m55 \
    -o startup.o
```

#### 4. **Linking**

```bash
# Link all objects into ELF
arm-none-eabi-g++ \
    main.o \
    camera.o \
    inference.o \
    lorawan_comm.o \
    -T linker_script.ld \
    -Wl,--gc-sections \
    -Wl,-Map=output.map \
    -lm -lc -lgcc -lstdc++ \
    -o output.elf
```

#### 5. **Post-Processing**

```bash
# Generate binary
arm-none-eabi-objcopy -O binary output.elf output.bin

# Generate hex
arm-none-eabi-objcopy -O ihex output.elf output.hex

# Generate disassembly
arm-none-eabi-objdump -d output.elf > output.asm

# Show size
arm-none-eabi-size output.elf
```

### Build Commands

#### Basic Build

```bash
# Clean build
cd EPII_CM55M_APP_S
make clean APP=water_meter_amr

# Build application
make APP=water_meter_amr

# Build with verbose output
make APP=water_meter_amr V=1

# Build with specific toolchain
make APP=water_meter_amr TOOLCHAIN=arm TOOLCHAIN_VER=13.2
```

#### Advanced Build Options

```bash
# Debug build (no optimization)
make APP=water_meter_amr DEBUG=1

# Release build (with optimization)
make APP=water_meter_amr RELEASE=1

# Build with specific sensor
make APP=water_meter_amr CAMERA_SENSOR=HM0360

# Build with TFLite disabled (for testing)
make APP=water_meter_amr ENABLE_ML=0

# Parallel build (faster)
make APP=water_meter_amr -j8
```

### Understanding Build Output

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

**Interpretation:**
- `FLASH: 245760 B (240 KB)` - Program code + const data
- `SRAM: 89472 B (87 KB)` - RAM usage (data + bss)
- `text: 245760` - Code section size
- `data: 1024` - Initialized data
- `bss: 88448` - Uninitialized data

### Build Troubleshooting

#### Problem: "undefined reference to `function`"

**Solution**: Missing source file or library

```makefile
# Add missing source
APPL_CSRC += $(APP_PATH)/missing_file.c

# Or add missing library
LIB_SEL += required_library
```

#### Problem: "No such file or directory" (include)

**Solution**: Missing include path

```makefile
APPL_INCLUDES += -I$(EPII_ROOT)/path/to/includes
```

#### Problem: "region 'FLASH' overflowed"

**Solution**: Program too large, optimize:

```makefile
# Increase optimization
APPL_OPT = -Os  # Size optimization

# Remove unused features
APPL_DEFINES += -DREMOVE_UNUSED_FEATURES=1

# Check what's using space
arm-none-eabi-nm --size-sort --print-size output.elf | tail -20
```

#### Problem: "region 'SRAM' overflowed"

**Solution**: Too much RAM usage

```bash
# Check stack usage
arm-none-eabi-objdump -t output.elf | grep stack

# Reduce tensor arena size
# In inference.cpp:
constexpr int kTensorArenaSize = 80 * 1024;  // Reduce from 100KB
```

### Custom Build Script

**File**: `scripts/build/build_amr.sh`

```bash
#!/bin/bash

##
# Build script for Water Meter AMR application
##

set -e  # Exit on error

APP_NAME="water_meter_amr"
BUILD_DIR="EPII_CM55M_APP_S"
OUTPUT_DIR="output_${APP_NAME}_gnu_epii_CM55M_s"

echo "========================================"
echo "  Building AMR Application"
echo "========================================"
echo ""

# Navigate to build directory
cd ${BUILD_DIR}

# Clean previous build
echo "Cleaning previous build..."
make clean APP=${APP_NAME}

# Build application
echo "Building ${APP_NAME}..."
make APP=${APP_NAME} -j$(nproc)

echo ""
echo "✓ Build complete!"
echo ""

# Show size
echo "Binary size:"
arm-none-eabi-size ${OUTPUT_DIR}/output.elf

echo ""
echo "Output files:"
ls -lh ${OUTPUT_DIR}/output.*

echo ""
echo "Flash command:"
echo "  ./scripts/flash/flash_amr.sh /dev/ttyACM0"
```

---

## Debugging and Testing

### Debugging Setup

#### Hardware Requirements

- **Debugger**: CMSIS-DAP, J-Link, or ST-Link compatible debugger
- **Connection**: SWD (Serial Wire Debug)
- **Pins**:
  - SWCLK - SWD Clock
  - SWDIO - SWD Data
  - GND - Ground
  - VDD - Power (3.3V)

#### Software Requirements

```bash
# Install OpenOCD (on-chip debugger)
sudo apt-get install openocd

# Install GDB for ARM
sudo apt-get install gdb-multiarch
# or
sudo apt-get install arm-none-eabi-gdb

# Install telnet (for OpenOCD interface)
sudo apt-get install telnet
```

### OpenOCD Configuration

**File**: `openocd_we2.cfg`

```tcl
# OpenOCD configuration for HIMAX WE2

# Interface (CMSIS-DAP example)
source [find interface/cmsis-dap.cfg]

# Transport
transport select swd

# Target chip: Cortex-M55
set CHIPNAME we2
set CPUTAPID 0x6ba02477

# Create target
swd newdap $CHIPNAME cpu -expected-id $CPUTAPID
dap create $CHIPNAME.dap -chain-position $CHIPNAME.cpu

# Create Cortex-M55 target
set _TARGETNAME $CHIPNAME.cpu
target create $_TARGETNAME cortex_m -dap $CHIPNAME.dap

# Work area (RAM for algorithm execution)
$_TARGETNAME configure -work-area-phys 0x20000000 -work-area-size 0x10000 -work-area-backup 0

# Flash configuration (if applicable)
# flash bank we2_flash <driver> <base> <size> ...

# Reset configuration
reset_config srst_only

# Debugging speed
adapter speed 4000

# Enable semihosting
$_TARGETNAME configure -rtos auto
```

### Starting Debug Session

#### Terminal 1: OpenOCD Server

```bash
# Start OpenOCD
openocd -f openocd_we2.cfg
```

**Output:**
```
Open On-Chip Debugger 0.12.0
Info : CMSIS-DAP: SWD supported
Info : CMSIS-DAP: Interface ready
Info : clock speed 4000 kHz
Info : SWD DPIDR 0x6ba02477
Info : [we2.cpu] Cortex-M55 r0p0 detected
Info : [we2.cpu] detected core: ARMv8.1-M Mainline
Info : [we2.cpu] Listening on port 3333 for gdb connections
```

#### Terminal 2: GDB Client

```bash
# Start GDB
arm-none-eabi-gdb output.elf

# Connect to OpenOCD
(gdb) target extended-remote localhost:3333

# Reset and halt
(gdb) monitor reset halt

# Load program
(gdb) load

# Set breakpoint
(gdb) break main

# Continue execution
(gdb) continue
```

### GDB Commands Reference

#### Basic Commands

```gdb
# Load program to target
load

# Run program
run
continue
c

# Step execution
step        # Step into functions
next        # Step over functions
stepi       # Step one instruction
nexti       # Next instruction

# Halt execution
interrupt
Ctrl+C

# Reset
monitor reset halt
monitor reset run
```

#### Breakpoints and Watchpoints

```gdb
# Set breakpoint
break main
break camera.c:123
break ml_inference

# Conditional breakpoint
break main if argc > 1

# List breakpoints
info breakpoints

# Delete breakpoint
delete 1

# Set watchpoint (break on variable change)
watch my_variable

# Set hardware watchpoint
hbreak address
```

#### Examining Code and Memory

```gdb
# Disassemble
disassemble main
disas /m main    # With source

# Print variable
print variable_name
print *pointer
print array[5]

# Print in different formats
print /x variable    # Hex
print /d variable    # Decimal
print /t variable    # Binary
print /a variable    # Address

# Examine memory
x/10x 0x20000000     # 10 hex words
x/10b 0x20000000     # 10 bytes
x/10i 0x08000000     # 10 instructions

# Display (auto-print on each step)
display variable
undisplay 1
```

#### Stack and Registers

```gdb
# Backtrace (call stack)
backtrace
bt

# Frame selection
frame 0
up
down

# Print all registers
info registers

# Print specific register
print $r0
print $sp
print $pc

# Set register
set $r0 = 0x12345678
```

### Printf-Style Debugging

#### UART Output Redirection

```c
// In your code
printf("Debug: value = %d\n", value);
printf("Camera frame captured\n");
printf("Inference result: %d (%.2f%%)\n", result, confidence * 100.0f);
```

Monitor via serial terminal:
```bash
# Using screen
screen /dev/ttyACM0 921600

# Using minicom
minicom -D /dev/ttyACM0 -b 921600

# Using pyserial
python3 -m serial.tools.miniterm /dev/ttyACM0 921600
```

#### Debug Levels

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

### Semihosting

Semihosting allows printf output through the debugger without UART:

```c
// Enable in your code
extern void initialise_monitor_handles(void);

int main(void)
{
    initialise_monitor_handles();  // Enable semihosting

    printf("Hello via semihosting!\n");
    // ...
}
```

**Build with semihosting:**
```makefile
APPL_LDFLAGS += --specs=rdimon.specs -lrdimon
```

**GDB command:**
```gdb
(gdb) monitor arm semihosting enable
```

### Testing Strategies

#### Unit Testing

**File**: `tests/unit/test_digit_detection.c`

```c
#include <assert.h>
#include <stdio.h>
#include "digit_detection.h"

void test_digit_detection_init(void)
{
    printf("Test: digit_detection_init()...");

    int result = digit_detection_init();
    assert(result == 0);

    printf(" PASS\n");
}

void test_extract_digit_roi(void)
{
    printf("Test: extract_digit_roi()...");

    uint8_t image[640 * 480];
    uint8_t roi[28 * 28];

    // Fill test image
    memset(image, 128, sizeof(image));

    // Extract ROI
    int result = extract_digit_roi(image, 0, roi);
    assert(result == 0);

    printf(" PASS\n");
}

int main(void)
{
    printf("\n");
    printf("========================================\n");
    printf("  Unit Tests: Digit Detection\n");
    printf("========================================\n");
    printf("\n");

    test_digit_detection_init();
    test_extract_digit_roi();

    printf("\n✓ All tests passed!\n\n");
    return 0;
}
```

**Build unit test:**
```bash
gcc -o test_digit_detection \
    test_digit_detection.c \
    ../src/digit_detection.c \
    -I../include \
    -DUNIT_TEST

./test_digit_detection
```

#### Integration Testing

Test complete workflows:

```c
int test_complete_amr_cycle(void)
{
    printf("Testing complete AMR cycle...\n");

    // 1. Initialize camera
    assert(camera_init() == 0);
    printf("  ✓ Camera initialized\n");

    // 2. Capture frame
    uint8_t image[640 * 480];
    assert(camera_capture_frame(image, 640, 480) == 0);
    printf("  ✓ Frame captured\n");

    // 3. Initialize ML
    assert(ml_init() == 0);
    printf("  ✓ ML initialized\n");

    // 4. Run inference
    int result;
    float confidence;
    uint8_t test_roi[28 * 28];
    memset(test_roi, 0, sizeof(test_roi));
    assert(ml_inference(test_roi, &result, &confidence) == 0);
    printf("  ✓ Inference successful\n");

    // 5. Initialize LoRaWAN
    assert(lorawan_init() == 0);
    printf("  ✓ LoRaWAN initialized\n");

    printf("✓ Complete AMR cycle test passed\n");
    return 0;
}
```

#### Performance Testing

**Measure inference time:**

```c
void test_inference_performance(void)
{
    printf("Testing inference performance...\n");

    uint8_t test_image[28 * 28];
    int result;
    float confidence;

    const int NUM_ITERATIONS = 100;
    uint32_t total_time = 0;

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        uint32_t start = hx_drv_timer_get_ms();
        ml_inference(test_image, &result, &confidence);
        uint32_t end = hx_drv_timer_get_ms();

        total_time += (end - start);
    }

    float avg_time = (float)total_time / NUM_ITERATIONS;

    printf("  Average inference time: %.2f ms\n", avg_time);
    printf("  Inferences per second: %.1f\n", 1000.0f / avg_time);

    assert(avg_time < 100.0f);  // Should be under 100ms
    printf("✓ Performance test passed\n");
}
```

**Measure memory usage:**

```c
void test_memory_usage(void)
{
    printf("Testing memory usage...\n");

    extern uint8_t __heap_start__;
    extern uint8_t __heap_end__;

    size_t heap_size = &__heap_end__ - &__heap_start__;

    printf("  Heap size: %zu bytes\n", heap_size);
    printf("  Stack pointer: %p\n", (void*)__get_MSP());

    // Check TFLite arena usage
    if (interpreter != nullptr) {
        printf("  TFLite arena used: %zu bytes\n", interpreter->arena_used_bytes());
    }
}
```

### Common Debugging Scenarios

#### Scenario 1: Hard Fault

**Symptoms:** Program crashes, HardFault_Handler called

**Debug steps:**

```gdb
# Set breakpoint on fault handler
(gdb) break HardFault_Handler

# When hit, examine fault registers
(gdb) info registers

# Check stack
(gdb) backtrace

# Common causes:
# - NULL pointer dereference
# - Stack overflow
# - Unaligned memory access
# - Invalid instruction
```

**Check fault status registers:**

```c
void HardFault_Handler(void)
{
    printf("\n***** HARD FAULT *****\n");

    // Print fault status
    uint32_t cfsr = SCB->CFSR;
    printf("CFSR: 0x%08lX\n", cfsr);

    if (cfsr & 0x0001) printf("  - Instruction access violation\n");
    if (cfsr & 0x0002) printf("  - Data access violation\n");
    if (cfsr & 0x0100) printf("  - Divide by zero\n");
    if (cfsr & 0x0200) printf("  - Unaligned access\n");

    // Print return address
    uint32_t* msp = (uint32_t*)__get_MSP();
    printf("PC: 0x%08lX\n", msp[6]);
    printf("LR: 0x%08lX\n", msp[5]);

    while (1);  // Halt
}
```

#### Scenario 2: Stack Overflow

**Detect:**

```c
// Check stack usage at runtime
void check_stack_usage(void)
{
    extern uint32_t __stack_top__;
    extern uint32_t __stack_limit__;

    uint32_t stack_size = &__stack_top__ - &__stack_limit__;
    uint32_t current_sp = __get_MSP();
    uint32_t stack_used = &__stack_top__ - current_sp;

    printf("Stack: %lu / %lu bytes used (%.1f%%)\n",
           stack_used, stack_size,
           (float)stack_used / stack_size * 100.0f);

    if (stack_used > stack_size * 0.9) {
        printf("WARNING: Stack usage > 90%%!\n");
    }
}
```

**Fix:** Reduce local variable size, increase stack size in linker script

#### Scenario 3: Memory Corruption

**Detect with canary values:**

```c
#define CANARY_VALUE 0xDEADBEEF

uint32_t buffer_canary_start = CANARY_VALUE;
uint8_t my_buffer[1024];
uint32_t buffer_canary_end = CANARY_VALUE;

void check_buffer_integrity(void)
{
    if (buffer_canary_start != CANARY_VALUE) {
        printf("ERROR: Buffer underflow detected!\n");
    }
    if (buffer_canary_end != CANARY_VALUE) {
        printf("ERROR: Buffer overflow detected!\n");
    }
}
```

### Automated Testing Scripts

**File**: `scripts/test/run_tests.sh`

```bash
#!/bin/bash

##
# Automated testing script
##

set -e

echo "========================================"
echo "  Running AMR Tests"
echo "========================================"
echo ""

# Build tests
echo "Building unit tests..."
cd tests/unit
make all

# Run tests
echo ""
echo "Running unit tests..."
./test_digit_detection
./test_ml_inference
./test_lorawan_comm

echo ""
echo "✓ All tests passed!"
```

### Debugging Checklist

Before releasing firmware:

- [ ] All printf debug statements reviewed (remove or disable in production)
- [ ] No hard-coded test values in production code
- [ ] Watchdog timer enabled
- [ ] Stack usage checked (< 80%)
- [ ] Heap usage checked (< 80%)
- [ ] All error paths tested
- [ ] Memory leaks checked (if using dynamic allocation)
- [ ] Timing verified (inference, LoRaWAN, sleep)
- [ ] Power consumption measured
- [ ] Flash and RAM usage within limits
- [ ] Unit tests passing
- [ ] Integration tests passing
- [ ] Field test successful

---

## Best Practices

### Code Organization
1. ✅ Separate concerns (recognition, communication, power management)
2. ✅ Use meaningful function and variable names
3. ✅ Add comments for complex logic
4. ✅ Keep functions small and focused
5. ✅ Use header files for interfaces

### Memory Management
1. ✅ Minimize dynamic allocation
2. ✅ Use static buffers where possible
3. ✅ Check stack usage
4. ✅ Monitor heap fragmentation
5. ✅ Use const for read-only data

### Error Handling
1. ✅ Check all return values
2. ✅ Use defensive programming
3. ✅ Add error logging
4. ✅ Implement watchdog timer
5. ✅ Handle edge cases

### Power Optimization
1. ✅ Use sleep modes when idle
2. ✅ Turn off unused peripherals
3. ✅ Optimize inference frequency
4. ✅ Use low-power camera modes
5. ✅ Minimize UART traffic

---

**End of Complete Firmware Development Guide**

See individual topic guides in `docs/03-development/firmware/` for deep dives into specific areas.
