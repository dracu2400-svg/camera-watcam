#!/usr/bin/env python3
"""
Generate C header file from TFLite model
For embedding in WE2 firmware
"""

import argparse
from pathlib import Path

def generate_c_header(tflite_path, output_path, array_name='water_meter_digit_model'):
    """
    Convert TFLite model to C header file

    Args:
        tflite_path: Path to TFLite model
        output_path: Output path for C header
        array_name: Name of the C array
    """
    print(f"Reading TFLite model: {tflite_path}")

    with open(tflite_path, 'rb') as f:
        tflite_data = f.read()

    model_size = len(tflite_data)
    print(f"Model size: {model_size} bytes ({model_size/1024:.2f} KB)")

    # Generate header
    header_content = f"""/* Auto-generated model header file */
/* Generated from: {Path(tflite_path).name} */
/* Model size: {model_size} bytes ({model_size/1024:.2f} KB) */

#ifndef WATER_METER_DIGIT_MODEL_H_
#define WATER_METER_DIGIT_MODEL_H_

#ifdef __cplusplus
extern "C" {{
#endif

/* Model data array */
const unsigned int {array_name}_len = {model_size};
const unsigned char {array_name}[] = {{
"""

    # Write bytes in rows of 12
    for i in range(0, len(tflite_data), 12):
        chunk = tflite_data[i:i+12]
        hex_values = ', '.join(f'0x{b:02x}' for b in chunk)
        header_content += f"  {hex_values},\n"

    header_content += f"""
}};

#ifdef __cplusplus
}}
#endif

#endif  /* WATER_METER_DIGIT_MODEL_H_ */
"""

    # Write to file
    with open(output_path, 'w') as f:
        f.write(header_content)

    print(f"\nHeader file generated: {output_path}")
    print(f"  Array name: {array_name}")
    print(f"  Array size: {array_name}_len = {model_size}")

    return True

def main():
    parser = argparse.ArgumentParser(description='Generate C header from TFLite model')
    parser.add_argument('--input', default='models/water_meter_digit_model.tflite',
                       help='Input TFLite model path')
    parser.add_argument('--output', default='models/water_meter_digit_model.h',
                       help='Output C header path')
    parser.add_argument('--array-name', default='water_meter_digit_model',
                       help='Name of C array')

    args = parser.parse_args()

    print("=" * 60)
    print("C Header Generation")
    print("=" * 60)
    print(f"Input:      {args.input}")
    print(f"Output:     {args.output}")
    print(f"Array name: {args.array_name}")
    print("=" * 60)
    print()

    if not Path(args.input).exists():
        print(f"Error: Input model not found: {args.input}")
        print("Please convert the model first using convert_to_tflite.py")
        return 1

    # Generate header
    success = generate_c_header(args.input, args.output, args.array_name)

    if success:
        print("\n" + "=" * 60)
        print("Header generation completed successfully!")
        print("=" * 60)
        print(f"\nCopy the header file to your WE2 project:")
        print(f"  cp {args.output} EPII_CM55M_APP_S/app/scenario_app/water_meter_amr/")
        return 0
    else:
        print("\nHeader generation failed!")
        return 1

if __name__ == "__main__":
    exit(main())
