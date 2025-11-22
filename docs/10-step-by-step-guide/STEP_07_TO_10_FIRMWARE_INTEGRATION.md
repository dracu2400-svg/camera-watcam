# Steps 7-10: Firmware Integration and Deployment

**Complete guide for integrating trained model into production firmware**

---

## Step 7: Integrate Model into Production Firmware

**Time Required:** 5-10 minutes
**Difficulty:** Medium

Now we'll create production firmware that uses your trained model for real digit recognition.

### 7.1 Create Production Firmware Directory

```bash
cd ~/development/Seeed_Grove_Vision_AI_Module_V2/EPII_CM55M_APP_S/app/scenario_app

# Copy complete AMR template
cp -r ~/development/camera-watcam/firmware-templates/complete-amr-app water_meter_amr

cd water_meter_amr
```

### 7.2 Copy Your Trained Model

```bash
# Create models directory
mkdir -p models

# Copy your trained model header
cp ~/development/camera-watcam/models/water_meter_digit_model.h \
   models/
```

**Verify model copied:**

```bash
ls -lh models/
```

**Expected output:**
```
-rw-rw-r-- 1 user user 25K Nov 22 14:35 water_meter_digit_model.h
```

### 7.3 Update Makefile

**Edit:** `water_meter_amr.mk`

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
    -I$(APP_PATH)/models

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

# Camera sensor (HM0360)
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
    -DDEBUG_LEVEL=2

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
    -fdata-sections

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
    -Wl,-Map=$(OUT_DIR)/output.map

APPL_LIBS = \
    -lm \
    -lc \
    -lgcc \
    -lstdc++
```

### 7.4 Create ML Inference Module

**Create:** `inference.cpp`

```cpp
/**
 * @file inference.cpp
 * @brief ML inference using TensorFlow Lite Micro
 */

#include <stdio.h>
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Include your trained model
#include "models/water_meter_digit_model.h"

// Tensor arena size (adjust if needed)
constexpr int kTensorArenaSize = 100 * 1024;  // 100KB
static uint8_t tensor_arena[kTensorArenaSize];

// Global pointers
static const tflite::Model* model = nullptr;
static tflite::MicroInterpreter* interpreter = nullptr;
static TfLiteTensor* input_tensor = nullptr;
static TfLiteTensor* output_tensor = nullptr;

extern "C" {

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
 * @return 0 on success, -1 on failure
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
 * @brief Deinitialize ML
 */
void ml_deinit(void)
{
    // Nothing to do for TFLite Micro
    printf("ML deinitialized\n");
}

} // extern "C"
```

### 7.5 Create Main Application

**Create:** `main.c`

```c
/**
 * @file main.c
 * @brief Water Meter AMR Main Application
 */

#include <stdio.h>
#include <string.h>
#include "WE2_device.h"
#include "hx_drv_timer.h"

// Module headers
#include "camera.h"
#include "inference.h"
#include "lorawan_comm.h"
#include "power_mgmt.h"
#include "digit_detection.h"

// Configuration
#define READING_INTERVAL_SECONDS 3600  // 1 hour
#define NUM_DIGITS 8                   // Number of digits on meter

// Credentials (from provisioning - replace with actual values)
#define DEV_EUI  "70B3D57ED0050001"
#define APP_EUI  "0000000000000000"
#define APP_KEY  "00000000000000000000000000000000"

