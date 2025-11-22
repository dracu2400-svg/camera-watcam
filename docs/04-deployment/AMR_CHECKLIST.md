# AMR Project Implementation Checklist

Use this checklist to track your progress building the complete Water Meter AMR solution.

---

## Phase 1: Environment Setup

- [ ] **Install ARM GNU Toolchain**
  - [ ] Download ARM GCC 13.2
  - [ ] Extract and add to PATH
  - [ ] Verify: `arm-none-eabi-gcc --version`

- [ ] **Install Python Dependencies**
  - [ ] Install Python 3.7+
  - [ ] Run: `pip install -r requirements_amr.txt`
  - [ ] Verify: `python3 -c "import tensorflow; print(tensorflow.__version__)"`

- [ ] **Install Flash Tools**
  - [ ] Install xmodem dependencies: `pip install -r xmodem/requirements.txt`
  - [ ] (Linux) Install minicom: `sudo apt install minicom`
  - [ ] (Linux) Set serial port permissions

- [ ] **Clone and Setup Repository**
  - [ ] Repository cloned
  - [ ] All scripts are executable: `chmod +x *.sh training_scripts/*.py tools/*.py`

---

## Phase 2: Dataset Collection

- [ ] **Collect Water Meter Images**
  - [ ] Create directory structure: `mkdir -p dataset/water_meter/raw/{train,val,test}/{0..9}`
  - [ ] Collect 200+ images per digit (0-9) for training
  - [ ] Collect 50+ images per digit for validation
  - [ ] Collect 50+ images per digit for testing
  - [ ] Ensure variety: different lighting, angles, meter types

- [ ] **Organize Dataset**
  - [ ] Training images in: `dataset/water_meter/raw/train/{0..9}/`
  - [ ] Validation images in: `dataset/water_meter/raw/val/{0..9}/`
  - [ ] Test images in: `dataset/water_meter/raw/test/{0..9}/`
  - [ ] All images are .jpg or .png format

**Note**: For quick testing, you can skip this and use synthetic/MNIST data

---

## Phase 3: Model Training

- [ ] **Prepare Dataset**
  - [ ] Run: `python3 training_scripts/prepare_dataset.py`
  - [ ] Verify processed images in: `dataset/water_meter/{train,val,test}/`
  - [ ] Check augmentation applied correctly

- [ ] **Train Model**
  - [ ] Run: `python3 training_scripts/train_digit_model.py`
  - [ ] Monitor training progress
  - [ ] Check test accuracy > 95%
  - [ ] Verify model saved: `models/best_digit_model.h5`
  - [ ] Review training plot: `models/training_history.png`

- [ ] **Convert to TFLite**
  - [ ] Run: `python3 training_scripts/convert_to_tflite.py`
  - [ ] Verify INT8 quantization applied
  - [ ] Check model size < 100KB
  - [ ] Test conversion successful: `models/water_meter_digit_model.tflite`

- [ ] **Generate C Header**
  - [ ] Run: `python3 training_scripts/generate_model_header.py`
  - [ ] Verify header created: `models/water_meter_digit_model.h`
  - [ ] Check array size matches TFLite model

---

## Phase 4: Firmware Development

- [ ] **Create Application Structure**
  - [ ] Directory: `EPII_CM55M_APP_S/app/scenario_app/water_meter_amr/`
  - [ ] Copy model header to app directory
  - [ ] Create/verify all source files (.c, .h)
  - [ ] Create/verify makefile (.mk)

- [ ] **Configure Application**
  - [ ] Edit `common_config.h` - set meter reading interval
  - [ ] Edit `water_meter_amr.mk` - set camera sensor
  - [ ] Edit main makefile - set APP_TYPE
  - [ ] Review UART settings for STM32WL

- [ ] **Build Firmware**
  - [ ] Run: `./build_amr.sh`
  - [ ] Check for compilation errors
  - [ ] Verify ELF file created
  - [ ] Verify image generated: `water_meter_amr.img`
  - [ ] Check image size < 1MB

