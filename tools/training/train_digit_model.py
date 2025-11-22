#!/usr/bin/env python3
"""
Water Meter Digit Recognition Model Training Script
Trains a lightweight CNN model optimized for TensorFlow Lite Micro
"""

import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers
import numpy as np
import os
from pathlib import Path
import argparse
import matplotlib.pyplot as plt

def load_dataset(data_dir):
    """
    Load preprocessed dataset from .npy files
    """
    images = []
    labels = []

    data_path = Path(data_dir)

    if not data_path.exists():
        print(f"Error: Dataset directory not found: {data_dir}")
        return None, None

    for digit in range(10):
        digit_path = data_path / str(digit)
        if not digit_path.exists():
            print(f"Warning: Directory for digit {digit} not found")
            continue

        npy_files = list(digit_path.glob('*.npy'))
        print(f"Loading digit {digit}: {len(npy_files)} samples")

        for npy_file in npy_files:
            try:
                img = np.load(npy_file)
                images.append(img)
                labels.append(digit)
            except Exception as e:
                print(f"Error loading {npy_file}: {e}")

    if len(images) == 0:
        print("Error: No images loaded!")
        return None, None

    images = np.array(images)
    labels = np.array(labels)

    # Add channel dimension
    images = np.expand_dims(images, -1)

    return images, labels

def create_digit_model(input_shape=(28, 28, 1), num_classes=10):
    """
    Create lightweight CNN for digit recognition
    Optimized for TensorFlow Lite Micro on WE2

    Model architecture:
    - Conv2D (16 filters) + MaxPool
    - Conv2D (32 filters) + MaxPool
    - Conv2D (32 filters)
    - Dense (64)
    - Dense (10, softmax)

    Total parameters: ~30K
    """
    model = keras.Sequential([
        # Input layer
        layers.Input(shape=input_shape),

        # Conv Block 1
        layers.Conv2D(16, (3, 3), activation='relu', padding='same', name='conv1'),
        layers.MaxPooling2D((2, 2), name='pool1'),

        # Conv Block 2
        layers.Conv2D(32, (3, 3), activation='relu', padding='same', name='conv2'),
        layers.MaxPooling2D((2, 2), name='pool2'),

        # Conv Block 3
        layers.Conv2D(32, (3, 3), activation='relu', padding='same', name='conv3'),

        # Classifier
        layers.Flatten(name='flatten'),
        layers.Dense(64, activation='relu', name='dense1'),
        layers.Dropout(0.3, name='dropout'),
        layers.Dense(num_classes, activation='softmax', name='output')
    ], name='WaterMeterDigitModel')

    return model

def plot_training_history(history, save_path='models/training_history.png'):
    """
    Plot training history
    """
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 4))

    # Accuracy
    ax1.plot(history.history['accuracy'], label='Train Accuracy')
    ax1.plot(history.history['val_accuracy'], label='Val Accuracy')
    ax1.set_xlabel('Epoch')
    ax1.set_ylabel('Accuracy')
    ax1.set_title('Model Accuracy')
    ax1.legend()
    ax1.grid(True)

    # Loss
    ax2.plot(history.history['loss'], label='Train Loss')
    ax2.plot(history.history['val_loss'], label='Val Loss')
    ax2.set_xlabel('Epoch')
    ax2.set_ylabel('Loss')
    ax2.set_title('Model Loss')
    ax2.legend()
    ax2.grid(True)

    plt.tight_layout()
    plt.savefig(save_path)
    print(f"Training history plot saved to: {save_path}")

def train_model(train_dir='dataset/water_meter/train',
               val_dir='dataset/water_meter/val',
               test_dir='dataset/water_meter/test',
               epochs=50,
               batch_size=32):
    """
    Train the digit recognition model
    """
    # Load data
    print("Loading training dataset...")
    X_train, y_train = load_dataset(train_dir)

    print("Loading validation dataset...")
    X_val, y_val = load_dataset(val_dir)

    print("Loading test dataset...")
    X_test, y_test = load_dataset(test_dir)

    if X_train is None or X_val is None or X_test is None:
        print("Error: Failed to load datasets!")
        return None

    print(f"\nDataset Summary:")
    print(f"  Training samples:   {len(X_train)}")
    print(f"  Validation samples: {len(X_val)}")
    print(f"  Test samples:       {len(X_test)}")
    print(f"  Input shape:        {X_train.shape[1:]}")
    print()

    # Create model
    model = create_digit_model()
    model.summary()

    # Compile
    model.compile(
        optimizer=keras.optimizers.Adam(learning_rate=0.001),
        loss='sparse_categorical_crossentropy',
        metrics=['accuracy']
    )

    # Callbacks
    callbacks = [
        keras.callbacks.ModelCheckpoint(
            'models/best_digit_model.h5',
            save_best_only=True,
            monitor='val_accuracy',
            verbose=1
        ),
        keras.callbacks.EarlyStopping(
            patience=10,
            restore_best_weights=True,
            monitor='val_accuracy',
            verbose=1
        ),
        keras.callbacks.ReduceLROnPlateau(
            factor=0.5,
            patience=5,
            min_lr=1e-6,
            verbose=1
        )
    ]

    # Train
    print("\nStarting training...")
    history = model.fit(
        X_train, y_train,
        validation_data=(X_val, y_val),
        epochs=epochs,
        batch_size=batch_size,
        callbacks=callbacks,
        verbose=1
    )

    # Evaluate
    print("\nEvaluating on test set...")
    test_loss, test_acc = model.evaluate(X_test, y_test, verbose=0)
    print(f"Test accuracy: {test_acc:.4f}")
    print(f"Test loss:     {test_loss:.4f}")

    # Plot training history
    plot_training_history(history)

    # Save final model
    model.save('models/water_meter_digit_model.h5')
    print("\nModel saved to: models/water_meter_digit_model.h5")

    return model

def main():
    parser = argparse.ArgumentParser(description='Train water meter digit recognition model')
    parser.add_argument('--train-dir', default='dataset/water_meter/train',
                       help='Training data directory')
    parser.add_argument('--val-dir', default='dataset/water_meter/val',
                       help='Validation data directory')
    parser.add_argument('--test-dir', default='dataset/water_meter/test',
                       help='Test data directory')
    parser.add_argument('--epochs', type=int, default=50,
                       help='Number of training epochs')
    parser.add_argument('--batch-size', type=int, default=32,
                       help='Batch size')

    args = parser.parse_args()

    print("=" * 60)
    print("Water Meter Digit Recognition Model Training")
    print("=" * 60)
    print(f"Train dir:  {args.train_dir}")
    print(f"Val dir:    {args.val_dir}")
    print(f"Test dir:   {args.test_dir}")
    print(f"Epochs:     {args.epochs}")
    print(f"Batch size: {args.batch_size}")
    print("=" * 60)
    print()

    # Create models directory
    os.makedirs('models', exist_ok=True)

    # Train model
    model = train_model(
        train_dir=args.train_dir,
        val_dir=args.val_dir,
        test_dir=args.test_dir,
        epochs=args.epochs,
        batch_size=args.batch_size
    )

    if model is not None:
        print("\n" + "=" * 60)
        print("Training completed successfully!")
        print("=" * 60)
    else:
        print("\nTraining failed!")
        return 1

    return 0

if __name__ == "__main__":
    exit(main())