int main(void)
{
    printf("\n");
    printf("========================================\n");
    printf("  Water Meter AMR Application\n");
    printf("  Version: 1.0.0\n");
    printf("========================================\n");
    printf("\n");

    // Initialize modules
    printf("Initializing modules...\n");

    if (camera_init() != 0) {
        printf("ERROR: Camera init failed\n");
        return -1;
    }
    printf("✓ Camera initialized\n");

    if (ml_init() != 0) {
        printf("ERROR: ML init failed\n");
        return -1;
    }
    printf("✓ ML initialized\n");

    if (lorawan_init() != 0) {
        printf("ERROR: LoRaWAN init failed\n");
        return -1;
    }
    printf("✓ LoRaWAN initialized\n");

    if (power_init() != 0) {
        printf("ERROR: Power init failed\n");
        return -1;
    }
    printf("✓ Power initialized\n");

    // Configure LoRaWAN (credentials from provisioning)
    printf("\nConfiguring LoRaWAN...\n");
    if (lorawan_configure_otaa(DEV_EUI, APP_EUI, APP_KEY) != 0) {
        printf("ERROR: LoRaWAN config failed\n");
        return -1;
    }

    // Join network
    printf("\nJoining LoRaWAN network...\n");
    if (lorawan_join(3) != 0) {
        printf("ERROR: Failed to join network\n");
        return -1;
    }

    printf("\n");
    printf("========================================\n");
    printf("✓ AMR System Ready\n");
    printf("========================================\n");
    printf("\n");

    // Main reading loop
    uint32_t reading_count = 0;

    while (1) {
        reading_count++;

        printf("\n");
        printf("========================================\n");
        printf("  Reading Cycle #%lu\n", reading_count);
        printf("========================================\n");
        printf("\n");

        // Capture image
        printf("Capturing image...\n");
        uint8_t image[640 * 480];
        if (camera_capture_frame(image, 640, 480) == 0) {
            printf("✓ Image captured (640x480)\n");

            // Detect and recognize digits
            uint8_t digits[NUM_DIGITS];
            uint8_t digit_count = 0;
            float total_confidence = 0.0f;

            printf("\nRecognizing digits...\n");

            for (int i = 0; i < NUM_DIGITS; i++) {
                // Extract digit ROI (Region of Interest) and resize to 28x28
                uint8_t digit_roi[28 * 28];
                if (extract_digit_roi(image, i, digit_roi) == 0) {

                    // Run inference
                    int result;
                    float confidence;
                    if (ml_inference(digit_roi, &result, &confidence) == 0) {
                        digits[digit_count] = result;
                        total_confidence += confidence;
                        digit_count++;

                        printf("  Digit %d: %d (%.2f%% confidence)\n",
                               i, result, confidence * 100.0f);
                    } else {
                        printf("  Digit %d: Inference failed\n", i);
                    }
                } else {
                    printf("  Digit %d: ROI extraction failed\n", i);
                }
            }

            if (digit_count > 0) {
                float avg_confidence = total_confidence / digit_count;

                printf("\nMeter Reading: ");
                for (int i = 0; i < digit_count; i++) {
                    printf("%d", digits[i]);
                }
                printf("\n");
                printf("Average Confidence: %.2f%%\n", avg_confidence * 100.0f);

                // Send reading via LoRaWAN
                printf("\nSending reading via LoRaWAN...\n");
                if (lorawan_send_meter_reading(digits, digit_count, avg_confidence) == 0) {
                    printf("✓ Reading sent successfully\n");
                } else {
                    printf("✗ Failed to send reading\n");
                }
            } else {
                printf("\n✗ No digits recognized\n");
            }
        } else {
            printf("✗ Image capture failed\n");
        }

        // Sleep until next reading
        printf("\nSleeping for %d seconds...\n", READING_INTERVAL_SECONDS);
        printf("Next reading at: +%d minutes\n\n", READING_INTERVAL_SECONDS / 60);

        power_enter_sleep(READING_INTERVAL_SECONDS);
    }

    return 0;
}
```

### 7.6 Verify All Files in Place

```bash
cd ~/development/Seeed_Grove_Vision_AI_Module_V2/EPII_CM55M_APP_S/app/scenario_app/water_meter_amr

# Check file structure
tree -L 2
```

**Expected structure:**
```
water_meter_amr/
├── water_meter_amr.mk
├── main.c
├── inference.cpp
├── camera.c
├── lorawan_comm.c
├── power_mgmt.c
├── digit_detection.c
├── include/
│   ├── camera.h
│   ├── inference.h
│   ├── lorawan_comm.h
│   ├── power_mgmt.h
│   └── digit_detection.h
└── models/
    └── water_meter_digit_model.h
```

✅ **Model integrated into firmware source!**

---

## Step 8: Build Production Firmware

**Time Required:** 5 minutes
**Difficulty:** Easy

### 8.1 Navigate to Build Directory

```bash
cd ~/development/Seeed_Grove_Vision_AI_Module_V2/EPII_CM55M_APP_S
```

### 8.2 Clean Previous Build

```bash
# Clean any previous builds
make clean APP=water_meter_amr
```

**Expected output:**
```
Cleaning water_meter_amr...
Removed build artifacts
```

### 8.3 Build Firmware

```bash
# Build with parallel jobs (faster)
make APP=water_meter_amr -j$(nproc)
```

**Expected output:**
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

   text    data     bss     dec     hex filename
 312456   2048  154624  469128   727e8 output.elf

✓ Build complete!

Output directory: output_water_meter_amr_gnu_epii_CM55M_s/
Files created:
  - output.elf (with debug symbols)
  - output.bin (binary for flashing)
  - output.hex (hex format)
  - output.map (memory map)
```

