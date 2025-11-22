#!/bin/bash

###############################################################################
# Complete AMR Workflow Automation Script
#
# This script automates the entire workflow from training to deployment
#
# Usage:
#   ./complete_workflow.sh [--skip-training] [--skip-flash]
#
# Steps:
#   1. Check environment
#   2. Build training firmware (optional)
#   3. Train model (or use existing)
#   4. Export model
#   5. Integrate into firmware
#   6. Build production firmware
#   7. Flash to device (optional)
#   8. Verify deployment
#
###############################################################################

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
SKIP_TRAINING=0
SKIP_FLASH=0
SERIAL_PORT=${SERIAL_PORT:-/dev/ttyACM0}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --skip-training)
            SKIP_TRAINING=1
            shift
            ;;
        --skip-flash)
            SKIP_FLASH=1
            shift
            ;;
        --port)
            SERIAL_PORT="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--skip-training] [--skip-flash] [--port PORT]"
            exit 1
            ;;
    esac
done

# Functions
print_header() {
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}  $1${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
}

print_step() {
    echo -e "${GREEN}✓${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

check_command() {
    if ! command -v $1 &> /dev/null; then
        print_error "$1 is not installed"
        return 1
    else
        print_step "$1 is installed"
        return 0
    fi
}

# Start workflow
print_header "Complete AMR Workflow Automation"

START_TIME=$(date +%s)

##
## STEP 1: Environment Check
##

print_header "Step 1: Environment Check"

print_step "Checking required tools..."

ALL_TOOLS_OK=1

check_command python3 || ALL_TOOLS_OK=0
check_command arm-none-eabi-gcc || ALL_TOOLS_OK=0
check_command make || ALL_TOOLS_OK=0

if [ $ALL_TOOLS_OK -eq 0 ]; then
    print_error "Missing required tools"
    exit 1
fi

# Check environment variables
if [ -z "$SDK_PATH" ]; then
    print_error "SDK_PATH environment variable not set"
    echo "Please set SDK_PATH to your HIMAX WE2 SDK directory"
    echo "  export SDK_PATH=~/development/Seeed_Grove_Vision_AI_Module_V2"
    exit 1
fi

print_step "SDK_PATH: $SDK_PATH"

# Check if SDK exists
if [ ! -d "$SDK_PATH" ]; then
    print_error "SDK directory not found: $SDK_PATH"
    exit 1
fi

print_step "SDK directory found"

# Get project root (3 levels up from this script)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

print_step "Project root: $PROJECT_ROOT"

##
## STEP 2: Build Training Firmware (Optional)
##

if [ $SKIP_TRAINING -eq 0 ]; then
    print_header "Step 2: Build Training Firmware"

    cd "$PROJECT_ROOT/tools/training-gui/firmware"

    if [ ! -f "build_training_firmware.sh" ]; then
        print_error "build_training_firmware.sh not found"
        exit 1
    fi

    ./build_training_firmware.sh

    print_step "Training firmware built"

    print_warning "Please flash training firmware and collect images using GUI"
    print_warning "Then run this script again with --skip-training"
    echo ""
    echo "To flash training firmware:"
    echo "  ./flash_training_firmware.sh $SERIAL_PORT"
    echo ""
    echo "To launch training GUI:"
    echo "  cd $PROJECT_ROOT/tools/training-gui"
    echo "  python3 src/training_gui.py --port $SERIAL_PORT"
    echo ""
    echo "After collecting images and training model, run:"
    echo "  $0 --skip-training"
    echo ""
    exit 0
fi

##
## STEP 3: Check for Trained Model
##

print_header "Step 3: Check for Trained Model"

TFLITE_MODEL="$PROJECT_ROOT/tools/training-gui/data/models/digit_model.tflite"

if [ ! -f "$TFLITE_MODEL" ]; then
    print_error "TFLite model not found: $TFLITE_MODEL"
    echo ""
    echo "Please train model using GUI first:"
    echo "  cd $PROJECT_ROOT/tools/training-gui"
    echo "  python3 src/training_gui.py --port $SERIAL_PORT"
    echo ""
    echo "Steps:"
    echo "  1. Collect 1000+ images"
    echo "  2. Click 'Train Model'"
    echo "  3. Click 'Export TFLite'"
    echo ""
    exit 1
fi

print_step "TFLite model found"

# Check model size
MODEL_SIZE=$(du -h "$TFLITE_MODEL" | cut -f1)
print_step "Model size: $MODEL_SIZE"

##
## STEP 4: Generate C Header
##

print_header "Step 4: Generate C Header"

cd "$PROJECT_ROOT/tools/training"

C_HEADER="$PROJECT_ROOT/models/water_meter_digit_model.h"

python3 generate_model_header.py \
    --input ../training-gui/data/models/digit_model.tflite \
    --output ../../models/water_meter_digit_model.h \
    --array-name digit_recognition_model

if [ ! -f "$C_HEADER" ]; then
    print_error "Failed to generate C header"
    exit 1
fi

print_step "C header generated: $C_HEADER"

##
## STEP 5: Copy Model to Firmware
##

print_header "Step 5: Copy Model to Firmware"

FIRMWARE_DIR="$SDK_PATH/EPII_CM55M_APP_S/app/scenario_app/water_meter_amr"

# Create firmware directory if it doesn't exist
if [ ! -d "$FIRMWARE_DIR" ]; then
    print_warning "Firmware directory not found, copying template..."

    # Copy template
    if [ -d "$PROJECT_ROOT/firmware-templates/complete-amr-app" ]; then
        cp -r "$PROJECT_ROOT/firmware-templates/complete-amr-app" "$FIRMWARE_DIR"
        print_step "Firmware template copied"
    else
        print_error "Firmware template not found"
        exit 1
    fi
fi

# Create models directory
mkdir -p "$FIRMWARE_DIR/models"

# Copy model header
cp "$C_HEADER" "$FIRMWARE_DIR/models/"

print_step "Model copied to firmware: $FIRMWARE_DIR/models/water_meter_digit_model.h"

##
## STEP 6: Build Production Firmware
##

print_header "Step 6: Build Production Firmware"

cd "$SDK_PATH/EPII_CM55M_APP_S"

# Clean
print_step "Cleaning previous build..."
make clean APP=water_meter_amr 2>&1 | tail -5

# Build
print_step "Building firmware..."
make APP=water_meter_amr -j$(nproc)

# Check build result
OUTPUT_ELF="output_water_meter_amr_gnu_epii_CM55M_s/output.elf"
OUTPUT_BIN="output_water_meter_amr_gnu_epii_CM55M_s/output.bin"

if [ ! -f "$OUTPUT_ELF" ]; then
    print_error "Build failed - output.elf not found"
    exit 1
fi

print_step "Firmware built successfully"

# Show size
echo ""
echo "Firmware size:"
arm-none-eabi-size "$OUTPUT_ELF"
echo ""

# Check for model symbol
if arm-none-eabi-nm "$OUTPUT_ELF" | grep -q "digit_recognition_model"; then
    print_step "Model successfully embedded in firmware"
else
    print_error "Model not found in firmware binary"
    exit 1
fi

##
## STEP 7: Flash to Device (Optional)
##

if [ $SKIP_FLASH -eq 0 ]; then
    print_header "Step 7: Flash to Device"

    # Check if serial port exists
    if [ ! -e "$SERIAL_PORT" ]; then
        print_warning "Serial port not found: $SERIAL_PORT"
        print_warning "Skipping flash step"
        SKIP_FLASH=1
    else
        print_step "Serial port found: $SERIAL_PORT"

        cd "$PROJECT_ROOT/scripts/flash"

        if [ ! -f "flash_amr.sh" ]; then
            print_error "flash_amr.sh not found"
            exit 1
        fi

        echo ""
        print_warning "Please put device in bootloader mode:"
        echo "  1. Press and HOLD BOOT button"
        echo "  2. Press and release RESET button"
        echo "  3. Release BOOT button"
        echo ""
        read -p "Press Enter when ready to flash..."

        ./flash_amr.sh "$SERIAL_PORT"

        print_step "Firmware flashed"
    fi
fi

##
## STEP 8: Summary
##

print_header "Workflow Complete!"

END_TIME=$(date +%s)
DURATION=$((END_TIME - START_TIME))
MINUTES=$((DURATION / 60))
SECONDS=$((DURATION % 60))

echo ""
echo "Summary:"
echo "--------"
print_step "Model trained and exported"
print_step "C header generated ($C_HEADER)"
print_step "Model integrated into firmware"
print_step "Firmware built successfully"

if [ $SKIP_FLASH -eq 0 ]; then
    print_step "Firmware flashed to device"
else
    echo ""
    echo "To flash firmware manually:"
    echo "  cd $PROJECT_ROOT/scripts/flash"
    echo "  ./flash_amr.sh $SERIAL_PORT"
fi

echo ""
echo "Total time: ${MINUTES}m ${SECONDS}s"
echo ""

print_header "Next Steps"

echo "1. Monitor device:"
echo "   screen $SERIAL_PORT 921600"
echo ""
echo "2. Verify system is working:"
echo "   - Check for 'AMR System Ready' message"
echo "   - Watch for inference results"
echo "   - Verify LoRaWAN join success"
echo ""
echo "3. Test with water meter:"
echo "   - Position camera at meter"
echo "   - Press RESET to trigger reading"
echo "   - Verify accuracy"
echo ""
echo "4. Deploy to production:"
echo "   - Provision devices (see PRODUCTION_PIPELINE.md)"
echo "   - Install in field"
echo "   - Monitor via cloud dashboard"
echo ""

print_step "Workflow automation complete!"

exit 0
