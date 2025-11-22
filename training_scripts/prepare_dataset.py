#!/usr/bin/env python3
"""
Water Meter Digit Dataset Preparation Script
Preprocesses images for training digit recognition model
"""

import cv2
import numpy as np
import os
from pathlib import Path
import argparse

def preprocess_image(image_path, output_size=(28, 28)):
    """
    Preprocess water meter digit images
    - Resize to target size
    - Convert to grayscale
    - Normalize
    - Apply preprocessing specific to water meters
    """
    img = cv2.imread(str(image_path))

    if img is None:
        print(f"Warning: Could not read {image_path}")
        return None

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
        h, w = img.shape
        M = cv2.getRotationMatrix2D((w//2, h//2), angle, 1.0)
        rotated = cv2.warpAffine(img, M, (w, h))
        augmented.append(rotated)

    # Brightness adjustment
    for beta in [-0.1, 0.1]:
        adjusted = np.clip(img + beta, 0, 1)
        augmented.append(adjusted)

    return augmented

def prepare_dataset(input_dir, output_dir, augment=True):
    """
    Process all images in dataset
    """
    input_path = Path(input_dir)
    output_path = Path(output_dir)

    if not input_path.exists():
        print(f"Error: Input directory does not exist: {input_dir}")
        return

    total_processed = 0

    for digit in range(10):
        digit_input = input_path / str(digit)
        digit_output = output_path / str(digit)
        digit_output.mkdir(parents=True, exist_ok=True)

        if not digit_input.exists():
            print(f"Warning: Directory for digit {digit} not found, skipping...")
            continue

        count = 0
        # Support multiple image formats
        image_files = list(digit_input.glob('*.jpg')) + \
                     list(digit_input.glob('*.png')) + \
                     list(digit_input.glob('*.jpeg'))

        for img_file in image_files:
            processed = preprocess_image(img_file)

            if processed is None:
                continue

            if augment:
                # Apply augmentation
                augmented_images = augment_image(processed)

                for idx, aug_img in enumerate(augmented_images):
                    output_file = digit_output / f"{img_file.stem}_aug{idx}.npy"
                    np.save(output_file, aug_img)
                    count += 1
            else:
                # Save without augmentation
                output_file = digit_output / f"{img_file.stem}.npy"
                np.save(output_file, processed)
                count += 1

        print(f"Processed digit {digit}: {count} images")
        total_processed += count

    print(f"\nTotal images processed: {total_processed}")

def main():
    parser = argparse.ArgumentParser(description='Prepare water meter digit dataset')
    parser.add_argument('--input', default='dataset/water_meter/raw/train',
                       help='Input directory with raw images')
    parser.add_argument('--output', default='dataset/water_meter/train',
                       help='Output directory for processed images')
    parser.add_argument('--no-augment', action='store_true',
                       help='Disable data augmentation')

    args = parser.parse_args()

    print("=" * 50)
    print("Water Meter Dataset Preparation")
    print("=" * 50)
    print(f"Input:  {args.input}")
    print(f"Output: {args.output}")
    print(f"Augmentation: {'Disabled' if args.no_augment else 'Enabled'}")
    print()

    prepare_dataset(args.input, args.output, augment=not args.no_augment)

    print("\nDone!")

if __name__ == "__main__":
    main()
