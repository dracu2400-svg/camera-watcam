# Complete AMR (Automatic Meter Reading) Solution Guide
## Water Meter Digit Recognition using HIMAX WE2 + STM32WL LoRaWAN

---

## 📋 Table of Contents

1. [System Architecture](#system-architecture)
2. [Hardware Requirements](#hardware-requirements)
3. [Software Requirements](#software-requirements)
4. [Phase 1: Model Training for Digit Recognition](#phase-1-model-training-for-digit-recognition)
5. [Phase 2: Application Development](#phase-2-application-development)
6. [Phase 3: LoRaWAN Communication Integration](#phase-3-lorawan-communication-integration)
7. [Phase 4: AMR Provisioning Method](#phase-4-amr-provisioning-method)
8. [Phase 5: Build and Flash Firmware](#phase-5-build-and-flash-firmware)
9. [Phase 6: Testing and Deployment](#phase-6-testing-and-deployment)
10. [Troubleshooting](#troubleshooting)

---

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    AMR Solution Overview                     │
└─────────────────────────────────────────────────────────────┘

Water Meter
    │
    │ (Visual)
    ▼
┌──────────────────┐
│  HIMAX WE2       │
│  - Camera Input  │         ┌─────────────────┐
│  - Digit         │  UART   │   STM32WL       │    LoRaWAN
│    Recognition   │◄───────►│   - AT Commands │◄───────────►  Cloud
│  - TFLite Model  │         │   - LoRaWAN     │              Platform
└──────────────────┘         │     Stack       │
                             └─────────────────┘
```

**Data Flow:**
1. WE2 captures water meter image
2. TFLite model performs digit recognition
3. Recognized digits sent to STM32WL via UART
4. STM32WL transmits data to cloud via LoRaWAN
5. Cloud platform receives and processes meter readings

---

## Hardware Requirements

### Required Components:
- **HIMAX WE2 (Grove Vision AI Module V2)** - Main vision processing unit
- **STM32WL Module** - LoRaWAN connectivity (e.g., STM32WLE5, NUCLEO-WL55JC)
- **Camera Module** - Compatible with WE2 (HM0360, OV5647, IMX219, etc.)
- **Power Supply** - 3.3V/5V depending on module
- **USB Cable** - For programming and debugging
- **Jumper Wires** - For UART connections

### Pin Connections (WE2 to STM32WL):
```
HIMAX WE2          STM32WL
---------          --------
UART_TX    ───►    UART_RX (PA3 or PA10)
UART_RX    ◄───    UART_TX (PA2 or PA9)
GND        ───     GND
```

---

## Software Requirements

### Development Environment:
```bash
# Ubuntu 20.04+ recommended
sudo apt update
sudo apt install make git python3 python3-pip

# ARM GNU Toolchain 13.2
wget https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz
tar -xvf arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz
export PATH="$HOME/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-eabi/bin/:$PATH"
```

### Python Dependencies:
```bash
pip install tensorflow==2.15.0
pip install numpy pillow opencv-python
pip install scikit-learn matplotlib
pip install serial pyserial
```

### Install xmodem for flashing:
```bash
cd xmodem
pip install -r requirements.txt
```

---

## Phase 1: Model Training for Digit Recognition

### Step 1.1: Dataset Preparation

#### Collect Water Meter Images:
```bash
# Create dataset directory structure
mkdir -p dataset/water_meter/{raw,processed,train,val,test}
mkdir -p dataset/water_meter/train/{0,1,2,3,4,5,6,7,8,9}
mkdir -p dataset/water_meter/val/{0,1,2,3,4,5,6,7,8,9}
mkdir -p dataset/water_meter/test/{0,1,2,3,4,5,6,7,8,9}
```

**Dataset Requirements:**
- **Minimum**: 200 images per digit class (0-9)
- **Recommended**: 500-1000 images per digit class
- **Image Format**: JPG/PNG
- **Resolution**: Capture at WE2 native resolution, will be resized to 28x28 or 32x32
- **Variety**: Different lighting conditions, angles, meter types

**Data Collection Methods:**
1. **Manual Collection**: Take photos of water meters with phone/camera
2. **Synthetic Data**: Generate digit images with augmentation
3. **Public Datasets**: MNIST, SVHN (Street View House Numbers) as baseline

#### Create Data Preparation Script:
```bash
mkdir -p training_scripts
```

**File: `training_scripts/prepare_dataset.py`**
```python
import cv2
import numpy as np
import os
from pathlib import Path

def preprocess_image(image_path, output_size=(28, 28)):
    """
    Preprocess water meter digit images
    - Resize to target size
    - Convert to grayscale
    - Normalize
    - Apply preprocessing specific to water meters
    """
    img = cv2.imread(image_path)

    # Convert to grayscale
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    # Apply CLAHE for better contrast
    clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8,8))
    enhanced = clahe.apply(gray)

    # Resize
    resized = cv2.resize(enhanced, output_size, interpolation=cv2.INTER_AREA)

    # Normalize to [0, 1]
    normalized = resized.astype(np.float32) / 255.0

    return normalized

def augment_image(img):
    """
    Apply data augmentation
    """
    augmented = []

    # Original
    augmented.append(img)

    # Slight rotation
    for angle in [-5, 5]:
        M = cv2.getRotationMatrix2D((img.shape[1]//2, img.shape[0]//2), angle, 1.0)
        rotated = cv2.warpAffine(img, M, (img.shape[1], img.shape[0]))
        augmented.append(rotated)

    # Brightness adjustment
    for beta in [-0.1, 0.1]:
        adjusted = np.clip(img + beta, 0, 1)
        augmented.append(adjusted)

    return augmented

def prepare_dataset(input_dir, output_dir):
    """
    Process all images in dataset
    """
    for digit in range(10):
        input_path = Path(input_dir) / str(digit)
        output_path = Path(output_dir) / str(digit)
        output_path.mkdir(parents=True, exist_ok=True)

        count = 0
        for img_file in input_path.glob('*.jpg'):
            processed = preprocess_image(str(img_file))

            # Apply augmentation
            augmented_images = augment_image(processed)

            for idx, aug_img in enumerate(augmented_images):
                output_file = output_path / f"{img_file.stem}_aug{idx}.npy"
                np.save(output_file, aug_img)
                count += 1

        print(f"Processed digit {digit}: {count} images")

if __name__ == "__main__":
    # Process training data
    prepare_dataset('dataset/water_meter/raw/train', 'dataset/water_meter/train')
    prepare_dataset('dataset/water_meter/raw/val', 'dataset/water_meter/val')
    prepare_dataset('dataset/water_meter/raw/test', 'dataset/water_meter/test')
```

### Step 1.2: Model Training

**File: `training_scripts/train_digit_model.py`**
```python
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers
import numpy as np
import os
from pathlib import Path

def load_dataset(data_dir):
    """
    Load preprocessed dataset
    """
    images = []
    labels = []

    for digit in range(10):
        digit_path = Path(data_dir) / str(digit)
        for npy_file in digit_path.glob('*.npy'):
            img = np.load(npy_file)
            images.append(img)
            labels.append(digit)

    images = np.array(images)
    labels = np.array(labels)

    # Add channel dimension
    images = np.expand_dims(images, -1)

    return images, labels

def create_digit_model(input_shape=(28, 28, 1), num_classes=10):
    """
    Create lightweight CNN for digit recognition
    Optimized for TensorFlow Lite Micro on WE2
    """
    model = keras.Sequential([
        # Input layer
        layers.Input(shape=input_shape),

        # Conv Block 1
        layers.Conv2D(16, (3, 3), activation='relu', padding='same'),
        layers.MaxPooling2D((2, 2)),

        # Conv Block 2
        layers.Conv2D(32, (3, 3), activation='relu', padding='same'),
        layers.MaxPooling2D((2, 2)),

        # Conv Block 3
        layers.Conv2D(32, (3, 3), activation='relu', padding='same'),

        # Classifier
        layers.Flatten(),
        layers.Dense(64, activation='relu'),
        layers.Dropout(0.3),
        layers.Dense(num_classes, activation='softmax')
    ])

    return model

def train_model():
    """
    Train the digit recognition model
    """
    # Load data
    print("Loading dataset...")
    X_train, y_train = load_dataset('dataset/water_meter/train')
    X_val, y_val = load_dataset('dataset/water_meter/val')
    X_test, y_test = load_dataset('dataset/water_meter/test')

    print(f"Training samples: {len(X_train)}")
    print(f"Validation samples: {len(X_val)}")
    print(f"Test samples: {len(X_test)}")

    # Create model
    model = create_digit_model()
    model.summary()

    # Compile
    model.compile(
        optimizer='adam',
        loss='sparse_categorical_crossentropy',
        metrics=['accuracy']
    )

    # Callbacks
    callbacks = [
        keras.callbacks.ModelCheckpoint(
            'models/best_digit_model.h5',
            save_best_only=True,
            monitor='val_accuracy'
        ),
        keras.callbacks.EarlyStopping(
            patience=10,
            restore_best_weights=True
        ),
        keras.callbacks.ReduceLROnPlateau(
            factor=0.5,
            patience=5
        )
    ]

    # Train
    history = model.fit(
        X_train, y_train,
        validation_data=(X_val, y_val),
        epochs=50,
        batch_size=32,
        callbacks=callbacks
    )

    # Evaluate
    test_loss, test_acc = model.evaluate(X_test, y_test)
    print(f"Test accuracy: {test_acc:.4f}")

    return model

if __name__ == "__main__":
    os.makedirs('models', exist_ok=True)
    model = train_model()
```

### Step 1.3: Convert Model to TensorFlow Lite

**File: `training_scripts/convert_to_tflite.py`**
```python
import tensorflow as tf
import numpy as np

def representative_dataset_gen():
    """
    Representative dataset for quantization
    """
    # Load some validation images
    for i in range(100):
        # Load random validation image
        data = np.random.rand(1, 28, 28, 1).astype(np.float32)
        yield [data]

def convert_to_tflite(model_path, output_path):
    """
    Convert Keras model to TFLite with quantization
    """
    # Load model
    model = tf.keras.models.load_model(model_path)

    # Convert to TFLite
    converter = tf.lite.TFLiteConverter.from_keras_model(model)

    # Optimization settings for microcontrollers
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    converter.representative_dataset = representative_dataset_gen
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    converter.inference_input_type = tf.int8
    converter.inference_output_type = tf.int8

    # Convert
    tflite_model = converter.convert()

    # Save
    with open(output_path, 'wb') as f:
        f.write(tflite_model)

    print(f"Model converted and saved to {output_path}")
    print(f"Model size: {len(tflite_model) / 1024:.2f} KB")

    # Test the model
    interpreter = tf.lite.Interpreter(model_content=tflite_model)
    interpreter.allocate_tensors()

    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()

    print(f"\nInput details:")
    print(f"  Shape: {input_details[0]['shape']}")
    print(f"  Type: {input_details[0]['dtype']}")

    print(f"\nOutput details:")
    print(f"  Shape: {output_details[0]['shape']}")
    print(f"  Type: {output_details[0]['dtype']}")

if __name__ == "__main__":
    convert_to_tflite(
        'models/best_digit_model.h5',
        'models/water_meter_digit_model.tflite'
    )
```

### Step 1.4: Generate Model Header File for WE2

**File: `training_scripts/generate_model_header.py`**
```python
def generate_c_header(tflite_path, output_path):
    """
    Convert TFLite model to C header file
    """
    with open(tflite_path, 'rb') as f:
        tflite_data = f.read()

    header_content = f"""
/* Auto-generated model header file */
#ifndef WATER_METER_DIGIT_MODEL_H_
#define WATER_METER_DIGIT_MODEL_H_

const unsigned int water_meter_digit_model_len = {len(tflite_data)};
const unsigned char water_meter_digit_model[] = {{
"""

    # Write bytes in rows of 12
    for i in range(0, len(tflite_data), 12):
        chunk = tflite_data[i:i+12]
        hex_values = ', '.join(f'0x{b:02x}' for b in chunk)
        header_content += f"  {hex_values},\n"

    header_content += """};

#endif  // WATER_METER_DIGIT_MODEL_H_
"""

    with open(output_path, 'w') as f:
        f.write(header_content)

    print(f"Header file generated: {output_path}")

if __name__ == "__main__":
    generate_c_header(
        'models/water_meter_digit_model.tflite',
        'models/water_meter_digit_model.h'
    )
```

### Step 1.5: Training Workflow

```bash
# Complete training pipeline
cd training_scripts

# 1. Prepare dataset
python3 prepare_dataset.py

# 2. Train model
python3 train_digit_model.py

# 3. Convert to TFLite
python3 convert_to_tflite.py

# 4. Generate C header
python3 generate_model_header.py

# Output: models/water_meter_digit_model.tflite
# Output: models/water_meter_digit_model.h
```

---

## Phase 2: Application Development

### Step 2.1: Create Water Meter Application Directory

```bash
cd EPII_CM55M_APP_S/app/scenario_app
mkdir -p water_meter_amr
cd water_meter_amr
mkdir -p cis_sensor/cis_hm0360
```

### Step 2.2: Application Files

**File: `EPII_CM55M_APP_S/app/scenario_app/water_meter_amr/common_config.h`**
```c
#ifndef WATER_METER_AMR_COMMON_CONFIG_H_
#define WATER_METER_AMR_COMMON_CONFIG_H_

// Application configuration
#define FRAME_CHECK_DEBUG       1
#define EN_ALGO                 1
#define WATCHDOG_VERSION
#define DBG_APP_LOG             1

// Camera settings
#define CAMERA_WIDTH            640
#define CAMERA_HEIGHT           480
#define INPUT_TENSOR_WIDTH      28
#define INPUT_TENSOR_HEIGHT     28

// Model flash address
#define WATER_METER_MODEL_FLASH_ADDR  (BASE_ADDR_FLASH1_R_ALIAS + 0x200000)

// UART communication settings
#define STM32WL_UART_ID         1  // UART1 for STM32WL communication
#define STM32WL_BAUDRATE        115200

// Reading interval (seconds)
#define METER_READ_INTERVAL     60  // Read every 60 seconds

// Number of digits to recognize
#define NUM_DIGITS              8   // Typical water meter has 8 digits

#endif /* WATER_METER_AMR_COMMON_CONFIG_H_ */
```

**File: `EPII_CM55M_APP_S/app/scenario_app/water_meter_amr/water_meter_amr.h`**
```c
#ifndef WATER_METER_AMR_H_
#define WATER_METER_AMR_H_

#include <stdint.h>

// Digit recognition result structure
typedef struct {
    uint8_t digit;          // Recognized digit (0-9)
    float confidence;       // Confidence score (0.0 - 1.0)
    uint16_t x;            // X position in image
    uint16_t y;            // Y position in image
} digit_result_t;

// Water meter reading structure
typedef struct {
    uint8_t digits[NUM_DIGITS];     // All recognized digits
    float avg_confidence;            // Average confidence
    uint32_t timestamp;              // Reading timestamp
    uint8_t valid;                   // Reading validity flag
} meter_reading_t;

// Function prototypes
int water_meter_amr_init(void);
int capture_and_recognize(meter_reading_t *reading);
int send_to_lorawan(meter_reading_t *reading);
void process_meter_reading(void);

#endif /* WATER_METER_AMR_H_ */
```

**File: `EPII_CM55M_APP_S/app/scenario_app/water_meter_amr/water_meter_amr.c`**
```c
#include <stdio.h>
#include <string.h>
#include "WE2_device.h"
#include "water_meter_amr.h"
#include "common_config.h"
#include "hx_drv_uart.h"
#include "hx_drv_timer.h"

// Include generated model header
#include "water_meter_digit_model.h"

// TensorFlow Lite Micro includes
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Global variables
static const tflite::Model* model = nullptr;
static tflite::MicroInterpreter* interpreter = nullptr;
static TfLiteTensor* input_tensor = nullptr;
static TfLiteTensor* output_tensor = nullptr;

// Tensor arena for TFLite
constexpr int kTensorArenaSize = 100 * 1024;  // 100KB
static uint8_t tensor_arena[kTensorArenaSize];

/**
 * Initialize TensorFlow Lite model
 */
int init_tflite_model(void) {
    // Load model
    model = tflite::GetModel(water_meter_digit_model);

    if (model->version() != TFLITE_SCHEMA_VERSION) {
        printf("Model schema version mismatch!\n");
        return -1;
    }

    // Setup operations resolver
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
        printf("AllocateTensors() failed\n");
        return -1;
    }

    // Get input and output tensors
    input_tensor = interpreter->input(0);
    output_tensor = interpreter->output(0);

    printf("Model initialized successfully\n");
    printf("Input: %d bytes, Output: %d bytes\n",
           input_tensor->bytes, output_tensor->bytes);

    return 0;
}

/**
 * Initialize UART for STM32WL communication
 */
int init_stm32wl_uart(void) {
    HX_DRV_UART_CONFIG uart_cfg;

    uart_cfg.uart_baudrate = STM32WL_BAUDRATE;
    uart_cfg.uart_databits = UART_DATA_8BITS;
    uart_cfg.uart_stopbits = UART_STOPBIT_1;
    uart_cfg.uart_parity = UART_PARITY_NONE;
    uart_cfg.uart_flowctrl = UART_FLOWCTRL_NONE;

    if (hx_drv_uart_init(STM32WL_UART_ID, &uart_cfg) != HX_DRV_LIB_PASS) {
        printf("UART init failed\n");
        return -1;
    }

    printf("STM32WL UART initialized\n");
    return 0;
}

/**
 * Initialize water meter AMR system
 */
int water_meter_amr_init(void) {
    printf("Initializing Water Meter AMR System...\n");

    // Initialize TFLite model
    if (init_tflite_model() != 0) {
        return -1;
    }

    // Initialize UART for STM32WL
    if (init_stm32wl_uart() != 0) {
        return -1;
    }

    printf("Water Meter AMR System initialized successfully\n");
    return 0;
}

/**
 * Preprocess image region for digit recognition
 */
void preprocess_digit_region(uint8_t* src, int src_w, int src_h,
                             int x, int y, int w, int h,
                             int8_t* dst) {
    // Extract and resize region to 28x28
    // Convert to grayscale
    // Normalize and quantize to int8

    // Simple implementation - should be optimized
    for (int dy = 0; dy < INPUT_TENSOR_HEIGHT; dy++) {
        for (int dx = 0; dx < INPUT_TENSOR_WIDTH; dx++) {
            int sx = x + (dx * w) / INPUT_TENSOR_WIDTH;
            int sy = y + (dy * h) / INPUT_TENSOR_HEIGHT;

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                int idx = sy * src_w + sx;
                // Convert to int8 range [-128, 127]
                dst[dy * INPUT_TENSOR_WIDTH + dx] = (int8_t)(src[idx] - 128);
            } else {
                dst[dy * INPUT_TENSOR_WIDTH + dx] = -128;
            }
        }
    }
}

/**
 * Recognize single digit using TFLite model
 */
int recognize_digit(int8_t* image_data, digit_result_t* result) {
    // Copy image data to input tensor
    memcpy(input_tensor->data.int8, image_data,
           INPUT_TENSOR_WIDTH * INPUT_TENSOR_HEIGHT);

    // Run inference
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        printf("Invoke failed\n");
        return -1;
    }

    // Get output (10 classes for digits 0-9)
    int8_t* output = output_tensor->data.int8;

    // Find digit with highest confidence
    int max_idx = 0;
    int8_t max_val = output[0];

    for (int i = 1; i < 10; i++) {
        if (output[i] > max_val) {
            max_val = output[i];
            max_idx = i;
        }
    }

    result->digit = max_idx;
    // Convert int8 to confidence (0.0 - 1.0)
    result->confidence = (max_val + 128) / 255.0f;

    return 0;
}

/**
 * Capture image and recognize all digits
 */
int capture_and_recognize(meter_reading_t *reading) {
    // TODO: Implement camera capture
    // TODO: Implement digit segmentation
    // TODO: Recognize each digit

    printf("Capturing and recognizing digits...\n");

    // Placeholder: Simulate digit recognition
    float total_confidence = 0.0f;

    for (int i = 0; i < NUM_DIGITS; i++) {
        digit_result_t digit_res;

        // Extract digit region from image
        // preprocess_digit_region(...);

        // Recognize digit
        // recognize_digit(..., &digit_res);

        // Placeholder values
        digit_res.digit = i % 10;
        digit_res.confidence = 0.95f;

        reading->digits[i] = digit_res.digit;
        total_confidence += digit_res.confidence;

        printf("Digit %d: %d (conf: %.2f)\n",
               i, digit_res.digit, digit_res.confidence);
    }

    reading->avg_confidence = total_confidence / NUM_DIGITS;
    reading->timestamp = hx_drv_timer_get_ms();
    reading->valid = (reading->avg_confidence > 0.8f) ? 1 : 0;

    return 0;
}

/**
 * Send meter reading to STM32WL via UART
 * Format: "READ:12345678,95\r\n" (digits,confidence%)
 */
int send_to_lorawan(meter_reading_t *reading) {
    char buffer[128];
    int len;

    if (!reading->valid) {
        printf("Invalid reading, not sending\n");
        return -1;
    }

    // Format: READ:digits,confidence
    len = sprintf(buffer, "READ:");

    // Add digits
    for (int i = 0; i < NUM_DIGITS; i++) {
        len += sprintf(buffer + len, "%d", reading->digits[i]);
    }

    // Add confidence
    len += sprintf(buffer + len, ",%d\r\n",
                  (int)(reading->avg_confidence * 100));

    // Send via UART
    hx_drv_uart_write(STM32WL_UART_ID, (uint8_t*)buffer, len);

    printf("Sent to LoRaWAN: %s", buffer);

    return 0;
}

/**
 * Main processing function
 */
void process_meter_reading(void) {
    meter_reading_t reading;

    // Capture and recognize
    if (capture_and_recognize(&reading) == 0) {
        // Send to LoRaWAN if valid
        send_to_lorawan(&reading);
    }
}
```

### Step 2.3: Main Application Entry Point

**File: `EPII_CM55M_APP_S/app/scenario_app/water_meter_amr/main.c`**
```c
#include <stdio.h>
#include "WE2_device.h"
#include "water_meter_amr.h"
#include "common_config.h"
#include "hx_drv_timer.h"
#include "hx_drv_pmu.h"

int main(void) {
    printf("\n");
    printf("========================================\n");
    printf("  Water Meter AMR System\n");
    printf("  HIMAX WE2 + STM32WL LoRaWAN\n");
    printf("========================================\n");

    // Initialize system
    if (water_meter_amr_init() != 0) {
        printf("System initialization failed!\n");
        return -1;
    }

    uint32_t last_read_time = 0;

    // Main loop
    while (1) {
        uint32_t current_time = hx_drv_timer_get_ms();

        // Check if it's time to read meter
        if ((current_time - last_read_time) >= (METER_READ_INTERVAL * 1000)) {
            printf("\n--- Taking Meter Reading ---\n");

            // Process meter reading
            process_meter_reading();

            last_read_time = current_time;
        }

        // Small delay to prevent busy loop
        hx_drv_timer_delay_ms(100);
    }

    return 0;
}
```

### Step 2.4: Makefile Configuration

**File: `EPII_CM55M_APP_S/app/scenario_app/water_meter_amr/water_meter_amr.mk`**
```makefile
APPL_DEFINES += -DWATER_METER_AMR

# Application source files
APPL_CSRC += \
    $(APP_PATH)/water_meter_amr.c \
    $(APP_PATH)/main.c

APPL_CXXSRC +=

# Application include paths
APPL_INCLUDES += \
    -I$(APP_PATH) \
    -I$(APP_PATH)/cis_sensor/cis_hm0360

# TensorFlow Lite Micro library
LIB_SEL += tflmtag2209_u55tag2205

# Camera sensor support
CIS_SUPPORT_INAPP = cis_hm0360

# Memory configuration
override LINKER_SCRIPT_FILE := $(SCENARIO_APP_ROOT)/$(APP_TYPE)/water_meter_amr.ld
```

**File: `EPII_CM55M_APP_S/app/scenario_app/water_meter_amr/drv_user_defined.mk`**
```makefile
# Include driver configuration
include $(DRIVERS)/mk_cfg/drv_user_defined.mk

# UART configuration
APPL_DEFINES += -DUSE_UART1
```

---

## Phase 3: LoRaWAN Communication Integration

### Step 3.1: STM32WL Firmware Overview

The STM32WL side needs to:
1. Receive meter readings via UART from WE2
2. Parse the reading data
3. Send data via LoRaWAN using AT commands or LoRaWAN stack

**Communication Protocol:**
```
WE2 → STM32WL: "READ:12345678,95\r\n"
                     ^^^^^^^^ ^^
                     digits   confidence%

STM32WL → WE2: "OK\r\n" or "ERROR\r\n"
```

### Step 3.2: STM32WL AT Command Integration

If using an STM32WL module with AT command firmware (e.g., I-CUBE-LRWAN):

**Common AT Commands:**
```
// Join LoRaWAN network
AT+JOIN

// Send unconfirmed uplink
AT+SEND=<port>:<payload_hex>

// Send confirmed uplink
AT+SENDC=<port>:<payload_hex>

// Examples:
AT+SEND=2:313233343536373838  // Port 2, data "12345678"
```

**WE2 Code to Send AT Commands:**
```c
int send_lorawan_at_command(const char* reading_str) {
    char at_cmd[256];
    char hex_payload[32];
    int len = 0;

    // Convert reading to hex
    for (int i = 0; reading_str[i] != '\0' && reading_str[i] != ','; i++) {
        len += sprintf(hex_payload + len, "%02X", reading_str[i]);
    }

    // Build AT command
    sprintf(at_cmd, "AT+SEND=2:%s\r\n", hex_payload);

    // Send via UART
    hx_drv_uart_write(STM32WL_UART_ID, (uint8_t*)at_cmd, strlen(at_cmd));

    // Wait for response
    uint8_t response[128];
    int rx_len = hx_drv_uart_read(STM32WL_UART_ID, response, sizeof(response), 5000);

    if (rx_len > 0) {
        response[rx_len] = '\0';
        printf("STM32WL response: %s\n", response);

        if (strstr((char*)response, "OK")) {
            return 0;
        }
    }

    return -1;
}
```

### Step 3.3: Data Payload Format

**Optimized LoRaWAN Payload:**
```c
typedef struct __attribute__((packed)) {
    uint32_t device_id;     // 4 bytes: Device identifier
    uint32_t timestamp;     // 4 bytes: Unix timestamp
    uint32_t reading;       // 4 bytes: Meter reading (e.g., 12345678)
    uint8_t confidence;     // 1 byte: Confidence (0-100)
    uint8_t battery;        // 1 byte: Battery level (0-100)
} lorawan_payload_t;
// Total: 14 bytes
```

**Encoding Function:**
```c
int encode_lorawan_payload(meter_reading_t *reading, uint8_t* payload) {
    lorawan_payload_t *pkt = (lorawan_payload_t*)payload;

    // Device ID (configure per device)
    pkt->device_id = 0x12345678;

    // Timestamp
    pkt->timestamp = reading->timestamp;

    // Convert digit array to integer
    pkt->reading = 0;
    for (int i = 0; i < NUM_DIGITS; i++) {
        pkt->reading = pkt->reading * 10 + reading->digits[i];
    }

    // Confidence
    pkt->confidence = (uint8_t)(reading->avg_confidence * 100);

    // Battery level (read from PMU)
    pkt->battery = 100;  // TODO: Implement battery reading

    return sizeof(lorawan_payload_t);
}
```

### Step 3.4: LoRaWAN Network Configuration

**Network Parameters (configure in STM32WL):**
```c
// LoRaWAN OTAA (Over-The-Air Activation)
DevEUI:  0x0123456789ABCDEF  // Unique device identifier
AppEUI:  0xFEDCBA9876543210  // Application identifier
AppKey:  0x2B7E151628AED2A6ABF7158809CF4F3C  // Application key

// LoRaWAN ABP (Activation By Personalization)
DevAddr: 0x26011234
NwkSKey: 0x2B7E151628AED2A6ABF7158809CF4F3C
AppSKey: 0x2B7E151628AED2A6ABF7158809CF4F3C

// Region
Region: EU868 / US915 / AS923 / etc.

// Class
Class: A (default for battery-powered devices)
```

---

## Phase 4: AMR Provisioning Method

### Step 4.1: Device Provisioning Overview

**Provisioning Steps:**
1. **Device Registration**: Register device in LoRaWAN network server
2. **Credential Configuration**: Program DevEUI, AppEUI, AppKey
3. **Calibration**: Calibrate camera for specific meter type
4. **Testing**: Verify end-to-end communication

### Step 4.2: Provisioning via UART

**Provisioning Commands:**
```c
// Command format: "CMD:<command>:<params>\r\n"

// Set Device EUI
CMD:SET_DEVEUI:0123456789ABCDEF

// Set App EUI
CMD:SET_APPEUI:FEDCBA9876543210

// Set App Key
CMD:SET_APPKEY:2B7E151628AED2A6ABF7158809CF4F3C

// Set Device ID
CMD:SET_DEVID:12345678

// Calibrate camera
CMD:CALIBRATE

// Test reading
CMD:TEST_READ

// Join network
CMD:JOIN

// Get status
CMD:STATUS
```

**Implementation:**
```c
void process_provision_command(char* cmd) {
    if (strncmp(cmd, "CMD:SET_DEVEUI:", 15) == 0) {
        // Parse and save DevEUI
        char* deveui = cmd + 15;
        save_deveui(deveui);
        send_uart_response("OK\r\n");
    }
    else if (strncmp(cmd, "CMD:SET_APPEUI:", 15) == 0) {
        char* appeui = cmd + 15;
        save_appeui(appeui);
        send_uart_response("OK\r\n");
    }
    else if (strncmp(cmd, "CMD:CALIBRATE", 13) == 0) {
        calibrate_camera();
        send_uart_response("CALIBRATED\r\n");
    }
    else if (strncmp(cmd, "CMD:TEST_READ", 13) == 0) {
        meter_reading_t reading;
        capture_and_recognize(&reading);

        char response[128];
        sprintf(response, "TEST:");
        for (int i = 0; i < NUM_DIGITS; i++) {
            sprintf(response + strlen(response), "%d", reading.digits[i]);
        }
        sprintf(response + strlen(response), ",%.0f%%\r\n",
                reading.avg_confidence * 100);
        send_uart_response(response);
    }
    else if (strncmp(cmd, "CMD:STATUS", 10) == 0) {
        send_status();
    }
}
```

### Step 4.3: Configuration Storage

```c
#include "hx_drv_scu.h"
#include "hx_drv_flash.h"

#define CONFIG_FLASH_ADDR   0x001F0000  // Last 64KB of flash

typedef struct {
    uint32_t magic;         // 0x414D5247 "AMRG"
    uint32_t device_id;
    uint8_t deveui[8];
    uint8_t appeui[8];
    uint8_t appkey[16];
    uint8_t region;
    uint8_t calibrated;
    uint32_t crc;
} amr_config_t;

int save_config(amr_config_t* config) {
    config->magic = 0x414D5247;
    // TODO: Calculate CRC

    // Erase flash sector
    hx_drv_flash_erase_sector(CONFIG_FLASH_ADDR);

    // Write config
    hx_drv_flash_write(CONFIG_FLASH_ADDR, (uint8_t*)config, sizeof(amr_config_t));

    return 0;
}

int load_config(amr_config_t* config) {
    hx_drv_flash_read(CONFIG_FLASH_ADDR, (uint8_t*)config, sizeof(amr_config_t));

    if (config->magic != 0x414D5247) {
        printf("No valid config found\n");
        return -1;
    }

    // TODO: Verify CRC

    return 0;
}
```

### Step 4.4: Provisioning Tool (Python)

**File: `tools/provision_device.py`**
```python
#!/usr/bin/env python3
import serial
import time
import sys

class AMRProvisioner:
    def __init__(self, port, baudrate=921600):
        self.ser = serial.Serial(port, baudrate, timeout=2)
        time.sleep(0.5)

    def send_command(self, cmd):
        """Send command and wait for response"""
        self.ser.write(f"{cmd}\r\n".encode())
        time.sleep(0.2)
        response = self.ser.readline().decode().strip()
        return response

    def provision(self, device_id, deveui, appeui, appkey):
        """Provision a new AMR device"""
        print(f"Provisioning device {device_id}...")

        # Set device ID
        resp = self.send_command(f"CMD:SET_DEVID:{device_id}")
        print(f"  Set Device ID: {resp}")

        # Set DevEUI
        resp = self.send_command(f"CMD:SET_DEVEUI:{deveui}")
        print(f"  Set DevEUI: {resp}")

        # Set AppEUI
        resp = self.send_command(f"CMD:SET_APPEUI:{appeui}")
        print(f"  Set AppEUI: {resp}")

        # Set AppKey
        resp = self.send_command(f"CMD:SET_APPKEY:{appkey}")
        print(f"  Set AppKey: {resp}")

        print("Provisioning complete!")

    def test_reading(self):
        """Test meter reading"""
        print("Testing meter reading...")
        resp = self.send_command("CMD:TEST_READ")
        print(f"  Result: {resp}")

    def calibrate(self):
        """Calibrate camera"""
        print("Calibrating camera...")
        resp = self.send_command("CMD:CALIBRATE")
        print(f"  Result: {resp}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 provision_device.py <serial_port>")
        print("Example: python3 provision_device.py /dev/ttyACM0")
        sys.exit(1)

    prov = AMRProvisioner(sys.argv[1])

    # Example provisioning
    prov.provision(
        device_id="12345678",
        deveui="0123456789ABCDEF",
        appeui="FEDCBA9876543210",
        appkey="2B7E151628AED2A6ABF7158809CF4F3C"
    )

    # Calibrate
    prov.calibrate()

    # Test reading
    prov.test_reading()
```

---

## Phase 5: Build and Flash Firmware

### Step 5.1: Build Configuration

**Edit: `EPII_CM55M_APP_S/makefile`**
```makefile
# Set application type to water_meter_amr
APP_TYPE = water_meter_amr

# Enable TensorFlow Lite Micro
LIB_TFLM_ENABLE = 1

# Camera sensor
CIS_SEL = HM0360_MONO

# Build options
TOOLCHAIN = gnu
DEBUG = 1
```

### Step 5.2: Build Script

**File: `build_amr.sh`**
```bash
#!/bin/bash

set -e

echo "========================================"
echo "Building Water Meter AMR Firmware"
echo "========================================"

# Check toolchain
if ! command -v arm-none-eabi-gcc &> /dev/null; then
    echo "ERROR: ARM GCC toolchain not found!"
    echo "Please install: arm-gnu-toolchain-13.2.Rel1"
    exit 1
fi

# Clean previous build
cd EPII_CM55M_APP_S
make clean

# Build firmware
echo "Compiling firmware..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "✓ Build successful!"

    # Copy model header
    if [ -f "../models/water_meter_digit_model.h" ]; then
        echo "Copying model header..."
        cp ../models/water_meter_digit_model.h \
           app/scenario_app/water_meter_amr/
    fi

    # Generate image
    echo "Generating firmware image..."
    cd ../we2_image_gen_local/

    cp ../EPII_CM55M_APP_S/obj_epii_evb_icv30_bdv10/gnu_epii_evb_WLCSP65/EPII_CM55M_gnu_epii_evb_WLCSP65_s.elf input_case1_secboot/

    ./we2_local_image_gen project_case1_blp_wlcsp.json

    if [ -f "output_case1_sec_wlcsp/output.img" ]; then
        echo "✓ Firmware image generated!"
        cp output_case1_sec_wlcsp/output.img ../water_meter_amr.img
        echo ""
        echo "Output: water_meter_amr.img"
        ls -lh ../water_meter_amr.img
    else
        echo "ERROR: Failed to generate image"
        exit 1
    fi
else
    echo "ERROR: Build failed!"
    exit 1
fi

echo ""
echo "========================================"
echo "Build Complete!"
echo "========================================"
```

### Step 5.3: Flash Script

**File: `flash_amr.sh`**
```bash
#!/bin/bash

set -e

if [ -z "$1" ]; then
    echo "Usage: ./flash_amr.sh <serial_port> [model_path]"
    echo "Example: ./flash_amr.sh /dev/ttyACM0"
    echo "Example: ./flash_amr.sh /dev/ttyACM0 models/water_meter_digit_model.tflite"
    exit 1
fi

PORT=$1
MODEL_PATH=${2:-"models/water_meter_digit_model.tflite"}
IMAGE="water_meter_amr.img"

if [ ! -f "$IMAGE" ]; then
    echo "ERROR: Firmware image not found: $IMAGE"
    echo "Please build first: ./build_amr.sh"
    exit 1
fi

echo "========================================"
echo "Flashing Water Meter AMR Firmware"
echo "========================================"
echo "Port: $PORT"
echo "Image: $IMAGE"
echo "Model: $MODEL_PATH"
echo ""

# Check if model exists
if [ ! -f "$MODEL_PATH" ]; then
    echo "WARNING: Model file not found: $MODEL_PATH"
    echo "Flashing firmware only..."

    python3 xmodem/xmodem_send.py \
        --port=$PORT \
        --baudrate=921600 \
        --protocol=xmodem \
        --file=$IMAGE
else
    # Flash firmware and model
    python3 xmodem/xmodem_send.py \
        --port=$PORT \
        --baudrate=921600 \
        --protocol=xmodem \
        --file=$IMAGE \
        --model="$MODEL_PATH 0x200000 0x00000"
fi

echo ""
echo "========================================"
echo "Flashing Complete!"
echo "Please press RESET button on device"
echo "========================================"
```

### Step 5.4: Complete Build and Flash Workflow

```bash
# 1. Train model (if not already done)
cd training_scripts
python3 train_digit_model.py
python3 convert_to_tflite.py
python3 generate_model_header.py
cd ..

# 2. Build firmware
chmod +x build_amr.sh
./build_amr.sh

# 3. Flash firmware
chmod +x flash_amr.sh
./flash_amr.sh /dev/ttyACM0 models/water_meter_digit_model.tflite

# 4. Press RESET button on WE2 module

# 5. Provision device
python3 tools/provision_device.py /dev/ttyACM0
```

---

## Phase 6: Testing and Deployment

### Step 6.1: Unit Testing

**Test Checklist:**
- [ ] Camera capture working
- [ ] Digit recognition accuracy > 95%
- [ ] UART communication with STM32WL
- [ ] LoRaWAN join successful
- [ ] Data transmission to cloud
- [ ] Power consumption acceptable
- [ ] Reading interval correct

### Step 6.2: Integration Testing

```bash
# Monitor UART output
minicom -D /dev/ttyACM0 -b 921600

# Or use screen
screen /dev/ttyACM0 921600

# Expected output:
# ========================================
#   Water Meter AMR System
#   HIMAX WE2 + STM32WL LoRaWAN
# ========================================
# Model initialized successfully
# STM32WL UART initialized
#
# --- Taking Meter Reading ---
# Capturing and recognizing digits...
# Digit 0: 1 (conf: 0.95)
# Digit 1: 2 (conf: 0.96)
# ...
# Sent to LoRaWAN: READ:12345678,95
```

### Step 6.3: Field Testing Script

**File: `tools/field_test.py`**
```python
#!/usr/bin/env python3
import serial
import time
import csv
from datetime import datetime

class AMRTester:
    def __init__(self, port, baudrate=921600):
        self.ser = serial.Serial(port, baudrate, timeout=1)
        self.results = []

    def monitor(self, duration_minutes=60):
        """Monitor device for specified duration"""
        print(f"Monitoring device for {duration_minutes} minutes...")

        start_time = time.time()
        end_time = start_time + (duration_minutes * 60)

        while time.time() < end_time:
            line = self.ser.readline().decode('utf-8', errors='ignore').strip()

            if line:
                timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                print(f"[{timestamp}] {line}")

                # Parse reading results
                if line.startswith("READ:"):
                    parts = line[5:].split(',')
                    if len(parts) == 2:
                        reading = parts[0]
                        confidence = parts[1]

                        self.results.append({
                            'timestamp': timestamp,
                            'reading': reading,
                            'confidence': confidence
                        })

        self.save_results()

    def save_results(self):
        """Save results to CSV"""
        filename = f"field_test_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"

        with open(filename, 'w', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=['timestamp', 'reading', 'confidence'])
            writer.writeheader()
            writer.writerows(self.results)

        print(f"\nResults saved to: {filename}")
        print(f"Total readings: {len(self.results)}")

if __name__ == "__main__":
    import sys

    if len(sys.argv) < 2:
        print("Usage: python3 field_test.py <serial_port> [duration_minutes]")
        sys.exit(1)

    port = sys.argv[1]
    duration = int(sys.argv[2]) if len(sys.argv) > 2 else 60

    tester = AMRTester(port)
    tester.monitor(duration)
```

### Step 6.4: Cloud Integration

**LoRaWAN Network Server Integration:**

1. **The Things Network (TTN):**
   - Create application
   - Register device with DevEUI, AppEUI, AppKey
   - Set up HTTP integration or MQTT

2. **Payload Decoder (JavaScript for TTN):**
```javascript
function decodeUplink(input) {
    var data = {};

    // Parse 14-byte payload
    if (input.bytes.length >= 14) {
        data.device_id = (input.bytes[0] << 24) | (input.bytes[1] << 16) |
                         (input.bytes[2] << 8) | input.bytes[3];

        data.timestamp = (input.bytes[4] << 24) | (input.bytes[5] << 16) |
                         (input.bytes[6] << 8) | input.bytes[7];

        data.reading = (input.bytes[8] << 24) | (input.bytes[9] << 16) |
                       (input.bytes[10] << 8) | input.bytes[11];

        data.confidence = input.bytes[12];
        data.battery = input.bytes[13];
    }

    return {
        data: data,
        warnings: [],
        errors: []
    };
}
```

3. **Cloud Platform Integration:**
   - AWS IoT Core
   - Azure IoT Hub
   - Google Cloud IoT
   - Custom MQTT broker

---

## Troubleshooting

### Common Issues and Solutions

#### 1. Build Errors

**Problem:** Compilation fails with undefined references
```
Solution:
- Check that TFLite library is enabled in makefile
- Verify all source files are in .mk file
- Ensure model header is generated correctly
```

#### 2. Model Loading Fails

**Problem:** "Model schema version mismatch"
```
Solution:
- Regenerate model with compatible TFLite version
- Check TFLite Micro library version
- Verify model was quantized correctly
```

#### 3. UART Communication Issues

**Problem:** No response from STM32WL
```
Solution:
- Check TX/RX connections (TX→RX, RX→TX)
- Verify baud rate matches on both sides
- Check ground connection
- Test with loopback (TX→RX on same device)
```

#### 4. Low Recognition Accuracy

**Problem:** Digit recognition < 90% accurate
```
Solution:
- Retrain model with more data
- Add more augmentation
- Improve lighting/camera positioning
- Calibrate camera for specific meter type
- Adjust preprocessing parameters
```

#### 5. LoRaWAN Join Fails

**Problem:** Device cannot join network
```
Solution:
- Verify DevEUI, AppEUI, AppKey are correct
- Check region configuration (EU868/US915/etc.)
- Ensure gateway is in range and online
- Check duty cycle restrictions
- Verify network server configuration
```

#### 6. Flash Memory Issues

**Problem:** Model won't fit in flash
```
Solution:
- Reduce model size (fewer layers/filters)
- Use higher quantization
- Move model to external flash if available
- Compress model
```

---

## Appendix

### A. File Structure
```
camera-watcam/
├── EPII_CM55M_APP_S/
│   └── app/scenario_app/water_meter_amr/
│       ├── water_meter_amr.c
│       ├── water_meter_amr.h
│       ├── main.c
│       ├── common_config.h
│       ├── water_meter_amr.mk
│       └── drv_user_defined.mk
├── training_scripts/
│   ├── prepare_dataset.py
│   ├── train_digit_model.py
│   ├── convert_to_tflite.py
│   └── generate_model_header.py
├── tools/
│   ├── provision_device.py
│   └── field_test.py
├── models/
│   ├── water_meter_digit_model.tflite
│   └── water_meter_digit_model.h
├── dataset/
│   └── water_meter/
├── build_amr.sh
├── flash_amr.sh
└── AMR_SOLUTION_GUIDE.md (this file)
```

### B. Resources

- **HIMAX WE2 SDK:** https://github.com/HimaxWiseEyePlus/HIMAX_WE2_Rabboni
- **TensorFlow Lite Micro:** https://www.tensorflow.org/lite/microcontrollers
- **STM32WL Documentation:** https://www.st.com/en/microcontrollers-microprocessors/stm32wl-series.html
- **LoRaWAN Specification:** https://lora-alliance.org/resource_hub/lorawan-specification-v1-1/
- **The Things Network:** https://www.thethingsnetwork.org/

### C. Performance Optimization

**Tips for optimizing the AMR solution:**

1. **Model Optimization:**
   - Use post-training quantization (INT8)
   - Prune unnecessary layers
   - Use depthwise separable convolutions
   - Consider MobileNet architecture

2. **Power Optimization:**
   - Use deep sleep between readings
   - Optimize camera capture time
   - Reduce UART baud rate if possible
   - Use confirmed uplinks sparingly

3. **Accuracy Improvement:**
   - Collect domain-specific training data
   - Implement ensemble methods
   - Add temporal consistency checks
   - Use sequence validation (digits should increment)

4. **LoRaWAN Optimization:**
   - Use adaptive data rate (ADR)
   - Optimize spreading factor
   - Batch multiple readings if possible
   - Use confirmed uplinks only when critical

---

## Summary

This guide provides a complete end-to-end solution for building an Automatic Meter Reading (AMR) system using:
- **HIMAX WE2** for vision processing and digit recognition
- **TensorFlow Lite Micro** for on-device AI inference
- **STM32WL** for LoRaWAN connectivity
- **LoRaWAN** for long-range, low-power data transmission

Follow each phase sequentially to build, deploy, and maintain your AMR solution.

**Next Steps:**
1. Collect and prepare your water meter digit dataset
2. Train and optimize the recognition model
3. Build and flash the firmware
4. Provision devices with LoRaWAN credentials
5. Deploy and monitor in the field

Good luck with your AMR project!
