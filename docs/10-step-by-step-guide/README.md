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

You've successfully completed all 10 steps:

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

- Check [Troubleshooting](#troubleshooting) section above
- Review main project documentation
- Contact support

**Congratulations! You now have a complete working Water Meter AMR system!**
