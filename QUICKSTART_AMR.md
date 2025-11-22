# AMR Solution - Quick Start Guide

This guide will get you started building your Water Meter Automatic Reading system using HIMAX WE2 + STM32WL LoRaWAN in 30 minutes.

## 📦 What You Need

### Hardware:
- ✅ HIMAX WE2 (Grove Vision AI Module V2)
- ✅ STM32WL Module with LoRaWAN
- ✅ USB cable
- ✅ Jumper wires (for UART connection)

### Software:
- ✅ Linux/macOS (Ubuntu 20.04+ recommended)
- ✅ Python 3.7+
- ✅ ARM GNU Toolchain 13.2

---

## 🚀 Quick Start (5 Steps)

### Step 1: Setup Development Environment (5 min)

```bash
# Install ARM GCC Toolchain
cd ~
wget https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz
tar -xvf arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz
export PATH="$HOME/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-eabi/bin/:$PATH"

# Install Python dependencies
cd camera-watcam
pip install -r requirements_amr.txt
```

### Step 2: Prepare Dataset (Optional - Use MNIST for Quick Test)

```bash
# For quick testing, you can skip custom dataset and use MNIST
# The system will work with any digit recognition dataset

# For production, collect 200+ images per digit (0-9) of your water meters
# Place them in: dataset/water_meter/raw/train/{0..9}/
```

### Step 3: Train Model (10 min with MNIST, longer with custom data)

```bash
cd training_scripts

# Prepare dataset
python3 prepare_dataset.py

# Train model
python3 train_digit_model.py

# Convert to TFLite
python3 convert_to_tflite.py

# Generate C header
python3 generate_model_header.py

cd ..
```

### Step 4: Build Firmware (5 min)

```bash
# Build AMR firmware
./build_amr.sh
```

### Step 5: Flash and Test (5 min)

```bash
# Flash firmware (replace /dev/ttyACM0 with your port)
./flash_amr.sh /dev/ttyACM0 models/water_meter_digit_model.tflite

# Press RESET button on device

# Provision device with LoRaWAN credentials
python3 tools/provision_device.py /dev/ttyACM0 \
  --device-id 12345678 \
  --deveui 0123456789ABCDEF \
  --appeui FEDCBA9876543210 \
  --appkey 2B7E151628AED2A6ABF7158809CF4F3C

# Test reading
python3 tools/provision_device.py /dev/ttyACM0 --test-only
```

---

## 📊 Monitor Your Device

```bash
# Real-time monitoring
screen /dev/ttyACM0 921600

# Or use minicom
minicom -D /dev/ttyACM0 -b 921600

# Field testing (logs to CSV)
python3 tools/field_test.py /dev/ttyACM0 --duration 60
```

---

## 🔧 Hardware Connection

```
HIMAX WE2          STM32WL
---------          --------
UART_TX    ───►    UART_RX
UART_RX    ◄───    UART_TX
GND        ───     GND
3.3V       ───     3.3V (if needed)
```

---

## 📁 Project Structure

```
camera-watcam/
├── AMR_SOLUTION_GUIDE.md          ← Complete documentation
├── QUICKSTART_AMR.md              ← This file
├── build_amr.sh                   ← Build script
├── flash_amr.sh                   ← Flash script
├── training_scripts/              ← Model training
│   ├── prepare_dataset.py
│   ├── train_digit_model.py
│   ├── convert_to_tflite.py
│   └── generate_model_header.py
├── tools/                         ← Device tools
│   ├── provision_device.py        ← Provisioning tool
│   └── field_test.py              ← Testing tool
├── models/                        ← Trained models
├── dataset/                       ← Training data
└── EPII_CM55M_APP_S/             ← WE2 SDK
```

---

## ⚡ Quick Commands Reference

```bash
# Build firmware
./build_amr.sh

# Flash firmware only
./flash_amr.sh /dev/ttyACM0

# Flash firmware + model
./flash_amr.sh /dev/ttyACM0 models/water_meter_digit_model.tflite

# Provision device
python3 tools/provision_device.py /dev/ttyACM0 \
  --device-id YOUR_ID \
  --deveui YOUR_DEVEUI \
  --appeui YOUR_APPEUI \
  --appkey YOUR_APPKEY

# Test reading
python3 tools/provision_device.py /dev/ttyACM0 --test-only

# Get device status
python3 tools/provision_device.py /dev/ttyACM0 --status

# Field test (60 minutes)
python3 tools/field_test.py /dev/ttyACM0 --duration 60
```

---

## 🐛 Troubleshooting

### Build fails?
```bash
# Check toolchain
arm-none-eabi-gcc --version

# Clean and rebuild
cd EPII_CM55M_APP_S && make clean && cd .. && ./build_amr.sh
```

### Flash fails?
```bash
# Check permissions (Linux)
sudo chmod 666 /dev/ttyACM0

# Or add user to dialout group
sudo usermod -a -G dialout $USER
# Then logout and login again

# Verify port
ls -la /dev/ttyACM*
```

### Model not loading?
```bash
# Regenerate model with correct format
cd training_scripts
python3 convert_to_tflite.py
python3 generate_model_header.py

# Copy to project
cp ../models/water_meter_digit_model.h \
   ../EPII_CM55M_APP_S/app/scenario_app/water_meter_amr/
```

---

## 📚 Next Steps

1. ✅ **Read full guide**: See [AMR_SOLUTION_GUIDE.md](AMR_SOLUTION_GUIDE.md)
2. ✅ **Collect real data**: Take photos of your water meters
3. ✅ **Retrain model**: Use your specific meter images
4. ✅ **Setup LoRaWAN**: Configure The Things Network or your preferred provider
5. ✅ **Deploy**: Install devices in the field
6. ✅ **Monitor**: Use cloud platform to track readings

---

## 🆘 Need Help?

- 📖 **Full Documentation**: [AMR_SOLUTION_GUIDE.md](AMR_SOLUTION_GUIDE.md)
- 🔧 **SDK Documentation**: [README.md](README.md)
- 💬 **Issues**: Check FAQ.md or open an issue

---

## ✅ Expected Output

When everything works, you should see:

```
========================================
  Water Meter AMR System
  HIMAX WE2 + STM32WL LoRaWAN
========================================
Model initialized successfully
STM32WL UART initialized
Water Meter AMR System initialized successfully

--- Taking Meter Reading ---
Capturing and recognizing digits...
Digit 0: 1 (conf: 0.95)
Digit 1: 2 (conf: 0.96)
Digit 2: 3 (conf: 0.94)
Digit 3: 4 (conf: 0.97)
Digit 4: 5 (conf: 0.95)
Digit 5: 6 (conf: 0.96)
Digit 6: 7 (conf: 0.93)
Digit 7: 8 (conf: 0.98)
Sent to LoRaWAN: READ:12345678,95
```

Good luck with your AMR project! 🚀
