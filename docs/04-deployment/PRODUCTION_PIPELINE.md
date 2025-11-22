# Production Pipeline - Complete Manufacturing to Deployment Workflow

**Document Version**: 1.0.0
**Target Scale**: 10,000 - 1,000,000 devices
**Last Updated**: 2024-11-22
**Owner**: Production Engineering Team

---

## Executive Summary

This document describes the **complete production pipeline** for manufacturing, provisioning, testing, and deploying Water Meter AMR devices at scale. This pipeline is designed to handle production volumes from 10,000 to 1,000,000+ units with high quality and efficiency.

**Key Metrics:**
- **Production Capacity**: 1,000-10,000 units/day per production line
- **Quality Target**: <0.1% defect rate (1 in 1000)
- **Provisioning Speed**: <2 minutes per device
- **Total Manufacturing Time**: 15-20 minutes per device
- **First-Pass Yield**: >98%

---

## Table of Contents

1. [Pipeline Overview](#pipeline-overview)
2. [Stage 1: Component Sourcing](#stage-1-component-sourcing)
3. [Stage 2: PCB Assembly](#stage-2-pcb-assembly)
4. [Stage 3: Firmware Programming](#stage-3-firmware-programming)
5. [Stage 4: Device Provisioning](#stage-4-device-provisioning)
6. [Stage 5: Calibration](#stage-5-calibration)
7. [Stage 6: Quality Assurance](#stage-6-quality-assurance)
8. [Stage 7: Packaging](#stage-7-packaging)
9. [Stage 8: Warehouse & Logistics](#stage-8-warehouse--logistics)
10. [Stage 9: Field Deployment](#stage-9-field-deployment)
11. [Stage 10: Activation & Commissioning](#stage-10-activation--commissioning)
12. [Production Monitoring](#production-monitoring)
13. [Quality Control](#quality-control)
14. [Continuous Improvement](#continuous-improvement)

---

## Pipeline Overview

### End-to-End Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                    PRODUCTION PIPELINE                          │
└─────────────────────────────────────────────────────────────────┘

Stage 1: Component Sourcing (2-8 weeks)
├── Bill of Materials (BOM) Management
├── Supplier Qualification
├── Component Procurement
├── Incoming Quality Control (IQC)
└── Inventory Management
         │
         ▼
Stage 2: PCB Assembly (2-4 hours)
├── SMT Assembly (Surface Mount)
├── Through-Hole Components
├── Automated Optical Inspection (AOI)
├── X-Ray Inspection (BGA components)
└── In-Circuit Test (ICT)
         │
         ▼
Stage 3: Firmware Programming (3-5 minutes)
├── Bootloader Flashing
├── Main Firmware Flashing
├── Model Flashing
├── Verification
└── Serial Number Assignment
         │
         ▼
Stage 4: Device Provisioning (2-3 minutes)
├── Unique ID Generation
├── LoRaWAN Credentials Assignment
├── Security Key Generation
├── Cloud Registration
└── Configuration Storage
         │
         ▼
Stage 5: Calibration (3-5 minutes)
├── Camera Focus Adjustment
├── Digit Recognition Test
├── Power Consumption Test
├── RSSI/RF Test
└── Calibration Data Storage
         │
         ▼
Stage 6: Quality Assurance (5-10 minutes)
├── Functional Test Suite
├── Environmental Test
├── LoRaWAN Join Test
├── End-to-End Test
└── Final Visual Inspection
         │
         ▼
Stage 7: Packaging (1-2 minutes)
├── QR Code/Barcode Label
├── User Manual & Documentation
├── Accessories (mounting, battery)
├── Anti-Static Packaging
└── Shipping Box
         │
         ▼
Stage 8: Warehouse & Logistics (variable)
├── Inventory Registration
├── Batch Tracking
├── Storage (temp/humidity controlled)
├── Order Fulfillment
└── Shipping Coordination
         │
         ▼
Stage 9: Field Deployment (30-60 minutes)
├── Site Survey
├── Device Installation
├── Network Connectivity Check
├── Initial Reading Test
└── Documentation
         │
         ▼
Stage 10: Activation & Commissioning (5-10 minutes)
├── Cloud Activation
├── Join LoRaWAN Network
├── First Data Transmission
├── Monitoring Setup
└── Customer Handover

┌─────────────────────────────────────────────────────────────────┐
│              CONTINUOUS MONITORING & SUPPORT                    │
├─────────────────────────────────────────────────────────────────┤
│  • Device Health Monitoring                                     │
│  • Performance Analytics                                        │
│  • Predictive Maintenance                                       │
│  • Field Support                                                │
│  • Firmware Updates (OTA)                                       │
└─────────────────────────────────────────────────────────────────┘
```

---

## Stage 1: Component Sourcing

### 1.1 Bill of Materials (BOM)

**Core Components:**

| Component | Supplier | Lead Time | MOQ | Cost/Unit |
|-----------|----------|-----------|-----|-----------|
| HIMAX WE2 Module | Seeed/HIMAX | 8-12 weeks | 1,000 | $15-20 |
| STM32WL55 Module | STMicro/Distributor | 4-8 weeks | 100 | $8-12 |
| Camera Sensor (HM0360) | HIMAX | 8-12 weeks | 1,000 | $3-5 |
| LoRa Antenna (868/915MHz) | Multiple | 2-4 weeks | 500 | $0.50-1 |
| Li-Ion Battery (3.7V, 5000mAh) | Multiple | 2-4 weeks | 1,000 | $3-5 |
| Custom PCB | PCB Fab | 2-3 weeks | 100 | $2-4 |
| Enclosure (IP67) | Injection Molding | 4-8 weeks | 1,000 | $3-5 |
| Misc (resistors, caps, etc.) | Multiple | 1-2 weeks | Varies | $1-2 |

**Total BOM Cost**: $35-55 per unit (at scale)

### 1.2 Supplier Management

**Qualification Criteria:**
- ✅ ISO 9001 Certified
- ✅ Proven reliability (>99% on-time delivery)
- ✅ Quality track record (<0.5% defect rate)
- ✅ Financial stability
- ✅ Scalability (can support 100K+ units)

**Dual Sourcing Strategy:**
- Primary supplier for 70% of volume
- Secondary supplier for 30% of volume
- Reduces supply chain risk

### 1.3 Inventory Management

**Safety Stock Levels:**
- **Critical Components** (WE2, STM32WL): 4-week buffer
- **Long Lead Time**: 8-week buffer
- **Standard Components**: 2-week buffer

**Inventory Tracking:**
```bash
# Use tools/manufacturing/inventory-tracker
python3 tools/manufacturing/inventory-tracker/main.py \
  --check-levels \
  --alert-threshold 2weeks \
  --forecast-demand 30days
```

---

## Stage 2: PCB Assembly

### 2.1 PCB Fabrication

**Specifications:**
- **PCB Type**: 4-layer FR4
- **Thickness**: 1.6mm
- **Surface Finish**: ENIG (Electroless Nickel Immersion Gold)
- **Copper Weight**: 1oz (35μm)
- **Silkscreen**: White on green soldermask

**Quality Requirements:**
- ✅ IPC Class 2 (minimum)
- ✅ AOI 100% coverage
- ✅ X-ray for BGA components
- ✅ Flying probe test

### 2.2 SMT Assembly Process

**Equipment Required:**
- Solder Paste Printer (SPI)
- Pick-and-Place Machine (high-speed)
- Reflow Oven (8-10 zone)
- AOI Machine (post-reflow)

**Process Steps:**

1. **Solder Paste Application** (30 seconds)
   - Automated stencil printing
   - SPI inspection (coplanarity)

2. **Component Placement** (2-3 minutes)
   - High-speed pick-and-place
   - Vision-guided placement
   - Placement accuracy: ±0.05mm

3. **Reflow Soldering** (5-8 minutes)
   - Temperature profile per component datasheet
   - Profile: Preheat → Soak → Reflow → Cooling

4. **Post-Reflow Inspection** (1 minute)
   - AOI for solder joints
   - Automated defect detection
   - Classification: Pass/Fail/Review

### 2.3 Through-Hole Assembly

**Components:**
- Battery connector
- Antenna connector
- Programming headers (if used)

**Process:**
- Wave soldering or selective soldering
- Manual touch-up if needed

### 2.4 In-Circuit Test (ICT)

**Test Coverage:**
- ✅ Power rails (all voltages)
- ✅ Component presence
- ✅ Shorts/Opens detection
- ✅ Component values verification

**Test Time**: 30-60 seconds per board

**Acceptance Criteria:**
- 100% pass rate for properly assembled boards
- Automated pass/fail decision

---

## Stage 3: Firmware Programming

### 3.1 Programming Station Setup

**Equipment:**
- **JTAG/SWD Programmer**: Segger J-Link, ST-Link V3
- **Programming Jig**: Pogo-pin bed-of-nails
- **PC with Programming Software**
- **Barcode Scanner**: For serial number tracking

**Station Layout:**
```
┌──────────────────────────────────────┐
│     FIRMWARE PROGRAMMING STATION      │
├──────────────────────────────────────┤
│                                       │
│  [Barcode Scanner]                   │
│                                       │
│  ┌────────────────┐                  │
│  │  Programming   │                  │
│  │     Jig        │◄─── [J-Link]     │
│  │   (Pogo Pins)  │                  │
│  └────────────────┘                  │
│          │                            │
│          └──► [Test LED Display]     │
│                                       │
│  [PC Running Flasher]                │
│                                       │
└──────────────────────────────────────┘
```

### 3.2 Programming Procedure

**Automated Programming Script:**

```bash
#!/bin/bash
# tools/manufacturing/flasher/flash_production.sh

# 1. Scan device barcode/serial number
SERIAL_NUMBER=$(scan_barcode)

# 2. Flash bootloader
echo "Flashing bootloader for $SERIAL_NUMBER..."
JLinkExe -device STM32WL55JC -if SWD -speed 4000 \
  -CommandFile bootloader_flash.jlink

# 3. Flash main firmware
echo "Flashing main firmware..."
JLinkExe -device STM32WL55JC -if SWD -speed 4000 \
  -CommandFile firmware_flash.jlink

# 4. Flash ML model to WE2
echo "Flashing ML model..."
python3 flash_we2_model.py \
  --model models/water_meter_digit_model.tflite \
  --address 0x200000 \
  --device $SERIAL_NUMBER

# 5. Verify programming
echo "Verifying..."
JLinkExe -device STM32WL55JC -if SWD -speed 4000 \
  -CommandFile verify.jlink

# 6. Write serial number to OTP
echo "Writing serial number to OTP..."
python3 write_serial_otp.py --serial $SERIAL_NUMBER

# 7. Log to database
echo "Logging to production database..."
curl -X POST https://production-api.company.com/devices/programmed \
  -H "Content-Type: application/json" \
  -d "{\"serial\": \"$SERIAL_NUMBER\", \"timestamp\": \"$(date -Iseconds)\"}"

echo "Programming complete for $SERIAL_NUMBER"
```

### 3.3 Version Control

**Firmware Versioning:**
- Major.Minor.Patch (Semantic Versioning)
- Example: v1.2.3
- Stored in device flash and reported in telemetry

**Model Versioning:**
- Independent versioning from firmware
- Example: model_v2.1.0
- Tracked separately for OTA updates

**Production Tracking:**
```json
{
  "serial_number": "AMR-WM-2024-001234",
  "firmware_version": "1.2.3",
  "model_version": "2.1.0",
  "bootloader_version": "1.0.0",
  "programming_date": "2024-11-22T10:30:00Z",
  "programmer_id": "STATION_01",
  "status": "PROGRAMMED"
}
```

---

## Stage 4: Device Provisioning

### 4.1 Provisioning Architecture

```
┌──────────────────────────────────────────────────────────────┐
│              PROVISIONING INFRASTRUCTURE                      │
└──────────────────────────────────────────────────────────────┘

Manufacturing Floor:
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│ Station 1   │     │ Station 2   │     │ Station N   │
│ Provision   │────►│ Provision   │────►│ Provision   │
│ Device      │     │ Device      │     │ Device      │
└──────┬──────┘     └──────┬──────┘     └──────┬──────┘
       │                   │                   │
       └───────────────────┼───────────────────┘
                           │
                           ▼
                  ┌────────────────┐
                  │ Provisioning   │
                  │ Server (Local) │
                  └────────┬───────┘
                           │
                           │ Secured VPN/API
                           ▼
Cloud Infrastructure:
                  ┌────────────────┐
                  │   LoRaWAN      │
                  │ Network Server │
                  └────────┬───────┘
                           │
                  ┌────────▼───────┐
                  │   Device       │
                  │   Registry     │
                  └────────┬───────┘
                           │
                  ┌────────▼───────┐
                  │   Cloud        │
                  │   Platform     │
                  └────────────────┘
```

### 4.2 Provisioning Process

**Step-by-Step:**

1. **Generate Unique Identifiers**
   ```python
   # Generate DevEUI (8 bytes, globally unique)
   dev_eui = generate_ieee_eui64()

   # Generate AppKey (16 bytes, random)
   app_key = generate_random_key(16)

   # Device Serial Number
   serial = f"AMR-WM-{year}-{sequential_number:06d}"
   ```

2. **Assign LoRaWAN Credentials**
   - DevEUI: Unique device identifier
   - AppEUI/JoinEUI: Application identifier
   - AppKey: Root encryption key (AES-128)

3. **Register in Network Server**
   ```bash
   # Using TTN CLI
   ttn-lw-cli devices create \
     --application-id water-meter-amr \
     --device-id $DEVICE_ID \
     --dev-eui $DEV_EUI \
     --app-eui $APP_EUI \
     --app-key $APP_KEY \
     --lorawan-version 1.0.3 \
     --lorawan-phy-version 1.0.3-a \
     --frequency-plan-id EU_863_870
   ```

4. **Write to Device**
   ```bash
   python3 tools/provisioning/cli/provision.py \
     --port /dev/ttyACM0 \
     --serial $SERIAL \
     --deveui $DEV_EUI \
     --appeui $APP_EUI \
     --appkey $APP_KEY \
     --batch-mode
   ```

5. **Store in Database**
   ```sql
   INSERT INTO devices (
     serial_number,
     dev_eui,
     app_eui,
     app_key,
     provisioned_at,
     provisioner_id,
     batch_number
   ) VALUES (?, ?, ?, ?, ?, ?, ?);
   ```

6. **Generate QR Code**
   - Contains: Serial Number, DevEUI, Activation Code
   - Printed on device label
   - Used for field activation

### 4.3 Batch Provisioning Tool

**High-Performance Provisioning:**

```bash
# Provision entire batch (1000 devices)
python3 tools/provisioning/batch/batch_provision.py \
  --batch-file batch_AMR_2024_001.csv \
  --network-server ttn \
  --region EU868 \
  --parallel 10 \
  --output-report provision_report.json
```

**Performance:**
- **Throughput**: 30-50 devices per minute (with 10 parallel stations)
- **Time for 1000 devices**: 20-35 minutes
- **Time for 10,000 devices**: 3.5-6 hours

---

## Stage 5: Calibration

### 5.1 Camera Calibration

**Calibration Target:**
```
┌─────────────────────────────────┐
│    CALIBRATION TARGET CARD      │
│                                 │
│    ┌───┬───┬───┬───┬───┐       │
│    │ 0 │ 1 │ 2 │ 3 │ 4 │       │
│    ├───┼───┼───┼───┼───┤       │
│    │ 5 │ 6 │ 7 │ 8 │ 9 │       │
│    └───┴───┴───┴───┴───┘       │
│                                 │
│  [Reference Digits - Standard]  │
│  [Known Size and Position]      │
└─────────────────────────────────┘
```

**Calibration Procedure:**

1. **Focus Adjustment**
   - Place device at standard distance (10-15cm from target)
   - Adjust camera focus for sharpest image
   - Capture test image
   - Verify edge detection quality

2. **Recognition Test**
   ```bash
   python3 tools/manufacturing/calibration/camera_calibration.py \
     --device /dev/ttyACM0 \
     --target-card standard_digits \
     --expected-accuracy 98 \
     --save-calibration
   ```

3. **Store Calibration Data**
   - Focus position
   - Exposure settings
   - Recognition baseline
   - Stored in device flash

**Acceptance Criteria:**
- ✅ Recognition accuracy ≥ 98% on calibration target
- ✅ All 10 digits correctly recognized
- ✅ Confidence scores ≥ 95%

### 5.2 RF Calibration

**LoRaWAN RF Test:**

1. **TX Power Test**
   - Measure output power with spectrum analyzer
   - Verify: 14dBm ±1dB (EU868)

2. **RX Sensitivity Test**
   - Test with RF chamber or signal generator
   - Verify: -137dBm sensitivity

3. **Frequency Accuracy**
   - Verify frequency offset < ±10ppm

**Test Equipment:**
- Spectrum Analyzer
- Signal Generator
- RF Chamber (optional)

### 5.3 Power Consumption Test

**Battery Life Validation:**

```bash
# Measure current consumption in different modes
python3 tools/manufacturing/test-jig/power_test.py \
  --device /dev/ttyACM0 \
  --test-modes sleep,active,tx \
  --duration 60 \
  --expected-sleep-current 10uA \
  --expected-active-current 50mA \
  --expected-tx-current 120mA
```

**Acceptance Criteria:**
- Sleep mode: <20µA
- Active mode (inference): <100mA
- TX mode (LoRa): <150mA
- Battery life projection: >12 months

---

## Stage 6: Quality Assurance

### 6.1 Functional Test Suite

**Automated Test Sequence:**

```python
# tools/manufacturing/test-jig/functional_test.py

def functional_test_suite(device_serial):
    """
    Complete functional test - takes 5-10 minutes
    """
    results = {
        'device': device_serial,
        'tests': []
    }

    # Test 1: Power-on Self-Test
    print("[1/10] Power-on self-test...")
    result = test_power_on()
    results['tests'].append(result)

    # Test 2: Camera Test
    print("[2/10] Camera capture test...")
    result = test_camera_capture()
    results['tests'].append(result)

    # Test 3: Digit Recognition
    print("[3/10] Digit recognition test...")
    result = test_digit_recognition()
    results['tests'].append(result)

    # Test 4: UART Communication
    print("[4/10] UART communication test...")
    result = test_uart_communication()
    results['tests'].append(result)

    # Test 5: LoRaWAN Join
    print("[5/10] LoRaWAN join test...")
    result = test_lorawan_join()
    results['tests'].append(result)

    # Test 6: Data Transmission
    print("[6/10] Data transmission test...")
    result = test_data_transmission()
    results['tests'].append(result)

    # Test 7: Battery Test
    print("[7/10] Battery charging/monitoring test...")
    result = test_battery()
    results['tests'].append(result)

    # Test 8: RTC Test
    print("[8/10] Real-time clock test...")
    result = test_rtc()
    results['tests'].append(result)

    # Test 9: Storage Test
    print("[9/10] Flash storage test...")
    result = test_flash_storage()
    results['tests'].append(result)

    # Test 10: Environmental Sensors
    print("[10/10] Environmental sensors test...")
    result = test_env_sensors()
    results['tests'].append(result)

    # Calculate overall result
    all_passed = all(t['status'] == 'PASS' for t in results['tests'])
    results['overall'] = 'PASS' if all_passed else 'FAIL'

    # Upload to QA database
    upload_test_results(results)

    return results
```

### 6.2 Environmental Testing

**Sample Testing (not every unit):**

- **Temperature Test**: -20°C to +60°C
- **Humidity Test**: 10% to 90% RH
- **Vibration Test**: Per IEC 60068-2-6
- **Drop Test**: 1m drop per IEC 60068-2-32
- **IP67 Ingress Protection**: Dust and water

**Sample Size**: 1% of production (minimum 30 units per batch)

### 6.3 Final Visual Inspection

**Inspection Checklist:**
- ✅ No visible defects (scratches, dents)
- ✅ All labels applied correctly
- ✅ Enclosure properly sealed
- ✅ Antenna properly attached
- ✅ QR code readable
- ✅ LED indicators functional

---

## Stage 7: Packaging

### 7.1 Packaging Materials

**Package Contents:**
1. AMR Device (1x)
2. Mounting Bracket (1x)
3. Mounting Screws (4x)
4. User Manual (QR code to digital manual)
5. Quick Start Guide (printed card)
6. Warranty Card
7. QR Code Activation Sticker

**Packaging Materials:**
- Anti-static bag
- Protective foam
- Corrugated box (10-pack or 100-pack master carton)

### 7.2 Labeling

**Device Label (on enclosure):**
```
┌──────────────────────────────────┐
│  [Company Logo]                  │
│                                  │
│  Water Meter AMR                 │
│  Model: AMR-WM-2024              │
│                                  │
│  S/N: AMR-WM-2024-001234        │
│                                  │
│  [QR Code]                       │
│  (Contains: Serial, DevEUI,      │
│   Activation Code)               │
│                                  │
│  CE  FCC  IC  [Certifications]  │
│                                  │
│  Made in [Country]               │
└──────────────────────────────────┘
```

**Master Carton Label:**
```
┌──────────────────────────────────┐
│  WATER METER AMR DEVICES         │
│  Quantity: 100 units             │
│  Batch: 2024-001                 │
│  S/N Range: 001000 - 001099      │
│  Manufactured: 2024-11-22        │
│                                  │
│  [Barcode]                       │
│                                  │
│  ⚠ Handle with Care              │
│  ⚠ Keep Dry                      │
│  ⚠ This Side Up                  │
└──────────────────────────────────┘
```

---

## Stage 8: Warehouse & Logistics

### 8.1 Inventory Management System

**Warehouse Management:**

```sql
-- Device inventory tracking
CREATE TABLE warehouse_inventory (
  id SERIAL PRIMARY KEY,
  batch_number VARCHAR(50),
  serial_range_start VARCHAR(50),
  serial_range_end VARCHAR(50),
  quantity INTEGER,
  location VARCHAR(100),
  received_date TIMESTAMP,
  status VARCHAR(20),  -- RECEIVED, IN_STOCK, SHIPPED, DEPLOYED
  notes TEXT
);

-- Stock movements
CREATE TABLE stock_movements (
  id SERIAL PRIMARY KEY,
  batch_number VARCHAR(50),
  movement_type VARCHAR(20),  -- IN, OUT, TRANSFER
  quantity INTEGER,
  from_location VARCHAR(100),
  to_location VARCHAR(100),
  moved_by VARCHAR(100),
  moved_at TIMESTAMP,
  reason TEXT
);
```

### 8.2 Storage Conditions

**Environmental Requirements:**
- Temperature: 15-25°C
- Humidity: 40-60% RH
- No direct sunlight
- Clean, dust-free environment

**Battery Storage:**
- Store at 40-60% charge
- Recharge every 3 months if in long-term storage

### 8.3 Order Fulfillment

**Process Flow:**

1. **Order Received** → System checks inventory
2. **Pick List Generated** → Warehouse staff retrieves items
3. **Verification** → Scan barcodes to confirm items
4. **Packing** → Pack in shipping boxes with documentation
5. **Shipping Label** → Generate tracking label
6. **Shipment** → Hand over to logistics partner
7. **Tracking** → Update status in system

---

## Stage 9: Field Deployment

### 9.1 Site Survey

**Pre-Installation Checklist:**

```markdown
# Site Survey Checklist

## Location Assessment
- [ ] Water meter accessible
- [ ] Clear view of meter digits
- [ ] Protected from weather
- [ ] Secure mounting surface available
- [ ] Power source if needed (or battery only)

## Network Coverage
- [ ] LoRaWAN coverage verified
- [ ] Signal strength > -120dBm
- [ ] Gateway within range (<5km)
- [ ] No significant obstacles

## Environmental Conditions
- [ ] Temperature range suitable (-20 to +60°C)
- [ ] Humidity acceptable
- [ ] No flooding risk
- [ ] Vibration level acceptable

## Security
- [ ] Tamper-proof mounting possible
- [ ] Visible to property owner
- [ ] Not accessible to vandals
- [ ] Compliant with local regulations
```

### 9.2 Installation Procedure

**Installation Steps:**

1. **Unbox and Inspect**
   - Check device condition
   - Verify serial number matches order

2. **Mount Device**
   - Position at optimal distance (10-15cm from meter)
   - Ensure clear view of all digits
   - Secure with provided mounting hardware
   - Verify stable mounting

3. **Power On**
   - Insert/connect battery
   - Verify LED indicators

4. **Connectivity Check**
   - Wait for LoRaWAN join (max 30 seconds)
   - Verify join success (LED pattern or app)

5. **Initial Reading Test**
   - Trigger manual reading
   - Verify recognized digits match physical meter
   - Check confidence scores

6. **Documentation**
   - Photo of installation
   - GPS coordinates
   - Installation notes
   - Sign-off by installer and property owner

**Installation Time:** 30-60 minutes per device

---

## Stage 10: Activation & Commissioning

### 10.1 Cloud Activation

**Activation Process:**

```bash
# Mobile app or web portal
1. Scan device QR code
2. Verify location on map
3. Associate with customer account
4. Set billing parameters
5. Configure alert thresholds
6. Activate monitoring
```

**Automated Activation:**

```python
# tools/deployment/activation_tool.py

def activate_device(qr_code_data):
    """
    Activate device in cloud platform
    """
    # Parse QR code
    device_info = parse_qr_code(qr_code_data)

    # Verify device exists in registry
    device = verify_device(device_info['serial'])

    # Associate with customer
    link_to_customer(device, customer_id)

    # Set configuration
    configure_device(device, {
        'reading_interval': 3600,  # 1 hour
        'alert_threshold': 1000,   # 1000L/hour leak alert
        'timezone': 'UTC'
    })

    # Activate monitoring
    enable_monitoring(device)

    # Send activation confirmation
    send_notification(customer_id, f"Device {device.serial} activated")

    return device
```

### 10.2 First Data Transmission

**Verification:**

1. Device joins LoRaWAN network
2. First uplink received within 5 minutes
3. Data decoded correctly
4. Displayed in dashboard
5. Alerts configured and tested

**Success Criteria:**
- ✅ Join successful within 30 seconds
- ✅ First uplink within 5 minutes
- ✅ Data accuracy 100% (matches physical meter)
- ✅ Dashboard shows real-time data

---

## Production Monitoring

### Real-Time Dashboards

**Manufacturing Dashboard:**

```
┌────────────────────────────────────────────────────────┐
│        PRODUCTION LINE MONITORING                      │
├────────────────────────────────────────────────────────┤
│                                                         │
│  Today's Production:  847 / 1000  (84.7%)              │
│  First Pass Yield:    98.3%                             │
│  Defect Rate:         0.12%                             │
│  Current Line Speed:  42 units/hour                     │
│                                                         │
│  ┌──────────────────────────────────────┐             │
│  │  Station Status:                      │             │
│  │  ✅ Assembly:     RUNNING  (2 units)  │             │
│  │  ✅ Programming:  RUNNING  (1 unit)   │             │
│  │  ✅ Provisioning: RUNNING  (1 unit)   │             │
│  │  ⚠️  Calibration:  SLOW    (3 units)   │             │
│  │  ✅ QA Testing:   RUNNING  (1 unit)   │             │
│  │  ✅ Packaging:    RUNNING  (0 units)  │             │
│  └──────────────────────────────────────┘             │
│                                                         │
│  Alerts:                                                │
│  ⚠️  Calibration station slow - check operator         │
│                                                         │
└────────────────────────────────────────────────────────┘
```

### Quality Metrics

**Key Performance Indicators:**

| Metric | Target | Current | Status |
|--------|--------|---------|--------|
| First Pass Yield | >98% | 98.3% | ✅ GOOD |
| Defect Rate | <0.2% | 0.12% | ✅ EXCELLENT |
| Programming Success | >99.5% | 99.8% | ✅ EXCELLENT |
| Calibration Pass | >95% | 96.1% | ✅ GOOD |
| Functional Test Pass | >98% | 97.9% | ⚠️ MONITOR |

---

## Quality Control

### Statistical Process Control (SPC)

**Control Charts:**
- P-Chart for defect rate
- X-bar and R charts for continuous variables
- Real-time monitoring
- Automatic alerts on out-of-control conditions

### Root Cause Analysis

**Process:**
1. Defect detected
2. Immediate containment
3. 5 Whys analysis
4. Corrective action
5. Verification
6. Documentation

---

## Continuous Improvement

### Kaizen Events

**Monthly Reviews:**
- Production efficiency
- Quality metrics
- Cost reduction opportunities
- Process improvements

### Lean Manufacturing

**Waste Reduction:**
- Minimize WIP inventory
- Reduce cycle time
- Eliminate non-value-added steps
- Continuous flow manufacturing

---

## Appendices

### A. Production Capacity Calculator

```python
# Calculate production capacity
stations = {
    'assembly': {'cycle_time': 180, 'stations': 2},      # 3 min
    'programming': {'cycle_time': 240, 'stations': 3},   # 4 min
    'provisioning': {'cycle_time': 150, 'stations': 2},  # 2.5 min
    'calibration': {'cycle_time': 300, 'stations': 3},   # 5 min
    'qa_test': {'cycle_time': 420, 'stations': 5},       # 7 min
    'packaging': {'cycle_time': 90, 'stations': 1}       # 1.5 min
}

def calculate_capacity(stations, shift_hours=8):
    bottleneck = max(stations.items(),
                    key=lambda x: x[1]['cycle_time'] / x[1]['stations'])

    capacity_per_hour = (3600 / bottleneck[1]['cycle_time']) * bottleneck[1]['stations']
    daily_capacity = capacity_per_hour * shift_hours

    return {
        'bottleneck_station': bottleneck[0],
        'units_per_hour': capacity_per_hour,
        'units_per_day': daily_capacity,
        'units_per_month': daily_capacity * 22  # working days
    }
```

### B. Cost Breakdown

**Manufacturing Cost Per Unit:**

| Category | Cost | Percentage |
|----------|------|------------|
| Components (BOM) | $45 | 56% |
| PCB Assembly | $8 | 10% |
| Labor (assembly, test, pack) | $12 | 15% |
| Overhead (facility, equipment) | $6 | 7% |
| QA/Testing | $3 | 4% |
| Packaging | $2 | 2% |
| Logistics | $4 | 5% |
| **Total** | **$80** | **100%** |

**At scale (100K+ units), target cost: $60-70**

---

**Document Control:**
- **Version**: 1.0.0
- **Last Updated**: 2024-11-22
- **Next Review**: 2025-01-22
- **Owner**: Production Engineering Team
- **Approved By**: VP of Manufacturing

---

**Related Documents:**
- [Manufacturing Process Guide](../06-manufacturing/MANUFACTURING_PROCESS.md)
- [Quality Assurance Procedures](../06-manufacturing/QUALITY_ASSURANCE.md)
- [Batch Provisioning Guide](../06-manufacturing/BATCH_PROVISIONING.md)
- [Deployment Strategy](DEPLOYMENT_STRATEGY.md)
