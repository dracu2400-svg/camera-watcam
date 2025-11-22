# Training GUI - Quick Start Guide

Get started with digit training in 5 minutes!

---

## 📋 Prerequisites

- HIMAX WE2 board with camera sensor
- Python 3.7+ installed
- USB cable
- HIMAX WE2 SDK (for firmware build)

---

## ⚡ Quick Setup (5 minutes)

### Step 1: Install Python Dependencies (1 minute)

```bash
cd tools/training-gui
pip3 install -r requirements.txt
```

### Step 2: Build & Flash Training Firmware (2 minutes)

```bash
# Set SDK path
export SDK_PATH=/path/to/HIMAX_WE2_SDK

# Build firmware
cd firmware
./build_training_firmware.sh

# Flash to device
./flash_training_firmware.sh /dev/ttyACM0
```

**Note:** Replace `/dev/ttyACM0` with your actual serial port

### Step 3: Launch GUI (30 seconds)

```bash
cd ..
python3 src/training_gui.py --port /dev/ttyACM0
```

### Step 4: Start Collecting Images!

1. Click **Connect** (should auto-connect with `--port` argument)
2. Position camera to point at water meter digit
3. Select digit label (0-9)
4. Click **📷 Capture Frame**
5. Click **💾 Save Image**
6. Repeat for 100+ images per digit

---

## 🎯 Quick Collection Tips

### Fast Collection Mode

1. Enable **🔄 Auto Capture** for automatic capturing every 1 second
2. Just click **💾 Save Image** when digit is in view
3. Switch digit label between saves

### Recommended Collection Strategy

**For first-time training (1-2 hours):**

1. **Start with digit 0**
   - Collect 100 images
   - Vary meter positions
   - Different lighting conditions

2. **Move to digits 1-9**
   - 100 images each
   - Use auto-capture to speed up

3. **Quality check**
   - Click **🔄 Refresh Stats**
   - Ensure ~100 per digit
   - Total: 1000+ images

---

## 🎓 Quick Training (5 minutes)

Once you have **1000+ images**:

1. Click **🎓 Train Model**
2. Wait 5-10 minutes
3. Model saves automatically
4. Check validation accuracy > 95%

---

## 🧪 Quick Testing

Test on a new image:

1. Capture new digit image
2. Click **🧪 Test Model**
3. View prediction results

Should see:
```
Predicted: 5
Confidence: 98.23%
```

---

## 💾 Quick Export

Export for deployment:

1. Click **💾 Export TFLite**
2. Model saved to: `data/models/digit_model.tflite`
3. Generate C header:
   ```bash
   cd ../training
   python3 generate_model_header.py \
       --input ../training-gui/data/models/digit_model.tflite \
       --output ../../models/water_meter_digit_model.h \
       --array-name digit_recognition_model
   ```

---

## 🐛 Quick Troubleshooting

### Can't connect to device?

```bash
# Check port exists
ls /dev/tty*

# Add user to dialout group (Linux)
sudo usermod -a -G dialout $USER
# Log out and back in

# Or set permissions (temporary)
sudo chmod 666 /dev/ttyACM0
```

### Firmware not responding?

1. Press RESET button on WE2
2. Check serial port in GUI
3. Try reflashing firmware

### Training accuracy low?

- Need more images (1000+ recommended)
- Balance dataset (similar count per digit)
- Improve lighting/focus
- Remove mislabeled images

---

## 📊 Quick Reference

### Minimum Dataset Requirements

| Quality Level | Images/Digit | Total | Expected Accuracy |
|---------------|--------------|-------|-------------------|
| Minimum | 50 | 500 | 85-90% |
| Recommended | 100 | 1000 | 95-97% |
| Excellent | 200+ | 2000+ | 98-99% |

### Training Time

| Dataset Size | Training Time | Hardware |
|--------------|---------------|----------|
| 500 images | 2-3 minutes | CPU |
| 1000 images | 5-8 minutes | CPU |
| 2000 images | 10-15 minutes | CPU |
| 1000 images | 1-2 minutes | GPU |

### Model Size

| Format | Size | Inference Time |
|--------|------|----------------|
| H5 (Keras) | ~500 KB | N/A |
| TFLite | ~100 KB | N/A |
| TFLite INT8 | ~25 KB | 10-30 ms (WE2) |

---

## 🚀 Next Steps

After successful training:

1. **Test thoroughly**
   - Different meters
   - Various lighting
   - Edge cases

2. **Deploy to production**
   - Copy model header to firmware
   - Build production firmware
   - Flash to devices

3. **Monitor performance**
   - Track accuracy in field
   - Collect failure cases
   - Retrain as needed

---

## 📚 Full Documentation

For detailed information, see:

- **[Full README](README.md)** - Complete documentation
- **[Firmware Guide](../../../docs/03-development/FIRMWARE_DEVELOPMENT_COMPLETE_GUIDE.md)** - Firmware integration
- **[Production Pipeline](../../../docs/04-deployment/PRODUCTION_PIPELINE.md)** - Deployment guide

---

## ⌨️ Keyboard Shortcuts (Future)

*Coming soon:*
- `Space` - Capture frame
- `S` - Save image
- `0-9` - Select digit
- `A` - Toggle auto-capture

---

## 🎬 Video Tutorial

*Coming soon: Video walkthrough of training process*

---

**Happy Training! 🎉**

If you run into issues, check the [Troubleshooting](README.md#troubleshooting) section in the full README.