---

## Phase 5: Hardware Setup

- [ ] **HIMAX WE2 Module**
  - [ ] Module powered and detected by PC
  - [ ] Driver installed (if needed on Windows)
  - [ ] Serial port identified (e.g., /dev/ttyACM0, COM3)
  - [ ] Camera sensor connected

- [ ] **STM32WL Module**
  - [ ] Module powered
  - [ ] LoRaWAN firmware flashed
  - [ ] AT command interface working
  - [ ] Region configured (EU868/US915/etc.)

- [ ] **Connect WE2 to STM32WL**
  - [ ] UART TX → RX connected
  - [ ] UART RX → TX connected
  - [ ] GND connected
  - [ ] Verify voltage levels compatible (3.3V)

- [ ] **Test UART Communication**
  - [ ] Send test data from WE2
  - [ ] Verify STM32WL receives data
  - [ ] Check baud rate matches (115200)

---

## Phase 6: Firmware Flashing

- [ ] **Flash WE2 Firmware**
  - [ ] Run: `./flash_amr.sh /dev/ttyACM0 models/water_meter_digit_model.tflite`
  - [ ] Wait for flashing to complete
  - [ ] Press RESET button
  - [ ] Verify device boots

- [ ] **Verify Firmware Running**
  - [ ] Connect serial monitor: `screen /dev/ttyACM0 921600`
  - [ ] See "Water Meter AMR System" message
  - [ ] See "Model initialized successfully"
  - [ ] See "STM32WL UART initialized"
  - [ ] No errors in boot sequence

---

## Phase 7: LoRaWAN Configuration

- [ ] **Register Device on Network Server**
  - [ ] Create account on TTN/ChirpStack/other
  - [ ] Create application
  - [ ] Add device (OTAA recommended)
  - [ ] Note DevEUI, AppEUI, AppKey

- [ ] **Configure STM32WL**
  - [ ] Flash LoRaWAN firmware
  - [ ] Set region (AT+REGION=EU868 or similar)
  - [ ] Set DevEUI, AppEUI, AppKey
  - [ ] Test AT commands work

- [ ] **Provision WE2 Device**
  - [ ] Run provisioning tool with credentials
  - [ ] Verify credentials saved
  - [ ] Check configuration stored in flash

---

## Phase 8: System Integration Testing

- [ ] **Test Digit Recognition**
  - [ ] Run: `python3 tools/provision_device.py /dev/ttyACM0 --test-only`
  - [ ] Place device in front of water meter
  - [ ] Verify digits recognized correctly
  - [ ] Check confidence scores > 80%

- [ ] **Test UART Communication**
  - [ ] Monitor serial output
  - [ ] Verify "READ:..." messages sent
  - [ ] Check STM32WL receives data
  - [ ] Verify format correct

- [ ] **Test LoRaWAN Join**
  - [ ] Run: `python3 tools/provision_device.py /dev/ttyACM0 --join`
  - [ ] Wait for join accept (up to 30 seconds)
  - [ ] Verify device shows "Joined" status
  - [ ] Check network server shows device online

- [ ] **Test Data Transmission**
  - [ ] Trigger a meter reading
  - [ ] Verify uplink on network server
  - [ ] Check payload decoded correctly
  - [ ] Verify reading matches actual meter

---

## Phase 9: Field Deployment

- [ ] **Calibration**
  - [ ] Run: `python3 tools/provision_device.py /dev/ttyACM0 --calibrate`
  - [ ] Adjust camera angle/focus
  - [ ] Verify all digits visible
  - [ ] Test multiple readings for consistency

- [ ] **Power Optimization**
  - [ ] Measure current consumption
  - [ ] Enable sleep mode if needed
  - [ ] Calculate battery life
  - [ ] Plan battery replacement schedule

- [ ] **Enclosure**
  - [ ] Select weatherproof enclosure (IP65+)
  - [ ] Mount camera with clear view of meter
  - [ ] Protect from direct sunlight/rain
  - [ ] Ensure antenna has clear path (LoRaWAN)