### 8.4 Analyze Memory Usage

**Check memory map:**

```bash
cd output_water_meter_amr_gnu_epii_CM55M_s

# Show size breakdown
arm-none-eabi-size output.elf
```

**Expected output:**
```
   text    data     bss     dec     hex filename
 312456   2048  154624  469128   727e8 output.elf
```

**What this means:**
- **text (312KB):** Your code + your model (25KB is embedded here)
- **data (2KB):** Initialized global variables
- **bss (154KB):** Uninitialized data (mainly TFLite tensor arena 100KB)
- **Total RAM:** data + bss = 156KB (well within 2MB SRAM limit)
- **Total Flash:** text + data = 314KB (well within 16MB Flash limit)

**Check largest symbols:**

```bash
arm-none-eabi-nm --size-sort --print-size output.elf | tail -20
```

**You should see your model:**
```
00006180 digit_recognition_model_len
00006180 t tflite::ops::micro::...
00018c20 d digit_recognition_model    ← Your model (24KB)
```

✅ **Firmware built successfully!**

### 8.5 Verify Model is Embedded

```bash
# Search for model symbol
arm-none-eabi-nm output.elf | grep digit_recognition_model
```

**Expected output:**
```
20003c20 d digit_recognition_model
20003c20 d digit_recognition_model_len
```

This confirms your trained model is embedded in the firmware!

---

## Step 9: Flash and Test on Device

**Time Required:** 10 minutes
**Difficulty:** Easy

### 9.1 Prepare Device for Flashing

**Put WE2 in bootloader mode:**

1. Press and HOLD **BOOT** button
2. While holding BOOT, press and release **RESET** button
3. Release **BOOT** button
4. Device is now waiting for firmware

### 9.2 Flash Firmware

**Method A: Using Flash Script (Recommended)**

```bash
cd ~/development/camera-watcam/scripts/flash

# Flash firmware
./flash_amr.sh /dev/ttyACM0
```

**Method B: Manual xmodem**

```bash
cd ~/development/Seeed_Grove_Vision_AI_Module_V2/EPII_CM55M_APP_S/output_water_meter_amr_gnu_epii_CM55M_s

# Send binary via xmodem
sx output.bin < /dev/ttyACM0 > /dev/ttyACM0
```

**Expected output:**
```
Sending output.bin...
Sending in Batch Mode
Bytes Sent: 312456   BPS:9600

Transfer complete
```

### 9.3 Monitor Device Boot

**Open serial terminal:**

```bash
screen /dev/ttyACM0 921600
```

**Press RESET button on WE2**

**Expected output:**
```
========================================
  Water Meter AMR Application
  Version: 1.0.0
========================================

Initializing modules...
Initializing camera...
✓ Camera initialized
Initializing TensorFlow Lite Micro...
✓ TFLite initialized successfully
  Model size: 24832 bytes
  Input:  28 x 28 x 1 (INT8)
  Output: 10 classes (INT8)
  Arena used: 98304 / 102400 bytes
✓ ML initialized
Initializing LoRaWAN...
✓ LoRaWAN UART initialized (115200 baud)
Testing AT communication...
✓ Modem responding
✓ LoRaWAN initialized
✓ Power initialized

Configuring LoRaWAN...
✓ OTAA credentials configured

Joining LoRaWAN network...
Join attempt 1/3...
✓ Successfully joined LoRaWAN network

========================================
✓ AMR System Ready
========================================

========================================
  Reading Cycle #1
========================================

Capturing image...
✓ Image captured (640x480)

Recognizing digits...
  Inference time: 23 ms
  Predicted class: 0 (confidence: 98.43%)
  Digit 0: 0 (98.43% confidence)
  Inference time: 21 ms
  Predicted class: 0 (confidence: 99.12%)
  Digit 1: 0 (99.12% confidence)
  Inference time: 22 ms
  Predicted class: 0 (confidence: 97.65%)
  Digit 2: 0 (97.65% confidence)
  Inference time: 23 ms
  Predicted class: 1 (confidence: 96.78%)
  Digit 3: 1 (96.78% confidence)
  Inference time: 21 ms
  Predicted class: 2 (confidence: 98.34%)
  Digit 4: 2 (98.34% confidence)
  Inference time: 22 ms
  Predicted class: 3 (confidence: 97.89%)
  Digit 5: 3 (97.89% confidence)
  Inference time: 23 ms
  Predicted class: 4 (confidence: 99.01%)
  Digit 6: 4 (99.01% confidence)
  Inference time: 21 ms
  Predicted class: 5 (confidence: 98.23%)
  Digit 7: 5 (98.23% confidence)

Meter Reading: 00012345
Average Confidence: 98.18%

Sending reading via LoRaWAN...
Sending 10 bytes on port 10 (confirmed=1)...
Data: 08610001020304055000
✓ Data sent successfully
✓ Reading sent successfully

Sleeping for 3600 seconds...
Next reading at: +60 minutes
```

