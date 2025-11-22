# Step-by-Step Guide: Complete Workflow from Image Acquisition to Production Deployment

**Complete Guide for Water Meter Digit Recognition**

Version: 1.0.0
Last Updated: 2024-11-22

---

## Table of Contents

1. [Workflow Overview](#workflow-overview)
2. [What You'll Need](#what-youll-need)
3. [Step 1: Environment Setup](#step-1-environment-setup)
4. [Step 2: Build and Flash Training Firmware](#step-2-build-and-flash-training-firmware)
5. [Step 3: Launch Training GUI](#step-3-launch-training-gui)
6. [Step 4: Image Acquisition and Labeling](#step-4-image-acquisition-and-labeling)
7. [Step 5: Train the Model](#step-5-train-the-model)
8. [Step 6: Export Model for Firmware](#step-6-export-model-for-firmware)
9. [Step 7: Integrate Model into Production Firmware](#step-7-integrate-model-into-production-firmware)
10. [Step 8: Build Production Firmware](#step-8-build-production-firmware)
11. [Step 9: Flash and Test on Device](#step-9-flash-and-test-on-device)
12. [Step 10: Production Deployment](#step-10-production-deployment)
13. [Troubleshooting](#troubleshooting)
14. [Next Steps](#next-steps)

---

## Workflow Overview

This guide will walk you through the **complete process** of creating a water meter digit recognition system:

```
┌─────────────────────────────────────────────────────────────┐
│                    COMPLETE WORKFLOW                        │
└─────────────────────────────────────────────────────────────┘

Step 1: SETUP (30 min)
   └─► Install tools, clone repo, prepare hardware
       │
Step 2: FLASH TRAINING FIRMWARE (10 min)
   └─► Build custom firmware for image capture
       │
Step 3: LAUNCH GUI (2 min)
   └─► Start training GUI application
       │
Step 4: ACQUIRE IMAGES (1-2 hours)
   └─► Capture 1000+ digit images with labels
       │
Step 5: TRAIN MODEL (5-10 min)
   └─► Train CNN model with TensorFlow
       │
Step 6: EXPORT MODEL (2 min)
   └─► Convert to TFLite and generate C header
       │
Step 7: INTEGRATE MODEL (5 min)
   └─► Copy model header into production firmware
       │
Step 8: BUILD FIRMWARE (5 min)
   └─► Compile production firmware with model
       │
Step 9: FLASH AND TEST (10 min)
   └─► Flash to device and verify accuracy
       │
Step 10: DEPLOY (varies)
   └─► Deploy to production devices
```

**Total Time:** ~3-4 hours for complete workflow

---

## What You'll Need

### Hardware

- ✅ **HIMAX WE2 development board**
- ✅ **Camera sensor** (HM0360, OV5647, or IMX219)
- ✅ **USB cable** for programming
- ✅ **Computer** (Linux/macOS/Windows)
- ✅ **Water meter(s)** for capturing digit images
- ✅ **(Optional) STM32WL** for LoRaWAN testing
- ✅ **(Optional) Debugger** (CMSIS-DAP, J-Link)

### Software

- ✅ **Python 3.7+**
- ✅ **ARM GNU Toolchain 13.2+**
- ✅ **HIMAX WE2 SDK**
- ✅ **Git**
- ✅ **Serial terminal** (screen, minicom, or PuTTY)

### Knowledge Prerequisites

- Basic command line usage
- Basic Python understanding (helpful but not required)
- Understanding of file paths and directories

---

## Step 1: Environment Setup

**Time Required:** 30 minutes
**Difficulty:** Easy

### 1.1 Install ARM Toolchain

**On Linux (Ubuntu/Debian):**

```bash
# Download ARM GNU Toolchain
cd ~/Downloads
wget https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz

# Extract
tar -xf arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz

# Move to /opt
sudo mv arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi /opt/

# Add to PATH
echo 'export PATH="/opt/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc

# Verify
arm-none-eabi-gcc --version
```

**Expected output:**
```
arm-none-eabi-gcc (Arm GNU Toolchain 13.2.rel1) 13.2.0
```

**On macOS:**

```bash
# Using Homebrew
brew install --cask gcc-arm-embedded

# Verify
arm-none-eabi-gcc --version
```

**On Windows:**

```bash
# Download installer from:
# https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

# Run installer
# Add to PATH in System Environment Variables

# Verify in Command Prompt
arm-none-eabi-gcc --version
```

### 1.2 Clone HIMAX WE2 SDK

```bash
# Create development directory
mkdir -p ~/development
cd ~/development

# Clone SDK
git clone https://github.com/HimaxWiseEyePlus/Seeed_Grove_Vision_AI_Module_V2.git
cd Seeed_Grove_Vision_AI_Module_V2

# Checkout stable branch
git checkout main
```

### 1.3 Clone AMR Project

```bash
cd ~/development
git clone <your-amr-project-repo-url>
cd camera-watcam
```

### 1.4 Install Python Dependencies

```bash
# Install Python 3.7+ if not already installed
python3 --version  # Should show 3.7 or higher

# Create virtual environment (recommended)
python3 -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install training GUI dependencies
pip install --upgrade pip
pip install -r tools/training-gui/requirements.txt
```

**Expected packages installed:**
- tensorflow>=2.10.0
- opencv-python>=4.6.0
- numpy>=1.23.0
- Pillow>=9.2.0
- pyserial>=3.5

### 1.5 Install Serial Tools

**On Linux:**

```bash
sudo apt-get update
sudo apt-get install screen minicom

# Add user to dialout group for serial port access
sudo usermod -a -G dialout $USER

# Log out and log back in for group change to take effect
```

**On macOS:**

```bash
brew install screen
# Or use built-in screen command
```

**On Windows:**

- Install PuTTY: https://www.putty.org/
- Or install Tera Term: https://ttssh2.osdn.jp/

### 1.6 Setup Hardware

**Connect Camera to WE2:**

1. Locate camera connector on WE2 board
2. Align camera ribbon cable (blue side up or down - check pinout)
3. Insert gently into connector
4. Lock connector by pushing down clip

**Connect WE2 to Computer:**

1. Use USB cable to connect WE2 to computer
2. Board should power on (LED indicator)
3. Note the serial port device:
   - **Linux:** `/dev/ttyACM0` or `/dev/ttyUSB0`
   - **macOS:** `/dev/tty.usbmodem*` or `/dev/tty.usbserial*`
   - **Windows:** `COM3`, `COM4`, etc.

**Verify Serial Port:**

```bash
# Linux
ls -l /dev/ttyACM*

# macOS
ls -l /dev/tty.usb*

# Windows (in PowerShell)
Get-WmiObject Win32_SerialPort | Select-Object DeviceID, Description
```

### 1.7 Verify Setup

**Test Serial Connection:**

```bash
# Linux/macOS
screen /dev/ttyACM0 921600

# Windows (using PuTTY)
# Open PuTTY, select COM port, set baud rate to 921600, click Open

# Press RESET button on WE2
# You should see bootloader or previous firmware messages
# Press Ctrl+A then K to exit screen
```

**Test ARM Toolchain:**

```bash
cd ~/development/Seeed_Grove_Vision_AI_Module_V2/EPII_CM55M_APP_S

# Try building example
make clean APP=hello_world
make APP=hello_world

# Should complete without errors
```

✅ **Setup Complete!** You're ready to move to Step 2.

---

## Step 2: Build and Flash Training Firmware

**Time Required:** 10 minutes
**Difficulty:** Easy

The training firmware is a **special custom firmware** designed ONLY for capturing images and sending them to your PC. It doesn't do digit recognition - it just captures images.

### 2.1 Set SDK Path

```bash
# Export SDK path
export SDK_PATH=~/development/Seeed_Grove_Vision_AI_Module_V2

# Or set permanently in ~/.bashrc
echo 'export SDK_PATH=~/development/Seeed_Grove_Vision_AI_Module_V2' >> ~/.bashrc
source ~/.bashrc
```

### 2.2 Build Training Firmware

```bash
cd ~/development/camera-watcam/tools/training-gui/firmware

# Build firmware
./build_training_firmware.sh
```

**What the script does:**
1. Checks if SDK path exists
2. Copies training firmware files to SDK
3. Builds firmware with `make`
4. Shows binary size and location

**Expected output:**
```
========================================
  Building Training Mode Firmware
========================================

SDK Path: /home/user/development/Seeed_Grove_Vision_AI_Module_V2
App Name: training_mode

Copying firmware files to SDK...
✓ Files copied

Cleaning previous build...
Building firmware...
  CC    training_mode.c
  LD    output.elf

Memory region         Used Size  Region Size  %age Used
           FLASH:       52KB         16 MB      0.32%
            SRAM:       12KB          2 MB      0.59%

✓ Build complete!

Binary size:
   text    data     bss     dec     hex filename
  51234    1024   11520   63778    f922 output.elf

Flash command:
  ./flash_training_firmware.sh /dev/ttyACM0
```

### 2.3 Flash Training Firmware

**Prepare Device:**

1. **Put WE2 in Bootloader Mode:**
   - Press and HOLD the **BOOT** button
   - While holding BOOT, press and release **RESET** button
   - Release **BOOT** button
   - Device is now in bootloader mode (waiting for firmware)

**Flash Firmware:**

```bash
# Flash to device
./flash_training_firmware.sh /dev/ttyACM0
```

**Alternative Manual Flash (if script fails):**

```bash
# Using xmodem (Linux/macOS)
cd ~/development/Seeed_Grove_Vision_AI_Module_V2/EPII_CM55M_APP_S
sx output_training_mode_gnu_epii_CM55M_s/output.bin < /dev/ttyACM0 > /dev/ttyACM0
```

### 2.4 Verify Training Firmware

**Open serial terminal:**

```bash
screen /dev/ttyACM0 921600
```

**Press RESET button on WE2**

**Expected output:**
```
========================================
  WE2 Training Mode Firmware
========================================
  Baudrate: 921600
  Image size: 640x480
========================================

Initializing camera...
✓ Camera initialized
✓ Training mode ready
  Waiting for commands...

READY
```

**Press Ctrl+A then K to exit screen**

✅ **Training Firmware Flashed Successfully!**

---

## Step 3: Launch Training GUI

**Time Required:** 2 minutes
**Difficulty:** Very Easy

### 3.1 Start GUI Application

```bash
cd ~/development/camera-watcam/tools/training-gui

# Activate virtual environment if not already active
source ../../venv/bin/activate  # Linux/macOS
# OR: ..\..\venv\Scripts\activate  # Windows

# Launch GUI with auto-connect
python3 src/training_gui.py --port /dev/ttyACM0
```

**Replace `/dev/ttyACM0` with your actual serial port!**

### 3.2 GUI Window Opens

You should see a window with:

```
┌─────────────────────────────────────────────────────┐
│  Water Meter Digit Training GUI                     │
├─────────────────────────────────────────────────────┤
│  Port: /dev/ttyACM0  [Refresh] [Connect]           │
│  Status: Connected ✓                                 │
├──────────────────────────┬──────────────────────────┤
│                          │  Digit Labeling          │
│   Image Display Area     │  Select Digit:           │
│   (640x480 preview)      │  [0][1][2][3][4]         │
│                          │  [5][6][7][8][9]         │
│                          │                          │
│                          │  Current: 0              │
│  [Capture Frame]         │  [Save Image]            │
│  [Auto Capture]          │                          │
│                          │  Dataset Info            │
│                          │  Digit 0: 0              │
│                          │  Digit 1: 0              │
│                          │  ...                     │
│                          │  Total: 0                │
│                          │                          │
│                          │  [Refresh Stats]         │
│                          │                          │
│                          │  Training                │
│                          │  [Train Model]           │
│                          │  [Test Model]            │
│                          │  [Export TFLite]         │
│                          │  Status: Ready           │
└──────────────────────────┴──────────────────────────┘
│  Total Images: 0 | Model: Not trained              │
└─────────────────────────────────────────────────────┘
```

### 3.3 Verify Connection

- Status should show: **"Connected"** in green
- If not connected, click **"Connect"** button
- If connection fails, check:
  - Serial port is correct
  - Training firmware is running
  - No other program is using the serial port

✅ **GUI is Running and Connected!**

---

## Step 4: Image Acquisition and Labeling

**Time Required:** 1-2 hours (for 1000 images)
**Difficulty:** Easy (but time-consuming)

This is the **most important step**. Good quality, well-labeled images = accurate model!

### 4.1 Position Your Water Meter

**Setup:**

1. Place water meter in front of camera
2. Ensure good, even lighting (avoid shadows, glare)
3. Frame should show all digits clearly
4. Distance: 10-20cm typically (adjust for your camera)
5. Keep meter stable (won't move during capture session)

**Tips for Good Images:**

✅ **DO:**
- Use consistent, bright lighting
- Center the digit in frame
- Keep camera steady
- Use multiple different meters if available
- Vary lighting slightly (but keep it good)
- Include clean meters
- Include slightly dirty meters (realistic conditions)

❌ **DON'T:**
- Use blurry images
- Use images with extreme glare/reflection
- Use images where digit is cut off
- Mix up labels (digit 5 labeled as 2, etc.)
- Use images with bad lighting

### 4.2 Capture First Image

**Step-by-step:**

1. **Position meter to show a specific digit** (let's start with digit "0")

2. **Click "Capture Frame"** button
   - Image appears in left preview area
   - Check if image is good (clear, centered, good lighting)

3. **If image is good:**
   - Make sure **Current Label shows "0"** (since we're capturing digit 0)
   - Click **"Save Image"** button
   - Image is saved to `data/images/0/digit_0_TIMESTAMP.png`
   - Counter increments: "Digit 0: 1"

4. **If image is bad:**
   - Don't save it
   - Adjust lighting/position
   - Click **"Capture Frame"** again

### 4.3 Capture More Images of Digit "0"

**Goal:** 100-200 images per digit

**Methods:**

**Method A: Manual Capture (More Control)**

1. Position meter
2. Click "Capture Frame"
3. Review image
4. Click "Save Image" if good
5. Repeat

**Method B: Auto Capture (Faster)**

1. Click **"Auto Capture"** button
2. GUI automatically captures frame every 1 second
3. Review each captured image
4. Click **"Save Image"** for good images
5. Ignore bad images (they won't be saved)
6. Click **"Auto Capture"** again to stop

**Recommended workflow for digit "0":**

```
1. Capture 20-30 images from one meter (vary angle slightly)
2. Switch to different water meter (if available)
3. Capture 20-30 more images
4. Change lighting slightly (add/remove light)
5. Capture 20-30 more images
6. Include some "realistic" conditions:
   - Slightly dirty meter
   - Different angles
   - Different backgrounds
7. Total: 100-200 images of digit "0"
```

### 4.4 Move to Next Digit

**Switch to digit "1":**

1. **Click button "1"** in "Select Digit" section
2. **"Current" label updates to "1"**
3. Position meter to show digit "1" clearly
4. Repeat capture process (100-200 images)

**Continue for all digits 0-9**

### 4.5 Track Your Progress

**Click "Refresh Stats"** button regularly to see progress:

```
Dataset Info
─────────────
Digit 0: 150
Digit 1: 120
Digit 2: 130
Digit 3: 110
Digit 4: 125
Digit 5: 140
Digit 6: 0    ← Still need to collect
Digit 7: 0
Digit 8: 0
Digit 9: 0

Total: 775
```

### 4.6 Dataset Quality Checklist

Before training, verify:

- ✅ **Balance:** Each digit has similar number of images (100-200 each)
- ✅ **Quality:** Images are clear, well-lit, focused
- ✅ **Variety:** Multiple meters, angles, lighting conditions
- ✅ **Correct Labels:** Digit "5" images are in folder "5", etc.
- ✅ **Total:** 1000+ images total (100+ per digit)

**Recommended Distribution:**

| Digit | Minimum | Recommended | Excellent |
|-------|---------|-------------|-----------|
| 0 | 50 | 100 | 200+ |
| 1 | 50 | 100 | 200+ |
| 2 | 50 | 100 | 200+ |
| 3 | 50 | 100 | 200+ |
| 4 | 50 | 100 | 200+ |
| 5 | 50 | 100 | 200+ |
| 6 | 50 | 100 | 200+ |
| 7 | 50 | 100 | 200+ |
| 8 | 50 | 100 | 200+ |
| 9 | 50 | 100 | 200+ |
| **Total** | **500** | **1000** | **2000+** |

### 4.7 Verify Dataset

**Check dataset folder structure:**

```bash
cd ~/development/camera-watcam/tools/training-gui/data/images

# List all folders and count images
for i in {0..9}; do
    count=$(ls -1 $i/*.png 2>/dev/null | wc -l)
    echo "Digit $i: $count images"
done
```

**Expected output:**
```
Digit 0: 150 images
Digit 1: 145 images
Digit 2: 138 images
Digit 3: 142 images
Digit 4: 151 images
Digit 5: 149 images
Digit 6: 140 images
Digit 7: 155 images
Digit 8: 147 images
Digit 9: 143 images
```

✅ **Dataset Collection Complete!** Ready for training.

**Time spent:** Typically 1-2 hours for 1000+ images

---

## Step 5: Train the Model

**Time Required:** 5-10 minutes
**Difficulty:** Very Easy (GUI does everything)

### 5.1 Verify Dataset

Before training:

1. **Click "Refresh Stats"** in GUI
2. Verify you have **1000+ total images**
3. Check distribution is balanced

**If you have <500 images:**

The GUI will warn you:
```
┌─────────────────────────────────────┐
│  Warning                            │
├─────────────────────────────────────┤
│  Only 487 images in dataset.        │
│  Recommended: 1000+                 │
│                                     │
│  Continue anyway?                   │
│                                     │
│     [Yes]        [No]               │
└─────────────────────────────────────┘
```

- **Recommended:** Collect more images first
- **For testing:** Can proceed with fewer images (but accuracy will be lower)

### 5.2 Start Training

**Click "Train Model" button**

**What happens:**

1. **Loading Dataset:**
   ```
   Status: Loading dataset...
   Progress: [░░░░░░░░░░░░░░░░░░░░] 0%
   ```

2. **Training Progress:**
   ```
   Status: Training model...
   Progress: [████████░░░░░░░░░░░░] 40%

   Epoch 1/10
   25/25 [==============================] - 2s 80ms/step
   loss: 1.8234 - accuracy: 0.3750
   val_loss: 1.2456 - val_accuracy: 0.6250

   Epoch 2/10
   25/25 [==============================] - 2s 75ms/step
   loss: 0.8923 - accuracy: 0.7125
   val_loss: 0.6234 - val_accuracy: 0.8125

   ...

   Epoch 10/10
   25/25 [==============================] - 2s 75ms/step
   loss: 0.0521 - accuracy: 0.9875
   val_loss: 0.1234 - val_accuracy: 0.9625
   ```

3. **Saving Model:**
   ```
   Status: Saving model...
   Progress: [████████████████████] 100%
   ```

4. **Complete:**
   ```
   ┌─────────────────────────────────────┐
   │  Training Complete!                 │
   ├─────────────────────────────────────┤
   │  Model trained successfully!        │
   │                                     │
   │  Validation accuracy: 96.25%        │
   │                                     │
   │  Model saved to:                    │
   │  data/models/digit_model.h5         │
   │                                     │
   │         [OK]                        │
   └─────────────────────────────────────┘
   ```

**Status updates to:** "Training complete! Accuracy: 96.25%"

### 5.3 Understand Training Results

**What the numbers mean:**

| Metric | What it means | Target | Your Result |
|--------|---------------|--------|-------------|
| **Training Accuracy** | How well model fits training data | >95% | 98.75% ✓ |
| **Validation Accuracy** | How well model works on unseen data | >90% | 96.25% ✓ |
| **Loss** | Error rate (lower = better) | <0.2 | 0.0521 ✓ |
| **Val Loss** | Validation error | <0.3 | 0.1234 ✓ |

**Results Interpretation:**

- **Excellent (>95% val accuracy):** Model is production-ready
- **Good (90-95% val accuracy):** Model is usable, might improve with more data
- **Fair (85-90% val accuracy):** Collect more images or improve image quality
- **Poor (<85% val accuracy):** Need more/better images, check labeling

**Common Issues:**

**Issue 1: Low Accuracy (<85%)**

Causes:
- Not enough images
- Poor image quality
- Mislabeled images
- Imbalanced dataset

Solutions:
- Collect more images (aim for 1500-2000)
- Improve lighting
- Review and fix labels
- Balance dataset (equal images per digit)

**Issue 2: Training Accuracy >> Validation Accuracy** (e.g., 99% vs 80%)

Cause: **Overfitting** - Model memorized training data

Solutions:
- Add more variety to dataset
- Collect from different meters
- Vary lighting and angles

**Issue 3: Training Takes Very Long** (>20 minutes)

Causes:
- Large dataset (good!)
- Slow CPU

Solutions:
- Wait (it will complete)
- Use computer with GPU
- Reduce epochs (edit training_gui.py)

### 5.4 Model Files Created

After training, check these files exist:

```bash
cd ~/development/camera-watcam/tools/training-gui/data/models

ls -lh
```

**Expected output:**
```
-rw-rw-r-- 1 user user 524K Nov 22 14:30 digit_model.h5
```

**File description:**
- `digit_model.h5` - Keras model (500KB) - Used for testing in GUI
- Will create `digit_model.tflite` in next step - For deployment (25KB)

✅ **Model Training Complete!**

---

## Step 6: Export Model for Firmware

**Time Required:** 2 minutes
**Difficulty:** Very Easy

Now we need to convert the Keras model to TensorFlow Lite format and then to a C header file for embedding in firmware.

### 6.1 Export TensorFlow Lite Model

**In the GUI:**

1. **Click "Export TFLite" button**

2. **Conversion Process:**
   ```
   Status: Exporting TFLite...

   Converting model to TensorFlow Lite...
   Applying INT8 quantization...
   Saving model...
   ```

3. **Success Message:**
   ```
   ┌─────────────────────────────────────┐
   │  TFLite Export Successful!          │
   ├─────────────────────────────────────┤
   │  Model exported successfully!       │
   │                                     │
   │  Saved to:                          │
   │  data/models/digit_model.tflite     │
   │                                     │
   │  Size: 24,832 bytes                 │
   │                                     │
   │  Ready for firmware integration!    │
   │                                     │
   │         [OK]                        │
   └─────────────────────────────────────┘
   ```

### 6.2 Verify TFLite Model

```bash
cd ~/development/camera-watcam/tools/training-gui/data/models

ls -lh
```

**Expected output:**
```
-rw-rw-r-- 1 user user 524K Nov 22 14:30 digit_model.h5
-rw-rw-r-- 1 user user  25K Nov 22 14:35 digit_model.tflite
```

**Check size:**
- Should be around **25-30 KB** (INT8 quantized)
- Much smaller than .h5 file (524KB)
- Optimized for edge devices

### 6.3 Generate C Header File

**Important:** This converts the TFLite model into a C array that can be compiled into firmware.

```bash
cd ~/development/camera-watcam/tools/training

# Generate header file
python3 generate_model_header.py \
    --input ../training-gui/data/models/digit_model.tflite \
    --output ../../models/water_meter_digit_model.h \
    --array-name digit_recognition_model
```

**Expected output:**
```
========================================
  TFLite to C Header Converter
========================================

Input:  ../training-gui/data/models/digit_model.tflite
Output: ../../models/water_meter_digit_model.h
Array:  digit_recognition_model

Reading TFLite model...
Model size: 24832 bytes

Generating C header...
Writing output file...

✓ Header file generated successfully!

File: ../../models/water_meter_digit_model.h
Size: 24832 bytes
Array: digit_recognition_model[]
Length: digit_recognition_model_len

Ready for firmware integration!
```

### 6.4 Verify C Header File

```bash
cat ~/development/camera-watcam/models/water_meter_digit_model.h | head -30
```

**Expected output:**
```c
/**
 * Auto-generated TensorFlow Lite model
 *
 * Generated: 2024-11-22 14:35:42
 * Model: digit_model.tflite
 * Size: 24832 bytes
 *
 * DO NOT EDIT THIS FILE MANUALLY
 */

#ifndef WATER_METER_DIGIT_MODEL_H
#define WATER_METER_DIGIT_MODEL_H

#ifdef __cplusplus
extern "C" {
#endif

// Model size
const unsigned int digit_recognition_model_len = 24832;

// Model data
const unsigned char digit_recognition_model[] = {
  0x1c, 0x00, 0x00, 0x00, 0x54, 0x46, 0x4c, 0x33,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00,
  0x1c, 0x00, 0x04, 0x00, 0x08, 0x00, 0x0c, 0x00,
  // ... thousands more bytes ...
```

✅ **Model exported and ready for firmware!**

**Files created:**
1. ✅ `data/models/digit_model.h5` - Keras model (for GUI testing)
2. ✅ `data/models/digit_model.tflite` - TFLite model (25KB)
3. ✅ `models/water_meter_digit_model.h` - C header (for firmware)

---

*Continue to Step 7 for firmware integration...*
