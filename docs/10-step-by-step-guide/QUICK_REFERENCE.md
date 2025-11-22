# Quick Reference Guide

**Fast lookup for common commands and workflows**

---

## Environment Setup Commands

```bash
# Set SDK path
export SDK_PATH=~/development/Seeed_Grove_Vision_AI_Module_V2

# Activate Python virtual environment
source venv/bin/activate  # Linux/Mac
venv\Scripts\activate     # Windows

# Check serial port
ls /dev/tty* | grep -E "ACM|USB"  # Linux
ls /dev/tty.usb*                   # Mac
mode                               # Windows (in CMD)
```

---

## Training Firmware Commands

```bash
# Build training firmware
cd tools/training-gui/firmware
./build_training_firmware.sh

# Flash training firmware
./flash_training_firmware.sh /dev/ttyACM0

# Verify training firmware
screen /dev/ttyACM0 921600
# Expected: "WE2 Training Mode Firmware" message
```

---

## Training GUI Commands

```bash
# Launch GUI
cd tools/training-gui
python3 src/training_gui.py --port /dev/ttyACM0

# Without auto-connect
python3 src/training_gui.py
```

---

## Model Training Commands

```bash
# In GUI:
1. Collect images (1000+ recommended)
2. Click "Train Model"
3. Wait 5-10 minutes
4. Click "Export TFLite"

# Command line model export (alternative)
cd tools/training
python3 convert_to_tflite.py \
    --input ../training-gui/data/models/digit_model.h5 \
    --output ../training-gui/data/models/digit_model.tflite \
    --quantize int8

# Generate C header
python3 generate_model_header.py \
    --input ../training-gui/data/models/digit_model.tflite \
    --output ../../models/water_meter_digit_model.h \
    --array-name digit_recognition_model
```

---

## Production Firmware Commands

```bash
# Copy model to firmware
cp models/water_meter_digit_model.h \
   $SDK_PATH/EPII_CM55M_APP_S/app/scenario_app/water_meter_amr/models/

# Build firmware
cd $SDK_PATH/EPII_CM55M_APP_S
make clean APP=water_meter_amr
make APP=water_meter_amr -j$(nproc)

# Flash firmware
cd ~/camera-watcam/scripts/flash
./flash_amr.sh /dev/ttyACM0

# Monitor device
screen /dev/ttyACM0 921600
```

---

## Bootloader Mode

**Put WE2 in bootloader mode:**

1. Press and HOLD **BOOT** button
2. While holding BOOT, press and release **RESET**
3. Release **BOOT**
4. Device ready for flashing

---

## Common File Locations

```
Training data:
  tools/training-gui/data/images/0-9/

Trained models:
  tools/training-gui/data/models/digit_model.h5      (Keras)
  tools/training-gui/data/models/digit_model.tflite (TFLite)
  models/water_meter_digit_model.h                  (C header)

Firmware source:
  $SDK_PATH/EPII_CM55M_APP_S/app/scenario_app/water_meter_amr/

Compiled firmware:
  $SDK_PATH/EPII_CM55M_APP_S/output_water_meter_amr_gnu_epii_CM55M_s/output.bin
```

---

## Verification Commands

```bash
# Verify ARM toolchain
arm-none-eabi-gcc --version

# Verify Python packages
pip list | grep -E "tensorflow|opencv|numpy"

# Count training images
cd tools/training-gui/data/images
for i in {0..9}; do
    count=$(ls -1 $i/*.png 2>/dev/null | wc -l)
    echo "Digit $i: $count"
done

# Check firmware size
arm-none-eabi-size output.elf

# Find model in firmware
arm-none-eabi-nm output.elf | grep digit_recognition_model
```

---

## Memory Analysis

```bash
# Check build output for memory usage
make APP=water_meter_amr 2>&1 | grep "Memory region"

# Show largest symbols
arm-none-eabi-nm --size-sort --print-size output.elf | tail -20

# Generate memory map
arm-none-eabi-objdump -h output.elf
```

---

## Serial Terminal Commands

```bash
# Linux - screen
screen /dev/ttyACM0 921600
# Exit: Ctrl+A then K

# Linux - minicom
minicom -D /dev/ttyACM0 -b 921600
# Exit: Ctrl+A then X

# Mac - screen (built-in)
screen /dev/tty.usbmodem* 921600

# Windows - PuTTY
putty -serial COM3 -sercfg 921600,8,n,1,N
```

---

## Debugging Commands

```bash
# Check serial port permissions (Linux)
ls -l /dev/ttyACM0
# If not accessible:
sudo chmod 666 /dev/ttyACM0
# Or add user to dialout group:
sudo usermod -a -G dialout $USER

# Check if port is in use
lsof | grep ttyACM0

# Kill process using port
sudo fuser -k /dev/ttyACM0

# Check Python virtual environment
which python3
python3 --version
pip list

# Check disk space
df -h

# Check available memory
free -h
```

---

## Dataset Quality Checks

```bash
# Check for corrupted images
cd tools/training-gui/data/images
find . -name "*.png" -exec file {} \; | grep -v "PNG image"

# Check image sizes
find . -name "*.png" -exec identify {} \; | awk '{print $3}' | sort | uniq -c

# Recommended: all should be 28x28
# find . -name "*.png" -exec identify {} \; | grep -v "28x28"

# Count total images
find . -name "*.png" | wc -l

# Check dataset balance
for i in {0..9}; do
    count=$(find $i -name "*.png" 2>/dev/null | wc -l)
    printf "Digit %d: %4d images\n" $i $count
done
```

