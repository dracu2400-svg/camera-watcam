#!/bin/bash

##
# Flash script for Training Mode Firmware
##

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Configuration
APP_NAME="training_mode"
SDK_PATH="${SDK_PATH:-/path/to/HIMAX_WE2_SDK}"
BUILD_DIR="$SDK_PATH/EPII_CM55M_APP_S"
OUTPUT_DIR="$BUILD_DIR/output_${APP_NAME}_gnu_epii_CM55M_s"

# Check arguments
if [ $# -lt 1 ]; then
    echo "Usage: $0 <serial_port> [baudrate]"
    echo ""
    echo "Examples:"
    echo "  $0 /dev/ttyACM0"
    echo "  $0 /dev/ttyACM0 921600"
    echo "  $0 COM3 921600  (Windows)"
    echo ""
    exit 1
fi

SERIAL_PORT="$1"
BAUDRATE="${2:-921600}"

echo "========================================"
echo "  Flashing Training Mode Firmware"
echo "========================================"
echo ""
echo "Serial Port: $SERIAL_PORT"
echo "Baudrate: $BAUDRATE"
echo ""

# Check if firmware exists
if [ ! -f "$OUTPUT_DIR/output.elf" ]; then
    echo "ERROR: Firmware not found: $OUTPUT_DIR/output.elf"
    echo ""
    echo "Please build firmware first:"
    echo "  $SCRIPT_DIR/build_training_firmware.sh"
    echo ""
    exit 1
fi

# Check if serial port exists (Linux/Mac only)
if [[ "$OSTYPE" != "msys" && "$OSTYPE" != "win32" ]]; then
    if [ ! -e "$SERIAL_PORT" ]; then
        echo "ERROR: Serial port not found: $SERIAL_PORT"
        echo ""
        echo "Available ports:"
        ls -1 /dev/tty* | grep -E "USB|ACM" || echo "  No USB/ACM ports found"
        echo ""
        exit 1
    fi
fi

# Method 1: Try using xmodem protocol (if available)
if command -v sx >/dev/null 2>&1; then
    echo "Using xmodem protocol..."
    echo ""
    echo "Please put WE2 into bootloader mode:"
    echo "  1. Press and hold BOOT button"
    echo "  2. Press and release RESET button"
    echo "  3. Release BOOT button"
    echo ""
    read -p "Press Enter when ready..."

    sx -vv "$OUTPUT_DIR/output.bin" < "$SERIAL_PORT" > "$SERIAL_PORT"

    echo ""
    echo "✓ Firmware flashed via xmodem"

# Method 2: Try using OpenOCD (if available)
elif command -v openocd >/dev/null 2>&1; then
    echo "Using OpenOCD..."

    openocd \
        -f interface/cmsis-dap.cfg \
        -f target/cortex_m.cfg \
        -c "program $OUTPUT_DIR/output.elf verify reset exit"

    echo ""
    echo "✓ Firmware flashed via OpenOCD"

# Method 3: Provide manual instructions
else
    echo "Automated flashing tools not found."
    echo ""
    echo "Please use one of these methods:"
    echo ""
    echo "Method 1: XMODEM (recommended)"
    echo "  1. Install lrzsz package:"
    echo "     sudo apt-get install lrzsz"
    echo "  2. Re-run this script"
    echo ""
    echo "Method 2: OpenOCD + CMSIS-DAP"
    echo "  1. Connect CMSIS-DAP debugger"
    echo "  2. Install OpenOCD:"
    echo "     sudo apt-get install openocd"
    echo "  3. Re-run this script"
    echo ""
    echo "Method 3: Manual UART upload"
    echo "  1. Open serial terminal:"
    echo "     screen $SERIAL_PORT $BAUDRATE"
    echo "  2. Put WE2 in bootloader mode"
    echo "  3. Send file: $OUTPUT_DIR/output.bin"
    echo ""
    exit 1
fi

echo ""
echo "========================================"
echo "  Verifying Firmware"
echo "========================================"
echo ""

# Open serial terminal to verify
echo "Opening serial terminal..."
echo "You should see: 'WE2 Training Mode Firmware'"
echo ""
echo "Press Ctrl+A then K to exit"
echo ""
sleep 2

if command -v screen >/dev/null 2>&1; then
    screen "$SERIAL_PORT" "$BAUDRATE"
elif command -v minicom >/dev/null 2>&1; then
    minicom -D "$SERIAL_PORT" -b "$BAUDRATE"
else
    echo "No serial terminal found. Install screen or minicom:"
    echo "  sudo apt-get install screen"
    echo ""
    echo "Then manually verify:"
    echo "  screen $SERIAL_PORT $BAUDRATE"
fi

echo ""
echo "✓ Flash complete!"
echo ""
echo "Next steps:"
echo "  1. Verify firmware is running"
echo "  2. Launch training GUI:"
echo "     python3 $SCRIPT_DIR/../src/training_gui.py --port $SERIAL_PORT"
