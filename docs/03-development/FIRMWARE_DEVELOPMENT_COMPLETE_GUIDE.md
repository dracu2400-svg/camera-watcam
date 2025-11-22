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

[Content continues with detailed LoRaWAN integration...]

---

## Build System

[Detailed build system explanation...]

---

## Debugging and Testing

[Debugging guide...]

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