---

## Model Performance Checks

```python
# Test model accuracy (Python)
import tensorflow as tf
import numpy as np

# Load model
model = tf.keras.models.load_model('tools/training-gui/data/models/digit_model.h5')

# Evaluate on test set
# (Assuming you have X_test, y_test prepared)
loss, accuracy = model.evaluate(X_test, y_test)
print(f"Test accuracy: {accuracy*100:.2f}%")

# Check model size
import os
model_size = os.path.getsize('tools/training-gui/data/models/digit_model.tflite')
print(f"TFLite model size: {model_size/1024:.1f} KB")
```

---

## Network Connectivity Checks

```bash
# Check LoRaWAN gateway availability
# (If using TTN)
curl -s https://www.thethingsnetwork.org/

# Ping network server
ping your-network-server.com

# Check firewall rules (if needed)
sudo ufw status

# Check internet connectivity
ping -c 3 8.8.8.8
```

---

## Battery Life Estimation

```python
# Calculate battery life
battery_capacity_mah = 3600  # mAh
battery_voltage = 3.6        # V

# Energy capacity
energy_mwh = battery_capacity_mah * battery_voltage  # = 12,960 mWh

# Daily consumption (from power budget)
daily_consumption_mwh = 10.6  # mWh/day (with hourly readings)

# Battery life
battery_life_days = energy_mwh / daily_consumption_mwh
battery_life_years = battery_life_days / 365

print(f"Battery life: {battery_life_days:.0f} days ({battery_life_years:.1f} years)")
```

---

## Typical Metrics

### Good Model Performance

| Metric | Target | Excellent |
|--------|--------|-----------|
| Training Accuracy | >95% | >98% |
| Validation Accuracy | >90% | >95% |
| Test Accuracy | >90% | >93% |
| Inference Time | <30ms | <20ms |
| Model Size | <30KB | <25KB |
| Confidence | >90% | >95% |

### Good System Performance

| Metric | Target | Excellent |
|--------|--------|-----------|
| Join Success Rate | >90% | >95% |
| Uplink Success Rate | >95% | >99% |
| Battery Life | >12 months | >18 months |
| Reading Accuracy | >95% | >98% |
| Device Uptime | >99% | >99.5% |

---

## File Size Limits

```
Training image: ~5-10 KB (28x28 PNG)
Dataset: ~10-50 MB (1000-5000 images)
Keras model (.h5): 500-1000 KB
TFLite model: 25-50 KB
C header file: 25-50 KB
Firmware binary: 300-500 KB
Total Flash usage: <1 MB (plenty of room in 16MB)
Total RAM usage: <200 KB (plenty of room in 2MB)
```

---

## Useful One-Liners

```bash
# Quick check if device is connected
ls /dev/ttyACM* && echo "Device found!" || echo "No device found"

# Count images by digit
seq 0 9 | xargs -I{} sh -c 'echo "Digit {}: $(ls tools/training-gui/data/images/{}/*.png 2>/dev/null | wc -l)"'

# Find largest files
du -sh * | sort -rh | head -10

# Monitor device power
# (Requires multimeter)
# Measure current on power line, multiply by voltage

# Quick firmware flash check
cat output.bin > /dev/ttyACM0 && echo "Flash started"

# Watch build process
watch -n 1 'ls -lh output.elf 2>/dev/null || echo "Building..."'
```

---

## Environment Variables

```bash
# Add to ~/.bashrc for permanent setup
export SDK_PATH=~/development/Seeed_Grove_Vision_AI_Module_V2
export AMR_PROJECT=~/development/camera-watcam
export SERIAL_PORT=/dev/ttyACM0
export PATH="$PATH:/opt/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi/bin"

# Then use like:
cd $SDK_PATH
cd $AMR_PROJECT
screen $SERIAL_PORT 921600
```

---

## Aliases (Optional)

```bash
# Add to ~/.bashrc
alias amr-gui='cd $AMR_PROJECT/tools/training-gui && source ../../venv/bin/activate && python3 src/training_gui.py'
alias amr-build='cd $SDK_PATH/EPII_CM55M_APP_S && make APP=water_meter_amr -j$(nproc)'
alias amr-flash='cd $AMR_PROJECT/scripts/flash && ./flash_amr.sh $SERIAL_PORT'
alias amr-monitor='screen $SERIAL_PORT 921600'
alias amr-bootloader='echo "1. Hold BOOT, 2. Press RESET, 3. Release BOOT"'

# Then use:
# amr-gui          # Launch training GUI
# amr-build        # Build firmware
# amr-flash        # Flash firmware
# amr-monitor      # Monitor device
```

---

## Support Resources

- **Main Documentation:** `docs/`
- **Training GUI Docs:** `tools/training-gui/README.md`
- **Firmware Guide:** `docs/03-development/FIRMWARE_DEVELOPMENT_COMPLETE_GUIDE.md`
- **Production Pipeline:** `docs/04-deployment/PRODUCTION_PIPELINE.md`
- **This Guide:** `docs/10-step-by-step-guide/README.md`

---

**Last Updated:** 2024-11-22
**Version:** 1.0.0
