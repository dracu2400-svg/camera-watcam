#!/bin/bash
#
# Flash Water Meter AMR Firmware to HIMAX WE2
# Supports flashing firmware with or without model
#

set -e

# Check arguments
if [ -z "$1" ]; then
    echo "Usage: ./flash_amr.sh <serial_port> [model_path]"
    echo ""
    echo "Examples:"
    echo "  ./flash_amr.sh /dev/ttyACM0"
    echo "  ./flash_amr.sh /dev/ttyACM0 models/water_meter_digit_model.tflite"
    echo "  ./flash_amr.sh COM3  (Windows)"
    echo ""
    echo "Available serial ports:"
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null || echo "  No devices found"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        ls /dev/tty.usb* /dev/cu.usb* 2>/dev/null || echo "  No devices found"
    fi
    exit 1
fi

PORT=$1
MODEL_PATH=${2:-""}
IMAGE="water_meter_amr.img"

echo "========================================"
echo "Flashing Water Meter AMR Firmware"
echo "========================================"

# Check if firmware image exists
if [ ! -f "$IMAGE" ]; then
    echo "ERROR: Firmware image not found: $IMAGE"
    echo ""
    echo "Please build firmware first:"
    echo "  ./build_amr.sh"
    exit 1
fi

echo "✓ Firmware image found"
echo ""
echo "Configuration:"
echo "  Port:     $PORT"
echo "  Firmware: $IMAGE ($(du -h $IMAGE | cut -f1))"

# Check Python dependencies
if ! python3 -c "import serial" 2>/dev/null; then
    echo ""
    echo "ERROR: Python serial module not found"
    echo "Please install: pip install -r xmodem/requirements.txt"
    exit 1
fi

# Build flash command
FLASH_CMD="python3 xmodem/xmodem_send.py --port=$PORT --baudrate=921600 --protocol=xmodem --file=$IMAGE"

# Check if model should be flashed
if [ -n "$MODEL_PATH" ]; then
    if [ -f "$MODEL_PATH" ]; then
        echo "  Model:    $MODEL_PATH ($(du -h $MODEL_PATH | cut -f1))"
        FLASH_CMD="$FLASH_CMD --model=\"$MODEL_PATH 0x200000 0x00000\""
    else
        echo ""
        echo "WARNING: Model file not found: $MODEL_PATH"
        echo "Flashing firmware only (without model)"
    fi
else
    echo "  Model:    None (firmware only)"
fi

echo ""
echo "========================================"
echo "Starting flash process..."
echo "========================================"
echo ""
echo "IMPORTANT:"
echo "  1. Make sure device is connected to $PORT"
echo "  2. The script will reset the device automatically"
echo "  3. Wait for flashing to complete"
echo ""

# Set permissions on Linux
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    if [ -e "$PORT" ]; then
        echo "Setting permissions on $PORT..."
        sudo chmod 666 $PORT 2>/dev/null || true
    fi
fi

# Execute flash command
eval $FLASH_CMD

if [ $? -eq 0 ]; then
    echo ""
    echo "========================================"
    echo "Flashing Complete!"
    echo "========================================"
    echo ""
    echo "Next steps:"
    echo "  1. Press RESET button on the device"
    echo "  2. Monitor output: screen $PORT 921600"
    echo "     (or use minicom/TeraTerm)"
    echo "  3. You should see: 'Water Meter AMR System'"
    echo ""
    echo "To provision device:"
    echo "  python3 tools/provision_device.py $PORT"
    echo ""
else
    echo ""
    echo "ERROR: Flashing failed!"
    echo ""
    echo "Troubleshooting:"
    echo "  - Check device is connected to $PORT"
    echo "  - Try a different USB port"
    echo "  - Ensure no other program is using the port"
    echo "  - Check USB cable quality"
    exit 1
fi
