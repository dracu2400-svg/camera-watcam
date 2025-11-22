#!/usr/bin/env python3
"""
Convert Keras model to TensorFlow Lite format
Optimized for microcontrollers (INT8 quantization)
"""

import tensorflow as tf
import numpy as np
import argparse
from pathlib import Path

def representative_dataset_gen(data_dir='dataset/water_meter/val'):
    """
    Representative dataset for quantization
    Uses validation set samples
    """
    data_path = Path(data_dir)

    samples = []
    for digit in range(10):
        digit_path = data_path / str(digit)
        if digit_path.exists():
            npy_files = list(digit_path.glob('*.npy'))[:10]  # Take first 10 per digit
            for npy_file in npy_files:
                img = np.load(npy_file)
                samples.append(img)

    # Shuffle samples
    np.random.shuffle(samples)

    # Limit to 100 samples
    samples = samples[:100]

    print(f"Using {len(samples)} samples for quantization calibration")

    def generator():
        for sample in samples:
            # Add batch and channel dimensions
            data = np.expand_dims(sample, axis=0)
            data = np.expand_dims(data, axis=-1)
            data = data.astype(np.float32)
            yield [data]

    return generator

def convert_to_tflite(model_path, output_path, quantize=True):
    """
    Convert Keras model to TFLite with optional quantization

    Args:
        model_path: Path to Keras model (.h5)
        output_path: Output path for TFLite model (.tflite)
        quantize: Whether to apply INT8 quantization
    """
    print(f"Loading model from: {model_path}")

    # Load model
    try:
        model = tf.keras.models.load_model(model_path)
        print("Model loaded successfully")
        model.summary()
    except Exception as e:
        print(f"Error loading model: {e}")
        return False

    # Convert to TFLite
    print("\nConverting to TensorFlow Lite...")
    converter = tf.lite.TFLiteConverter.from_keras_model(model)

    if quantize:
        print("Applying INT8 quantization...")

        # Optimization settings for microcontrollers
        converter.optimizations = [tf.lite.Optimize.DEFAULT]
        converter.representative_dataset = representative_dataset_gen()

        # Set input/output types to INT8
        converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
        converter.inference_input_type = tf.int8
        converter.inference_output_type = tf.int8

    # Convert
    try:
        tflite_model = converter.convert()
        print("Conversion successful!")
    except Exception as e:
        print(f"Error during conversion: {e}")
        return False

    # Save
    with open(output_path, 'wb') as f:
        f.write(tflite_model)

    print(f"\nModel saved to: {output_path}")
    print(f"Model size: {len(tflite_model) / 1024:.2f} KB")

    # Test the model
    print("\nTesting converted model...")
    try:
        interpreter = tf.lite.Interpreter(model_content=tflite_model)
        interpreter.allocate_tensors()

        input_details = interpreter.get_input_details()
        output_details = interpreter.get_output_details()

        print("\nInput details:")
        print(f"  Name:   {input_details[0]['name']}")
        print(f"  Shape:  {input_details[0]['shape']}")
        print(f"  Type:   {input_details[0]['dtype']}")
        print(f"  Quant:  {input_details[0].get('quantization', 'N/A')}")

        print("\nOutput details:")
        print(f"  Name:   {output_details[0]['name']}")
        print(f"  Shape:  {output_details[0]['shape']}")
        print(f"  Type:   {output_details[0]['dtype']}")
        print(f"  Quant:  {output_details[0].get('quantization', 'N/A')}")

        # Test inference with random input
        print("\nTesting inference...")
        input_shape = input_details[0]['shape']
        input_dtype = input_details[0]['dtype']

        if input_dtype == np.int8:
            test_input = np.random.randint(-128, 127, size=input_shape, dtype=np.int8)
        else:
            test_input = np.random.rand(*input_shape).astype(np.float32)

        interpreter.set_tensor(input_details[0]['index'], test_input)
        interpreter.invoke()
        output = interpreter.get_tensor(output_details[0]['index'])

        print(f"  Input shape:  {test_input.shape}")
        print(f"  Output shape: {output.shape}")
        print("  Inference successful!")

    except Exception as e:
        print(f"Error testing model: {e}")
        return False

    return True

def main():
    parser = argparse.ArgumentParser(description='Convert Keras model to TensorFlow Lite')
    parser.add_argument('--input', default='models/best_digit_model.h5',
                       help='Input Keras model path (.h5)')
    parser.add_argument('--output', default='models/water_meter_digit_model.tflite',
                       help='Output TFLite model path (.tflite)')
    parser.add_argument('--no-quantize', action='store_true',
                       help='Disable INT8 quantization')

    args = parser.parse_args()

    print("=" * 60)
    print("TensorFlow Lite Model Conversion")
    print("=" * 60)
    print(f"Input:      {args.input}")
    print(f"Output:     {args.output}")
    print(f"Quantize:   {'No' if args.no_quantize else 'Yes (INT8)'}")
    print("=" * 60)
    print()

    if not Path(args.input).exists():
        print(f"Error: Input model not found: {args.input}")
        print("Please train the model first using train_digit_model.py")
        return 1

    # Convert
    success = convert_to_tflite(args.input, args.output, quantize=not args.no_quantize)

    if success:
        print("\n" + "=" * 60)
        print("Conversion completed successfully!")
        print("=" * 60)
        print(f"\nNext step: Generate C header file")
        print(f"  python3 training_scripts/generate_model_header.py")
        return 0
    else:
        print("\nConversion failed!")
        return 1

if __name__ == "__main__":
    exit(main())