- [ ] **Installation**
  - [ ] Install at meter location
  - [ ] Verify LoRaWAN signal strength
  - [ ] Test reading from installed position
  - [ ] Secure device against tampering

---

## Phase 10: Cloud Integration

- [ ] **Setup Cloud Platform**
  - [ ] Choose platform (AWS IoT/Azure/Custom)
  - [ ] Create IoT Hub or MQTT broker
  - [ ] Configure integration with network server

- [ ] **Data Pipeline**
  - [ ] Setup payload decoder
  - [ ] Configure data ingestion
  - [ ] Setup database storage
  - [ ] Create data visualization

- [ ] **Alerts and Notifications**
  - [ ] Configure anomaly detection
  - [ ] Setup leak alerts (unusual consumption)
  - [ ] Configure low battery alerts
  - [ ] Setup device offline alerts

---

## Phase 11: Monitoring and Maintenance

- [ ] **Field Testing**
  - [ ] Run: `python3 tools/field_test.py /dev/ttyACM0 --duration 120`
  - [ ] Monitor for 2+ hours
  - [ ] Check reading consistency
  - [ ] Verify confidence scores
  - [ ] Review logs for errors

- [ ] **Performance Monitoring**
  - [ ] Track reading accuracy
  - [ ] Monitor battery levels
  - [ ] Check LoRaWAN signal quality
  - [ ] Review uplink success rate

- [ ] **Maintenance Schedule**
  - [ ] Plan battery replacement (every 6-12 months)
  - [ ] Clean camera lens (monthly)
  - [ ] Verify mounting secure (quarterly)
  - [ ] Update firmware as needed

---

## Phase 12: Optimization

- [ ] **Model Improvement**
  - [ ] Collect field data
  - [ ] Retrain with real-world images
  - [ ] Improve accuracy for specific meter types
  - [ ] Reduce model size if needed

- [ ] **Power Optimization**
  - [ ] Optimize reading interval
  - [ ] Tune sleep mode settings
  - [ ] Reduce UART baud rate if possible
  - [ ] Use ADR (Adaptive Data Rate) for LoRaWAN

- [ ] **Scalability**
  - [ ] Document lessons learned
  - [ ] Create deployment checklist
  - [ ] Automate provisioning process
  - [ ] Plan multi-meter rollout

---

## Documentation Checklist

- [ ] Read `AMR_README.md` - Project overview
- [ ] Read `QUICKSTART_AMR.md` - Quick start guide
- [ ] Read `AMR_SOLUTION_GUIDE.md` - Complete technical guide
- [ ] Review `README.md` - Original WE2 SDK docs
- [ ] Check `FAQ.md` - Common issues

---

## Troubleshooting Reference

### Build Issues
- [ ] Toolchain installed and in PATH
- [ ] All dependencies installed
- [ ] Correct APP_TYPE in makefile

### Flash Issues
- [ ] Serial port permissions set
- [ ] Correct port specified
- [ ] Device connected and powered
- [ ] No other program using port

### Recognition Issues
- [ ] Model trained with good data
- [ ] Camera focused on digits
- [ ] Lighting adequate
- [ ] Digits clearly visible

### LoRaWAN Issues
- [ ] Credentials correct
- [ ] Region configured properly
- [ ] Gateway in range
- [ ] Duty cycle not exceeded

---

## Success Criteria

Your AMR system is working when:

- ✅ Device boots without errors
- ✅ Digit recognition accuracy > 95%
- ✅ Readings transmitted via LoRaWAN
- ✅ Data appears on cloud platform
- ✅ Battery life > 6 months
- ✅ Device operates reliably in field

---

## Next Steps After Completion

1. **Scale deployment** - Deploy to more meters
2. **Add features** - OTA updates, remote configuration
3. **Improve accuracy** - Continuous model retraining
4. **Expand** - Support gas, electricity meters
5. **Analytics** - Build consumption prediction models

---

**Track your progress and mark items as complete! 🎯**