### 9.4 Verify System is Working

**Check these indicators:**

✅ **Model loaded:** "Model size: 24832 bytes"
✅ **Arena allocated:** "Arena used: 98304 / 102400 bytes"
✅ **Inference working:** "Predicted class: X (confidence: XX%)"
✅ **Inference speed:** Should be 15-30ms per digit
✅ **Accuracy:** Confidence should be >90% for good images
✅ **LoRaWAN working:** "Data sent successfully"

### 9.5 Test with Your Water Meter

**Position your water meter in front of camera:**

1. Ensure all digits are visible
2. Good lighting (no shadows, no glare)
3. Press RESET to trigger new reading cycle
4. Watch serial output for recognition results

**Manually verify accuracy:**

```
Meter Reading: 00012345  ← What device detected
```

Compare with actual meter reading by visual inspection.

**Calculate accuracy:**
- All 8 digits correct: 100% accuracy ✓
- 7/8 correct: 87.5% accuracy
- <6/8 correct: Need to improve (retrain or adjust camera)

✅ **System is working on device!**

---

## Step 10: Production Deployment

**Time Required:** Varies (1 hour to several days depending on scale)
**Difficulty:** Medium to Hard

### 10.1 Pre-Deployment Checklist

Before deploying to production devices:

**Model Performance:**
- ✅ Validation accuracy >95%
- ✅ Field testing accuracy >93%
- ✅ Inference time <30ms
- ✅ Model size <30KB

**Firmware:**
- ✅ All features working (camera, ML, LoRaWAN, power)
- ✅ Memory usage <80% (Flash and RAM)
- ✅ Error handling implemented
- ✅ Watchdog timer enabled
- ✅ Deep sleep working
- ✅ Battery life projections acceptable

**Testing:**
- ✅ Tested on multiple meter types
- ✅ Tested in various lighting conditions
- ✅ Tested temperature range (-20°C to +60°C)
- ✅ Long-term stability test (48-72 hours)
- ✅ Network connectivity tested
- ✅ LoRaWAN uplink success >99%

### 10.2 Provisioning Preparation

**For each device, you need:**

1. **Unique DevEUI** (Device EUI)
2. **AppEUI** (Application EUI - same for all devices)
3. **AppKey** (Application Key - unique per device)
4. **Serial number** for tracking

**Generate credentials for batch:**

```bash
cd ~/development/camera-watcam/tools/provisioning/batch

# Generate 100 device credentials
python3 generate_credentials.py \
    --count 100 \
    --output batch_production_001.csv \
    --batch-number PROD-BATCH-001 \
    --dev-eui-prefix 70B3D57ED005
```

**Output:** `batch_production_001.csv`

```csv
serial_number,dev_eui,app_eui,app_key,device_name,batch_number,firmware_version
WM-PROD-0001,70B3D57ED0050001,0000000000000000,A1B2C3D4...,WM-Device-0001,PROD-BATCH-001,1.0.0
WM-PROD-0002,70B3D57ED0050002,0000000000000000,B2C3D4E5...,WM-Device-0002,PROD-BATCH-001,1.0.0
...
WM-PROD-0100,70B3D57ED0050064,0000000000000000,C3D4E5F6...,WM-Device-0100,PROD-BATCH-001,1.0.0
```

