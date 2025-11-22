# Water Meter Digit Training GUI

Complete training system for collecting digit images and training recognition models for the Water Meter AMR project.

---

## 🎯 Overview

This training GUI provides an end-to-end workflow for:
1. **Capturing images** from WE2 camera via UART
2. **Labeling digits** (0-9) interactively
3. **Building dataset** automatically organized by digit
4. **Training models** with real-time progress
5. **Testing models** on live images
6. **Exporting TFLite** models for deployment

---

## 📋 Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Detailed Usage](#detailed-usage)
- [Training Workflow](#training-workflow)
- [Troubleshooting](#troubleshooting)

---

## ✨ Features

### GUI Features
- ✅ **Live image capture** from WE2 device
- ✅ **Interactive labeling** with digit buttons (0-9)
- ✅ **Auto-capture mode** for rapid dataset building
- ✅ **Dataset statistics** showing per-digit image counts
- ✅ **Real-time training** with progress bar
- ✅ **Model testing** on captured images
- ✅ **TFLite export** with INT8 quantization

### Firmware Features
- ✅ **Simple UART protocol** for easy integration
- ✅ **High-speed transfer** at 921600 baud
- ✅ **Streaming mode** for continuous capture
- ✅ **Status monitoring** for debugging
- ✅ **Minimal footprint** for fast flashing

---

## 📦 Requirements

### Hardware
- HIMAX WE2 development board
- Camera sensor (HM0360, OV5647, IMX219, or IMX477)
- USB cable for UART connection
- PC with available USB port

### Software
- **Python 3.7+**
- **TensorFlow 2.x**
- **OpenCV (cv2)**
- **NumPy**
- **Pillow (PIL)**
- **pyserial**
- **tkinter** (usually included with Python)

### Firmware Build Tools
- ARM GNU Toolchain 13.2+
- HIMAX WE2 SDK

---

## 🔧 Installation

### 1. Install Python Dependencies

```bash
cd tools/training-gui

# Install required packages
pip3 install -r requirements.txt
```

**requirements.txt:**
```
tensorflow>=2.10.0
opencv-python>=4.6.0
numpy>=1.23.0
Pillow>=9.2.0
pyserial>=3.5
```

### 2. Build Training Firmware

```bash
# Copy firmware to SDK
cd /path/to/HIMAX_WE2_SDK
cp -r /path/to/camera-watcam/tools/training-gui/firmware \
      EPII_CM55M_APP_S/app/scenario_app/training_mode

# Build firmware
cd EPII_CM55M_APP_S
make clean APP=training_mode
make APP=training_mode -j$(nproc)
```

### 3. Flash Training Firmware

```bash
# Connect WE2 via USB
# Identify port (e.g., /dev/ttyACM0 on Linux, COM3 on Windows)

# Flash firmware
./flash_training.sh /dev/ttyACM0
```

---

## 🚀 Quick Start

### Step 1: Connect Hardware

1. Connect camera sensor to WE2 board
2. Flash training firmware to WE2
3. Connect WE2 to PC via USB
4. Note the serial port (e.g., `/dev/ttyACM0`)

### Step 2: Start GUI

```bash
cd tools/training-gui

# Launch GUI
python3 src/training_gui.py

# Or with auto-connect:
python3 src/training_gui.py --port /dev/ttyACM0
```

### Step 3: Collect Images

1. Click "Connect" to connect to WE2
2. Select digit label (0-9)
3. Click "📷 Capture Frame"
4. Review captured image
5. Click "💾 Save Image" to add to dataset
6. Repeat for all digits

**Tip:** Use "🔄 Auto Capture" for rapid dataset building!

### Step 4: Train Model

1. Collect at least 100 images per digit (1000+ recommended)
2. Click "🔄 Refresh Stats" to verify dataset
3. Click "🎓 Train Model"
4. Wait for training to complete
5. Model is saved automatically

### Step 5: Test Model

1. Capture a new image
2. Click "🧪 Test Model"
3. View prediction results

### Step 6: Export for Deployment

1. Click "💾 Export TFLite"
2. Model is saved as `data/models/digit_model.tflite`
3. Use with `tools/training/generate_model_header.py` to create C header

---

## 📖 Detailed Usage

### GUI Layout

```
┌─────────────────────────────────────────────────┐
│  [Port: /dev/ttyACM0] [Refresh] [Connect]       │
│  Status: Connected                               │
├──────────────────────────┬──────────────────────┤
│                          │  Digit Labeling      │
│   Image Display          │  [0][1][2][3][4]     │
│   (Live camera preview)  │  [5][6][7][8][9]     │
│                          │  Current: 3          │
│                          │  [💾 Save Image]     │
│                          ├──────────────────────┤
│  [📷 Capture]            │  Dataset Info        │
│  [🔄 Auto Capture]       │  Digit 0: 120        │
│                          │  Digit 1: 115        │
│                          │  ...                 │
│                          │  Total: 1250         │
│                          ├──────────────────────┤
│                          │  Training            │
│                          │  [🎓 Train Model]    │
│                          │  [🧪 Test Model]     │
│                          │  [💾 Export TFLite]  │
│                          │  Status: Ready       │
│                          │  [Progress Bar]      │
└──────────────────────────┴──────────────────────┘
│  Total Images: 1250 | Model: Trained           │
└──────────────────────────────────────────────────┘
```

### UART Protocol

The training firmware uses a simple text-based protocol:

| Command | Description | Response |
|---------|-------------|----------|
| `CAPTURE` | Capture and send one frame | `OK\r\n` + image data |
| `STREAM` | Start continuous streaming | `OK\r\n` |
| `STOP` | Stop streaming | `OK\r\n` |
| `STATUS` | Get device status | `STATUS: frames=123 ...` |

**Image Data Format:**
- Size: 640x480 = 307200 bytes
- Format: Grayscale (1 byte per pixel)
- Sent immediately after `OK` response

### Dataset Structure

Images are automatically organized by digit:

```
tools/training-gui/data/images/
├── 0/
│   ├── digit_0_20241122_143022_123456.png
│   ├── digit_0_20241122_143023_234567.png
│   └── ...
├── 1/
│   ├── digit_1_20241122_143024_345678.png
│   └── ...
├── 2/
│   └── ...
...
└── 9/
    └── ...
```

Each image is:
- **28x28 pixels** (resized from 640x480)
- **Grayscale** (PNG format)
- **Named** with digit, timestamp, and microseconds

---

## 🔄 Training Workflow

### Recommended Workflow

#### Phase 1: Dataset Collection (1-2 hours)

1. **Prepare water meters**
   - Collect 5-10 different water meter types
   - Ensure good lighting conditions
   - Position camera at correct distance

2. **Collect images systematically**
   - 100-200 images per digit minimum
   - Vary lighting conditions
   - Include different meter styles
   - Capture different angles

3. **Quality check**
   - Review dataset statistics
   - Ensure balanced distribution
   - Remove bad images if needed

**Best Practices:**
- ✅ Collect from multiple meter types
- ✅ Vary lighting (bright, dim, shadows)
- ✅ Include edge cases (dirty meters, reflections)
- ✅ Balance dataset (similar count per digit)
- ❌ Don't use blurry images
- ❌ Don't mislabel digits

#### Phase 2: Model Training (5-10 minutes)

1. **Verify dataset**
   - Click "🔄 Refresh Stats"
   - Check total images (1000+ recommended)
   - Verify all digits have data

2. **Start training**
   - Click "🎓 Train Model"
   - Wait for completion (5-10 mins)
   - Note validation accuracy

3. **Evaluate results**
   - Accuracy > 95%: Good
   - Accuracy 90-95%: Acceptable
   - Accuracy < 90%: Need more data

#### Phase 3: Testing & Validation (30 minutes)

1. **Test on live images**
   - Capture new images
   - Click "🧪 Test Model"
   - Verify predictions

2. **Field testing**
   - Test on actual meters
   - Check different lighting
   - Verify accuracy in real conditions

3. **Iterate if needed**
   - Add more images where model fails
   - Retrain model
   - Re-test

#### Phase 4: Deployment

1. **Export TFLite**
   - Click "💾 Export TFLite"
   - Model saved as INT8 quantized

2. **Generate C header**
   ```bash
   cd tools/training
   python3 generate_model_header.py \
       --input ../training-gui/data/models/digit_model.tflite \
       --output ../../models/water_meter_digit_model.h \
       --array-name digit_recognition_model
   ```

3. **Integrate into firmware**
   - Copy header to firmware project
   - Build and flash production firmware
   - Test on device

---

## 🐛 Troubleshooting

### Connection Issues

**Problem:** "Failed to connect to /dev/ttyACM0"

**Solutions:**
1. Check USB cable is connected
2. Verify port with `ls /dev/tty*`
3. Add user to dialout group:
   ```bash
   sudo usermod -a -G dialout $USER
   # Log out and back in
   ```
4. Check device permissions:
   ```bash
   sudo chmod 666 /dev/ttyACM0
   ```

### Capture Issues

**Problem:** "Failed to capture frame"

**Solutions:**
1. Verify firmware is running:
   - Open serial terminal:
     ```bash
     screen /dev/ttyACM0 921600
     ```
   - Should see "READY" on boot
   - Send `STATUS` command
2. Check camera connection
3. Reflash training firmware
4. Try lower baud rate (115200)

### Training Issues

**Problem:** "Training accuracy is low"

**Solutions:**
1. Collect more images (1000+ total)
2. Balance dataset (similar count per digit)
3. Improve image quality (lighting, focus)
4. Remove mislabeled images
5. Add more variety (different meters, angles)

**Problem:** "Training is slow"

**Solutions:**
1. Reduce batch size (edit `training_gui.py`)
2. Use GPU if available
3. Reduce number of epochs
4. Use smaller model architecture

### Export Issues

**Problem:** "TFLite export failed"

**Solutions:**
1. Ensure TensorFlow 2.x is installed
2. Check model was trained successfully
3. Verify sufficient disk space
4. Try without quantization first

---

## 📊 Model Performance

### Expected Performance

With a good dataset (1000+ images):

| Metric | Value |
|--------|-------|
| **Training Accuracy** | 98-99% |
| **Validation Accuracy** | 95-97% |
| **Inference Time (WE2)** | 10-30 ms |
| **Model Size (TFLite INT8)** | ~25 KB |
| **RAM Usage** | ~100 KB |

### Model Architecture

```
Input (28x28x1)
    ↓
Conv2D (32 filters, 3x3) → ReLU
    ↓
MaxPooling2D (2x2)
    ↓
Conv2D (64 filters, 3x3) → ReLU
    ↓
MaxPooling2D (2x2)
    ↓
Conv2D (64 filters, 3x3) → ReLU
    ↓
Flatten
    ↓
Dense (64) → ReLU → Dropout(0.5)
    ↓
Dense (10) → Softmax
    ↓
Output (10 classes)
```

**Total Parameters:** ~130,000
**TFLite Size (INT8):** ~25 KB

---

## 🎓 Tips & Best Practices

### Dataset Collection

1. **Lighting variation** is crucial
   - Collect in bright, dim, and mixed lighting
   - Include images with shadows
   - Try different times of day

2. **Meter variety** improves generalization
   - Different manufacturers
   - Different digit styles
   - Various backgrounds

3. **Quality over quantity**
   - 1000 good images > 5000 poor images
   - Remove blurry/mislabeled images
   - Ensure digits are centered

### Training

1. **Monitor validation accuracy**
   - Stop if overfitting (train >> val accuracy)
   - Add more data if underfitting

2. **Save checkpoints**
   - GUI auto-saves after training
   - Keep multiple versions for comparison

3. **Test on unseen data**
   - Don't test on training images
   - Use different meters for testing

### Deployment

1. **Test on device first**
   - Verify inference speed
   - Check accuracy on device
   - Monitor memory usage

2. **Field testing is essential**
   - Test in actual deployment conditions
   - Verify with different meters
   - Check battery impact

---

## 📝 Advanced Usage

### Custom Model Architecture

Edit `training_gui.py` and modify `build_model()`:

```python
def build_model(self):
    model = keras.Sequential([
        # Your custom architecture here
        layers.Conv2D(16, (3, 3), activation='relu', input_shape=(28, 28, 1)),
        layers.MaxPooling2D((2, 2)),
        # ...
        layers.Dense(10, activation='softmax')
    ])
    return model
```

### Data Augmentation

Add augmentation for better generalization:

```python
from tensorflow.keras.preprocessing.image import ImageDataGenerator

datagen = ImageDataGenerator(
    rotation_range=10,
    width_shift_range=0.1,
    height_shift_range=0.1,
    zoom_range=0.1
)
```

### Transfer Learning

Use a pre-trained model as base:

```python
base_model = keras.applications.MobileNetV2(
    input_shape=(28, 28, 1),
    include_top=False,
    weights=None  # Or use 'imagenet' with adaptation
)
```

---

## 🔗 See Also

- [Main AMR Documentation](../../docs/README.md)
- [Firmware Development Guide](../../docs/03-development/FIRMWARE_DEVELOPMENT_COMPLETE_GUIDE.md)
- [Production Pipeline](../../docs/04-deployment/PRODUCTION_PIPELINE.md)
- [Training Scripts](../training/)

---

## 📞 Support

For issues or questions:
1. Check [Troubleshooting](#troubleshooting) section
2. Review logs in terminal
3. Consult firmware development guide
4. Open issue in project repository

---

**Version:** 1.0.0
**Last Updated:** 2024-11-22
**Tested with:** Python 3.10, TensorFlow 2.15, HIMAX WE2 SDK 1.5
