#!/bin/bash
#
# Build Water Meter AMR Firmware
# This script compiles the complete AMR solution for HIMAX WE2
#

set -e

echo "========================================"
echo "Building Water Meter AMR Firmware"
echo "========================================"

# Check toolchain
if ! command -v arm-none-eabi-gcc &> /dev/null; then
    echo "ERROR: ARM GCC toolchain not found!"
    echo ""
    echo "Please install ARM GNU Toolchain 13.2:"
    echo "  wget https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz"
    echo "  tar -xvf arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz"
    echo "  export PATH=\"\$HOME/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-eabi/bin/:\$PATH\""
    exit 1
fi

echo "✓ ARM GCC toolchain found"
arm-none-eabi-gcc --version | head -1

# Check if model header exists
if [ ! -f "models/water_meter_digit_model.h" ]; then
    echo ""
    echo "WARNING: Model header not found!"
    echo "Please train and generate model first:"
    echo "  cd training_scripts"
    echo "  python3 train_digit_model.py"
    echo "  python3 convert_to_tflite.py"
    echo "  python3 generate_model_header.py"
    echo ""
    echo "For now, using default face detection app..."
    APP_TYPE="tflm_fd_fm"
else
    echo "✓ Model header found"
    APP_TYPE="water_meter_amr"
fi

# Clean previous build
echo ""
echo "Cleaning previous build..."
cd EPII_CM55M_APP_S
make clean

# Set APP_TYPE in makefile
echo ""
echo "Setting APP_TYPE to: $APP_TYPE"
sed -i "s/^APP_TYPE = .*/APP_TYPE = $APP_TYPE/" makefile

# Build firmware
echo ""
echo "Compiling firmware..."
echo "This may take a few minutes..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Build successful!"

    # Generate image
    echo ""
    echo "Generating firmware image..."
    cd ../we2_image_gen_local/

    cp ../EPII_CM55M_APP_S/obj_epii_evb_icv30_bdv10/gnu_epii_evb_WLCSP65/EPII_CM55M_gnu_epii_evb_WLCSP65_s.elf input_case1_secboot/

    # Check OS type for image generator
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS
        if [[ $(uname -m) == "arm64" ]]; then
            IMAGE_GEN="./we2_local_image_gen_macOS_arm64"
        else
            IMAGE_GEN="./we2_local_image_gen_macOS"
        fi
    else
        # Linux
        IMAGE_GEN="./we2_local_image_gen"
    fi

    $IMAGE_GEN project_case1_blp_wlcsp.json

    if [ -f "output_case1_sec_wlcsp/output.img" ]; then
        echo ""
        echo "✓ Firmware image generated!"
        cp output_case1_sec_wlcsp/output.img ../water_meter_amr.img
        cd ..
        echo ""
        echo "========================================"
        echo "Build Complete!"
        echo "========================================"
        echo ""
        echo "Output firmware: water_meter_amr.img"
        ls -lh water_meter_amr.img
        echo ""
        echo "Next steps:"
        echo "  1. Flash firmware: ./flash_amr.sh /dev/ttyACM0"
        echo "  2. Or flash with model: ./flash_amr.sh /dev/ttyACM0 models/water_meter_digit_model.tflite"
        echo ""
    else
        echo "ERROR: Failed to generate image"
        cd ..
        exit 1
    fi
else
    echo ""
    echo "ERROR: Build failed!"
    cd ..
    exit 1
fi