### 10.3 Flash Production Firmware at Scale

**Setup:**
- Multiple programming stations (8-16 devices in parallel)
- Gang programmers
- Automated handlers (optional)

**Process per device:**

```bash
# 1. Put device in bootloader mode
# 2. Flash firmware
./flash_amr.sh /dev/ttyACM0

# 3. Verify boot
screen /dev/ttyACM0 921600
# Check for "AMR System Ready" message

# 4. Record serial number
# 5. Move to provisioning station
```

**Throughput:**
- Manual: 5-10 devices/hour/station
- Automated: 20-30 devices/hour/station
- With 8 stations: 160-240 devices/hour

### 10.4 Provision Devices

```bash
cd ~/development/camera-watcam/tools/provisioning/batch

# Provision batch with 8 parallel stations
python3 batch_provisioner.py \
    --input batch_production_001.csv \
    --parallel 8 \
    --network-server chirpstack \
    --generate-qr \
    --ports /dev/ttyACM0,/dev/ttyACM1,...,/dev/ttyACM7
```

**Provisioning does:**
1. Connect to device via UART
2. Write DevEUI, AppEUI, AppKey
3. Verify credentials stored
4. Register on network server
5. Generate QR code label
6. Record in database

**Throughput:** 30-50 devices/minute (with 8-10 stations)

### 10.5 Quality Assurance Testing

**For each device or sample batch:**

1. **Functional Test:**
   - Power on test
   - Camera capture test
   - ML inference test (using test image)
   - LoRaWAN join test
   - Sleep/wake test

2. **Accuracy Test:**
   - Test on 3-5 different meters
   - Verify >95% digit accuracy
   - Check confidence scores >90%

3. **Network Test:**
   - Join test (should succeed within 30 seconds)
   - Uplink test (verify data reaches cloud)
   - RSSI/SNR measurement
   - Verify retry logic

4. **Environmental Test (Sample):**
   - Temperature test: -20°C to +60°C
   - Humidity test: 0-95% RH
   - Vibration test (if applicable)

**Acceptance Criteria:**
- ✅ First-pass yield >98%
- ✅ Defect rate <0.1%
- ✅ All tests passing

### 10.6 Packaging and Labeling

**Each device should have:**

1. **QR Code Label** with:
   - Serial number
   - DevEUI
   - Batch number
   - Manufacturing date
   - Firmware version

2. **Packaging:**
   - Anti-static bag
   - Desiccant pack
   - Quick start guide
   - Installation instructions

3. **Documentation:**
   - Warranty card
   - Compliance certificates
   - Safety information

### 10.7 Warehouse and Logistics

**Inventory management:**
- Track inventory by batch
- FIFO (First-In-First-Out)
- Climate control (if needed)
- Quality shelf-life tracking

**Shipping:**
- Corrugated boxes (50-100 devices per box)
- Foam inserts for protection
- Shipping labels
- Track shipments

### 10.8 Field Installation

**Installation process:**

1. **Pre-installation:**
   - Site survey
   - Verify LoRaWAN coverage
   - Identify meter locations

2. **Installation:**
   - Mount device on/near meter
   - Align camera to meter display
   - Verify all digits visible
   - Connect power (battery)
   - Verify power LED

3. **Commissioning:**
   - Device attempts join (automatic)
   - Verify join success
   - Trigger test reading
   - Verify data in cloud

4. **Handoff:**
   - Record installation location
   - Take photos
   - Update asset database
   - Provide customer access

**Installation time:** 10-20 minutes per device

### 10.9 Monitoring and Support

**Cloud monitoring:**
- Device online/offline status
- Battery level
- Reading success rate
- RSSI/SNR
- Last seen time
- Firmware version

**Alerts:**
- Low battery (<20%)
- Device offline (>24 hours)
- Poor signal (RSSI < -120 dBm)
- Reading failures

**Support process:**
- Remote troubleshooting via logs
- OTA firmware updates
- RMA for hardware failures
- Field service dispatch

### 10.10 Success Metrics

**Track these KPIs:**

