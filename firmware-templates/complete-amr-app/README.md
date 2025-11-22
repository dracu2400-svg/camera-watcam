# Water Meter AMR Complete Firmware Template

This is a complete, production-ready firmware template for the Water Meter Automatic Meter Reading (AMR) system.

## Overview

This firmware integrates:
- **Camera capture** (HIMAX WE2 + HM0360 sensor)
- **ML inference** (TensorFlow Lite Micro for digit recognition)
- **LoRaWAN communication** (STM32WL modem)
- **Power management** (Deep sleep for battery operation)

## Directory Structure

```
complete-amr-app/
├── README.md                    # This file
├── water_meter_amr.mk           # Makefile for building
├── src/                         # Source files
│   ├── main.c                   # Main application
│   ├── inference.cpp            # ML inference (TFLite Micro)
│   ├── camera.c                 # Camera interface
│   ├── lorawan_comm.c           # LoRaWAN communication
│   ├── power_mgmt.c             # Power management
│   └── digit_detection.c        # Digit ROI extraction
├── include/                     # Header files
│   ├── camera.h
│   ├── inference.h
│   ├── lorawan_comm.h
│   ├── power_mgmt.h
│   └── digit_detection.h
├── models/                      # Trained model (add your model here)
│   └── water_meter_digit_model.h  (placeholder - copy your trained model here)
└── config/                      # Configuration files
    └── (reserved for future config files)
```

## Prerequisites

1. **HIMAX WE2 SDK** installed
2. **ARM GNU Toolchain 13.2+** installed
3. **Trained model** (see `docs/10-step-by-step-guide/` for training instructions)

## How to Use This Template

### Step 1: Copy Template to SDK

```bash
# Set SDK path
export SDK_PATH=~/development/Seeed_Grove_Vision_AI_Module_V2

# Copy template to SDK application directory
cp -r complete-amr-app $SDK_PATH/EPII_CM55M_APP_S/app/scenario_app/water_meter_amr
```

### Step 2: Add Your Trained Model

```bash
# Copy your trained model header to the firmware
cp path/to/your/water_meter_digit_model.h \
   $SDK_PATH/EPII_CM55M_APP_S/app/scenario_app/water_meter_amr/models/
```

The model header should contain:
- `digit_recognition_model[]` - Model data array
- `digit_recognition_model_len` - Model size

### Step 3: Configure LoRaWAN Credentials

Edit `src/main.c` and update:

```c
#define DEV_EUI  "YOUR_DEVICE_EUI"
#define APP_EUI  "YOUR_APP_EUI"
#define APP_KEY  "YOUR_APP_KEY"
```

### Step 4: Configure Digit Positions

Edit `src/digit_detection.c` and adjust the `digit_rois[]` array for your meter's digit positions:

```c
static const digit_roi_config_t digit_rois[8] = {
    {x, y, width, height},  // Digit 0
    {x, y, width, height},  // Digit 1
    // ... etc
};
```

To find the correct positions:
1. Capture a test image
2. Use an image viewer to find pixel coordinates
3. Update the ROI positions

### Step 5: Build Firmware

```bash
cd $SDK_PATH/EPII_CM55M_APP_S

# Clean previous builds
make clean APP=water_meter_amr

# Build firmware
make APP=water_meter_amr -j$(nproc)
```

### Step 6: Flash to Device

```bash
# Put device in bootloader mode (BOOT + RESET buttons)

# Flash firmware
cd ~/camera-watcam/scripts/flash
./flash_amr.sh /dev/ttyACM0
```

### Step 7: Test

```bash
# Open serial terminal
screen /dev/ttyACM0 921600

# Press RESET button on device
# You should see initialization messages and reading cycles
```

## Configuration Options

### Reading Interval

Edit `src/main.c`:

```c
#define READING_INTERVAL_SECONDS 3600  // 1 hour (default)
```

### Number of Digits

Edit `src/main.c`:

```c
#define NUM_DIGITS 8  // Adjust for your meter
```

### Camera Settings

Edit `src/camera.c`:

```c
#define CAMERA_WIDTH  640
#define CAMERA_HEIGHT 480
#define CAMERA_FPS    15
```

### LoRaWAN Settings

Edit `src/lorawan_comm.c`:

```c
#define LORAWAN_UART_BAUD    115200
#define LORAWAN_TX_PORT      10
```

## Memory Usage

Typical memory footprint:

| Component | Flash | SRAM |
|-----------|-------|------|
| Application code | ~80KB | ~10KB |
| TFLite library | ~200KB | ~50KB |
| Trained model | ~25KB | 0KB |
| Tensor arena | 0KB | ~100KB |
| **Total** | **~312KB** | **~156KB** |

Available:
- Flash: 16 MB (1.9% used)
- SRAM: 2 MB (7.6% used)

## Power Consumption

Typical power profile:

| Mode | Current | Duration |
|------|---------|----------|
| Deep sleep | 10 µA | 3590 sec |
| Wake + init | 50 mA | 2 sec |
| Camera capture | 80 mA | 1 sec |
| ML inference (8 digits) | 100 mA | 2 sec |
| LoRaWAN TX | 120 mA | 5 sec |

**Average:** ~0.5 mA (assuming 1-hour reading interval)

**Battery life:** ~6 months on 2000 mAh battery

## Troubleshooting

### Build Errors

**Error:** `fatal error: water_meter_digit_model.h: No such file or directory`

**Solution:** Copy your trained model header to `models/` directory.

**Error:** `undefined reference to 'tflite::...'`

**Solution:** Ensure TFLite library is included in Makefile:
```makefile
LIB_SEL += tflmtag2209_u55tag2205
```

### Runtime Issues

**Issue:** Camera captures black images

**Solution:**
- Check camera connection (ribbon cable)
- Verify camera sensor type in Makefile
- Increase exposure time

**Issue:** Low recognition accuracy

**Solution:**
- Adjust digit ROI positions
- Improve lighting conditions
- Retrain model with more/better images
- Enable preprocessing: `preprocess_digit(digit_roi)`

**Issue:** LoRaWAN join fails

**Solution:**
- Verify credentials (DevEUI, AppEUI, AppKey)
- Check gateway coverage (move closer)
- Verify device is registered on network server

## API Reference

### Camera Module (`camera.h`)

```c
int camera_init(void);
int camera_capture_frame(uint8_t* buffer, uint16_t width, uint16_t height);
void camera_deinit(void);
```

### Inference Module (`inference.h`)

```c
int ml_init(void);
int ml_inference(const uint8_t* image, int* result, float* confidence);
void ml_deinit(void);
```

### LoRaWAN Module (`lorawan_comm.h`)

```c
int lorawan_init(void);
int lorawan_configure_otaa(const char* dev_eui, const char* app_eui, const char* app_key);
int lorawan_join(uint8_t max_attempts);
int lorawan_send_meter_reading(const uint8_t* digits, uint8_t count, float confidence);
```

### Power Management Module (`power_mgmt.h`)

```c
int power_init(void);
int power_enter_sleep(uint32_t sleep_seconds);
int8_t power_get_battery_percentage(void);
```

### Digit Detection Module (`digit_detection.h`)

```c
int extract_digit_roi(const uint8_t* image, uint8_t digit_index, uint8_t* output);
int preprocess_digit(uint8_t* digit);
int8_t validate_image_quality(const uint8_t* image);
```

## Customization

This template is designed to be easily customized for different:
- Water meter types (adjust digit positions)
- Communication protocols (replace LoRaWAN with WiFi, NB-IoT, etc.)
- ML models (replace digit recognition with other models)
- Power sources (solar, mains, etc.)

## License

See project root LICENSE file.

## Support

For issues and questions:
- Check main project documentation: `docs/10-step-by-step-guide/`
- Review troubleshooting sections
- Open an issue on project GitHub repository

## Version History

- **v1.0.0** - Initial release
  - Complete AMR firmware implementation
  - TensorFlow Lite Micro integration
  - LoRaWAN communication
  - Power management with deep sleep
  - Digit detection and ROI extraction
