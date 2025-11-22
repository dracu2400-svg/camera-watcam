#!/bin/bash

##
# Build script for Training Mode Firmware
##

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

# Configuration
APP_NAME="training_mode"
SDK_PATH="${SDK_PATH:-/path/to/HIMAX_WE2_SDK}"
BUILD_DIR="$SDK_PATH/EPII_CM55M_APP_S"
APP_DIR="$BUILD_DIR/app/scenario_app/$APP_NAME"

echo "========================================"
echo "  Building Training Mode Firmware"
echo "========================================"
echo ""
echo "SDK Path: $SDK_PATH"
echo "App Name: $APP_NAME"
echo ""

# Check if SDK path exists
if [ ! -d "$SDK_PATH" ]; then
    echo "ERROR: SDK path not found: $SDK_PATH"
    echo ""
    echo "Please set SDK_PATH environment variable:"
    echo "  export SDK_PATH=/path/to/HIMAX_WE2_SDK"
    echo ""
    exit 1
fi

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "ERROR: Build directory not found: $BUILD_DIR"
    exit 1
fi

# Copy firmware files to SDK
echo "Copying firmware files to SDK..."
rm -rf "$APP_DIR"
mkdir -p "$APP_DIR"

cp "$SCRIPT_DIR/training_mode.c" "$APP_DIR/"
cp "$SCRIPT_DIR/training_mode.mk" "$APP_DIR/"

echo "✓ Files copied"
echo ""

# Navigate to build directory
cd "$BUILD_DIR"

# Clean previous build
echo "Cleaning previous build..."
make clean APP=$APP_NAME

# Build firmware
echo ""
echo "Building firmware..."
make APP=$APP_NAME -j$(nproc)

echo ""
echo "========================================"
echo "✓ Build complete!"
echo "========================================"
echo ""

# Show size
OUTPUT_DIR="output_${APP_NAME}_gnu_epii_CM55M_s"
if [ -f "$OUTPUT_DIR/output.elf" ]; then
    echo "Binary size:"
    arm-none-eabi-size "$OUTPUT_DIR/output.elf"
    echo ""
    echo "Output files:"
    ls -lh "$OUTPUT_DIR"/output.* 2>/dev/null || true
    echo ""
    echo "Flash command:"
    echo "  $PROJECT_ROOT/tools/training-gui/firmware/flash_training_firmware.sh /dev/ttyACM0"
else
    echo "ERROR: Build failed - output.elf not found"
    exit 1
fi