| Metric | Target | Monitoring |
|--------|--------|------------|
| **Uptime** | >99.5% | 24/7 |
| **Join Success** | >95% | Per batch |
| **Data Accuracy** | >95% | Sample testing |
| **Battery Life** | >12 months | Projected from drain rate |
| **Defect Rate** | <0.1% | Manufacturing QA |
| **RMA Rate** | <1% | First year |

✅ **Ready for Production Deployment!**

---

## Troubleshooting

### Common Issues and Solutions

**Issue 1: Model doesn't load (ERROR: Model schema version mismatch)**

**Cause:** TFLite version mismatch

**Solution:**
- Rebuild model with correct TensorFlow Lite version
- Check TFLite Micro library version in SDK
- Regenerate model header

**Issue 2: Low inference accuracy on device (<80%)**

**Cause:** Camera alignment, lighting, or model quality

**Solution:**
- Verify camera alignment (all digits visible)
- Improve lighting (add LED if needed)
- Clean meter display
- Retrain model with more/better images
- Check preprocessing matches training

**Issue 3: Inference very slow (>50ms)**

**Cause:** Not using NPU or large model

**Solution:**
- Verify Ethos-U55 NPU is enabled in build
- Check tensor arena size (might be too small, causing fallback)
- Reduce model complexity if needed

**Issue 4: LoRaWAN join fails**

**Cause:** Credentials wrong or network issues

**Solution:**
- Verify DevEUI, AppEUI, AppKey are correct
- Check device is registered on network server
- Verify gateway coverage (RSSI should be > -120 dBm)
- Try moving closer to gateway
- Check frequency plan matches region

**Issue 5: High battery drain**

**Cause:** Not entering deep sleep or frequent readings

**Solution:**
- Verify deep sleep is working (measure current, should be <10 µA)
- Increase reading interval
- Fix any infinite loops
- Check for join failures causing retries

**Issue 6: "Out of memory" during build**

**Cause:** Model too large or too many features

**Solution:**
- Reduce tensor arena size in inference.cpp
- Remove unused libraries from makefile
- Optimize code size with `-Os` flag
- Check memory map for largest symbols

**Issue 7: Camera captures blank/black images**

**Cause:** Camera not initialized or wrong sensor

**Solution:**
- Verify camera sensor type in makefile
- Check camera connection (ribbon cable)
- Verify sensor power supply
- Test with simple capture example

**Issue 8: Digits not detected (all zeros or random)**

**Cause:** ROI extraction failing or wrong positioning

**Solution:**
- Verify ROI extraction logic
- Check digit positions/coordinates
- Print debug images to verify ROI contents
- Test with known-good digit images

---

## Next Steps

### For Ongoing Operations

1. **Monitor Performance:**
   - Set up cloud dashboard
   - Track device metrics
   - Monitor data quality

2. **Continuous Improvement:**
   - Collect edge cases
   - Retrain model periodically
   - Add new meter types to training set

3. **Firmware Updates:**
   - Implement OTA update process
   - Version control firmware
   - Test updates on pilot devices first

4. **Scale Operations:**
   - Automate provisioning
   - Increase manufacturing capacity
   - Optimize logistics

### For Further Development

1. **Add Features:**
   - Anomaly detection (leaks)
   - Battery optimization
   - Local data caching
   - Multi-meter support

2. **Improve ML Model:**
   - Add more training data
   - Try different architectures
   - Implement uncertainty quantification
   - Add OCR for alphanumeric displays

3. **Enhance Reliability:**
   - Add redundancy
   - Improve error recovery
   - Implement self-diagnostics
   - Add remote debugging

---

## Summary

You've successfully:

✅ **Step 1:** Set up development environment
✅ **Step 2:** Built and flashed training firmware
✅ **Step 3:** Launched training GUI
✅ **Step 4:** Collected and labeled 1000+ digit images
✅ **Step 5:** Trained CNN model (96%+ accuracy)
✅ **Step 6:** Exported model to TFLite and C header
✅ **Step 7:** Integrated model into production firmware
✅ **Step 8:** Built production firmware
✅ **Step 9:** Flashed and tested on device
✅ **Step 10:** Prepared for production deployment

**Total workflow time:** ~3-4 hours

**Result:** Production-ready Water Meter AMR system with your custom-trained model!

---

**Questions or Issues?**

- Check [Troubleshooting](#troubleshooting) section
- Review main project documentation
- Check GitHub issues
- Contact support

**End of Guide**
