#!/usr/bin/env python3
"""
Water Meter Digit Training GUI

This GUI application allows you to:
1. Capture images from WE2 camera via UART
2. Label and store digit images
3. Train model in real-time
4. Test trained model
5. Export TFLite model

Usage:
    python3 training_gui.py --port /dev/ttyACM0
"""

import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import serial
import serial.tools.list_ports
import numpy as np
from PIL import Image, ImageTk, ImageDraw, ImageFont
import cv2
import os
import json
import threading
import time
from datetime import datetime
import struct

# ML imports
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers


class WE2Camera:
    """Interface to WE2 camera via UART"""

    def __init__(self, port, baudrate=921600):
        self.port = port
        self.baudrate = baudrate
        self.serial = None
        self.connected = False

    def connect(self):
        """Connect to WE2 device"""
        try:
            self.serial = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=2.0
            )
            time.sleep(0.5)
            self.connected = True
            print(f"✓ Connected to {self.port} at {self.baudrate} baud")
            return True
        except Exception as e:
            print(f"✗ Failed to connect: {e}")
            self.connected = False
            return False

    def disconnect(self):
        """Disconnect from WE2 device"""
        if self.serial and self.serial.is_open:
            self.serial.close()
        self.connected = False
        print("✓ Disconnected")

    def send_command(self, cmd):
        """Send AT-style command to WE2"""
        if not self.connected:
            return False

        try:
            self.serial.write(f"{cmd}\r\n".encode())
            return True
        except Exception as e:
            print(f"✗ Failed to send command: {e}")
            return False

    def read_response(self, timeout=2.0):
        """Read response from WE2"""
        if not self.connected:
            return None

        start_time = time.time()
        response = b""

        while time.time() - start_time < timeout:
            if self.serial.in_waiting > 0:
                response += self.serial.read(self.serial.in_waiting)
                if b"OK\r\n" in response or b"ERROR\r\n" in response:
                    break
            time.sleep(0.01)

        return response.decode('utf-8', errors='ignore')

    def capture_frame(self, width=640, height=480):
        """
        Capture frame from WE2 camera

        Returns:
            numpy array of shape (height, width) or None
        """
        if not self.connected:
            print("✗ Not connected")
            return None

        try:
            # Send capture command
            self.send_command("CAPTURE")

            # Wait for response header
            response = self.read_response(timeout=1.0)
            if "OK" not in response:
                print(f"✗ Capture failed: {response}")
                return None

            # Read image data
            expected_size = width * height
            image_data = b""

            timeout = 5.0
            start_time = time.time()

            while len(image_data) < expected_size:
                if time.time() - start_time > timeout:
                    print("✗ Timeout reading image data")
                    return None

                if self.serial.in_waiting > 0:
                    chunk = self.serial.read(min(self.serial.in_waiting, expected_size - len(image_data)))
                    image_data += chunk
                else:
                    time.sleep(0.01)

            # Convert to numpy array
            image = np.frombuffer(image_data, dtype=np.uint8)
            image = image.reshape((height, width))

            return image

        except Exception as e:
            print(f"✗ Capture error: {e}")
            return None


class DigitTrainingGUI:
    """Main GUI application for digit training"""

    def __init__(self, root):
        self.root = root
        self.root.title("Water Meter Digit Training GUI")
        self.root.geometry("1200x800")

        # Camera
        self.camera = None

        # Dataset
        self.dataset_path = "tools/training-gui/data/images"
        self.model_path = "tools/training-gui/data/models"
        os.makedirs(self.dataset_path, exist_ok=True)
        os.makedirs(self.model_path, exist_ok=True)

        # Training
        self.model = None
        self.training_thread = None
        self.is_training = False

        # Current state
        self.current_image = None
        self.current_digit_label = 0
        self.image_count = self.load_image_count()

        # Create UI
        self.create_ui()

        # Load available ports
        self.refresh_ports()

    def create_ui(self):
        """Create the user interface"""

        # ==================== TOP BAR ====================
        top_frame = ttk.Frame(self.root)
        top_frame.pack(side=tk.TOP, fill=tk.X, padx=10, pady=5)

        # Connection controls
        ttk.Label(top_frame, text="Port:").pack(side=tk.LEFT, padx=5)

        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(top_frame, textvariable=self.port_var, width=20)
        self.port_combo.pack(side=tk.LEFT, padx=5)

        ttk.Button(top_frame, text="Refresh", command=self.refresh_ports).pack(side=tk.LEFT, padx=2)

        self.connect_btn = ttk.Button(top_frame, text="Connect", command=self.toggle_connection)
        self.connect_btn.pack(side=tk.LEFT, padx=5)

        self.status_label = ttk.Label(top_frame, text="Disconnected", foreground="red")
        self.status_label.pack(side=tk.LEFT, padx=10)

        # ==================== MAIN AREA ====================
        main_frame = ttk.Frame(self.root)
        main_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True, padx=10, pady=5)

        # Left panel - Image capture
        left_panel = ttk.LabelFrame(main_frame, text="Image Capture", padding=10)
        left_panel.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=5)

        # Image display
        self.image_label = ttk.Label(left_panel, text="No image captured", relief=tk.SUNKEN)
        self.image_label.pack(fill=tk.BOTH, expand=True, pady=5)

        # Capture controls
        capture_frame = ttk.Frame(left_panel)
        capture_frame.pack(fill=tk.X, pady=5)

        ttk.Button(capture_frame, text="📷 Capture Frame", command=self.capture_frame).pack(side=tk.LEFT, padx=5)
        ttk.Button(capture_frame, text="🔄 Auto Capture", command=self.toggle_auto_capture).pack(side=tk.LEFT, padx=5)

        self.auto_capture_active = False

        # Right panel - Labeling & Training
        right_panel = ttk.Frame(main_frame)
        right_panel.pack(side=tk.RIGHT, fill=tk.Y, padx=5)

        # Labeling section
        label_frame = ttk.LabelFrame(right_panel, text="Digit Labeling", padding=10)
        label_frame.pack(fill=tk.X, pady=5)

        ttk.Label(label_frame, text="Select Digit:").pack(anchor=tk.W, pady=2)

        # Digit buttons (0-9)
        digit_grid = ttk.Frame(label_frame)
        digit_grid.pack(pady=5)

        for i in range(10):
            row = i // 5
            col = i % 5
            btn = ttk.Button(
                digit_grid,
                text=str(i),
                width=5,
                command=lambda d=i: self.set_digit_label(d)
            )
            btn.grid(row=row, column=col, padx=2, pady=2)

        self.current_label_display = ttk.Label(
            label_frame,
            text=f"Current: {self.current_digit_label}",
            font=('Arial', 14, 'bold')
        )
        self.current_label_display.pack(pady=5)

        ttk.Button(label_frame, text="💾 Save Image", command=self.save_image).pack(fill=tk.X, pady=5)

        # Dataset info
        info_frame = ttk.LabelFrame(right_panel, text="Dataset Info", padding=10)
        info_frame.pack(fill=tk.X, pady=5)

        self.dataset_info_text = tk.Text(info_frame, height=8, width=30)
        self.dataset_info_text.pack(fill=tk.BOTH, expand=True)

        ttk.Button(info_frame, text="🔄 Refresh Stats", command=self.update_dataset_stats).pack(fill=tk.X, pady=5)

        # Training section
        train_frame = ttk.LabelFrame(right_panel, text="Training", padding=10)
        train_frame.pack(fill=tk.X, pady=5)

        ttk.Button(train_frame, text="🎓 Train Model", command=self.start_training).pack(fill=tk.X, pady=2)
        ttk.Button(train_frame, text="🧪 Test Model", command=self.test_model).pack(fill=tk.X, pady=2)
        ttk.Button(train_frame, text="💾 Export TFLite", command=self.export_tflite).pack(fill=tk.X, pady=2)

        self.training_status = ttk.Label(train_frame, text="Ready", foreground="blue")
        self.training_status.pack(pady=5)

        # Progress bar
        self.progress_var = tk.DoubleVar()
        self.progress_bar = ttk.Progressbar(
            train_frame,
            variable=self.progress_var,
            maximum=100
        )
        self.progress_bar.pack(fill=tk.X, pady=5)

        # ==================== BOTTOM BAR ====================
        bottom_frame = ttk.Frame(self.root)
        bottom_frame.pack(side=tk.BOTTOM, fill=tk.X, padx=10, pady=5)

        self.info_label = ttk.Label(
            bottom_frame,
            text=f"Total Images: {self.image_count} | Model: Not trained",
            relief=tk.SUNKEN
        )
        self.info_label.pack(fill=tk.X)

        # Update stats on startup
        self.update_dataset_stats()

    def refresh_ports(self):
        """Refresh available serial ports"""
        ports = [port.device for port in serial.tools.list_ports.comports()]
        self.port_combo['values'] = ports
        if ports:
            self.port_combo.current(0)

    def toggle_connection(self):
        """Toggle camera connection"""
        if self.camera and self.camera.connected:
            self.camera.disconnect()
            self.camera = None
            self.connect_btn.config(text="Connect")
            self.status_label.config(text="Disconnected", foreground="red")
        else:
            port = self.port_var.get()
            if not port:
                messagebox.showerror("Error", "Please select a port")
                return

            self.camera = WE2Camera(port)
            if self.camera.connect():
                self.connect_btn.config(text="Disconnect")
                self.status_label.config(text="Connected", foreground="green")
            else:
                messagebox.showerror("Error", f"Failed to connect to {port}")
                self.camera = None

    def capture_frame(self):
        """Capture a single frame"""
        if not self.camera or not self.camera.connected:
            messagebox.showwarning("Warning", "Camera not connected")
            return

        self.status_label.config(text="Capturing...", foreground="orange")
        self.root.update()

        image = self.camera.capture_frame()

        if image is not None:
            self.current_image = image
            self.display_image(image)
            self.status_label.config(text="Connected", foreground="green")
        else:
            messagebox.showerror("Error", "Failed to capture frame")
            self.status_label.config(text="Connected", foreground="green")

    def toggle_auto_capture(self):
        """Toggle auto-capture mode"""
        self.auto_capture_active = not self.auto_capture_active

        if self.auto_capture_active:
            self.auto_capture_loop()

    def auto_capture_loop(self):
        """Auto-capture loop"""
        if not self.auto_capture_active:
            return

        self.capture_frame()
        self.root.after(1000, self.auto_capture_loop)  # Capture every 1 second

    def display_image(self, image):
        """Display image in GUI"""
        # Resize for display
        display_size = (400, 300)

        # Convert to PIL Image
        pil_image = Image.fromarray(image)
        pil_image = pil_image.resize(display_size, Image.Resampling.LANCZOS)

        # Convert to PhotoImage
        photo = ImageTk.PhotoImage(pil_image)

        # Update label
        self.image_label.config(image=photo, text="")
        self.image_label.image = photo  # Keep reference

    def set_digit_label(self, digit):
        """Set current digit label"""
        self.current_digit_label = digit
        self.current_label_display.config(text=f"Current: {digit}")

    def save_image(self):
        """Save current image to dataset"""
        if self.current_image is None:
            messagebox.showwarning("Warning", "No image to save")
            return

        # Create digit directory
        digit_dir = os.path.join(self.dataset_path, str(self.current_digit_label))
        os.makedirs(digit_dir, exist_ok=True)

        # Generate filename
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
        filename = f"digit_{self.current_digit_label}_{timestamp}.png"
        filepath = os.path.join(digit_dir, filename)

        # Resize to 28x28 for training
        image_28x28 = cv2.resize(self.current_image, (28, 28))

        # Save image
        cv2.imwrite(filepath, image_28x28)

        self.image_count += 1
        self.save_image_count()

        # Update UI
        self.info_label.config(text=f"Total Images: {self.image_count} | Saved: {filename}")
        self.update_dataset_stats()

        # Auto-increment label (optional)
        # self.set_digit_label((self.current_digit_label + 1) % 10)

    def load_image_count(self):
        """Load total image count"""
        count = 0
        if os.path.exists(self.dataset_path):
            for digit in range(10):
                digit_dir = os.path.join(self.dataset_path, str(digit))
                if os.path.exists(digit_dir):
                    count += len([f for f in os.listdir(digit_dir) if f.endswith('.png')])
        return count

    def save_image_count(self):
        """Save image count (just recalculate)"""
        self.image_count = self.load_image_count()

    def update_dataset_stats(self):
        """Update dataset statistics"""
        stats_text = ""
        total = 0

        for digit in range(10):
            digit_dir = os.path.join(self.dataset_path, str(digit))
            if os.path.exists(digit_dir):
                count = len([f for f in os.listdir(digit_dir) if f.endswith('.png')])
                stats_text += f"Digit {digit}: {count}\n"
                total += count
            else:
                stats_text += f"Digit {digit}: 0\n"

        stats_text += f"\nTotal: {total}"

        self.dataset_info_text.delete(1.0, tk.END)
        self.dataset_info_text.insert(1.0, stats_text)

    def start_training(self):
        """Start model training in background thread"""
        if self.is_training:
            messagebox.showwarning("Warning", "Training already in progress")
            return

        # Check if we have enough data
        total_images = self.load_image_count()
        if total_images < 100:
            result = messagebox.askyesno(
                "Warning",
                f"Only {total_images} images in dataset. Recommended: 1000+. Continue anyway?"
            )
            if not result:
                return

        self.is_training = True
        self.training_status.config(text="Training...", foreground="orange")

        # Run training in background thread
        self.training_thread = threading.Thread(target=self.train_model_thread)
        self.training_thread.start()

    def train_model_thread(self):
        """Training thread (runs in background)"""
        try:
            # Load dataset
            self.update_training_status("Loading dataset...")
            X_train, y_train = self.load_dataset()

            if len(X_train) == 0:
                self.update_training_status("No training data", "red")
                self.is_training = False
                return

            # Split train/validation
            split_idx = int(len(X_train) * 0.8)
            X_val = X_train[split_idx:]
            y_val = y_train[split_idx:]
            X_train = X_train[:split_idx]
            y_train = y_train[:split_idx]

            # Build model
            self.update_training_status("Building model...")
            self.model = self.build_model()

            # Train model
            self.update_training_status("Training model...")

            history = self.model.fit(
                X_train, y_train,
                batch_size=32,
                epochs=10,
                validation_data=(X_val, y_val),
                verbose=1
            )

            # Save model
            self.update_training_status("Saving model...")
            model_file = os.path.join(self.model_path, "digit_model.h5")
            self.model.save(model_file)

            # Update UI
            val_acc = history.history['val_accuracy'][-1]
            self.update_training_status(
                f"Training complete! Accuracy: {val_acc*100:.2f}%",
                "green"
            )

            messagebox.showinfo("Success", f"Model trained successfully!\nValidation accuracy: {val_acc*100:.2f}%")

        except Exception as e:
            self.update_training_status(f"Training failed: {e}", "red")
            messagebox.showerror("Error", f"Training failed:\n{e}")

        finally:
            self.is_training = False
            self.progress_var.set(0)

    def update_training_status(self, text, color="blue"):
        """Update training status label (thread-safe)"""
        self.root.after(0, lambda: self.training_status.config(text=text, foreground=color))

    def load_dataset(self):
        """Load dataset from disk"""
        images = []
        labels = []

        for digit in range(10):
            digit_dir = os.path.join(self.dataset_path, str(digit))
            if not os.path.exists(digit_dir):
                continue

            for filename in os.listdir(digit_dir):
                if not filename.endswith('.png'):
                    continue

                filepath = os.path.join(digit_dir, filename)
                image = cv2.imread(filepath, cv2.IMREAD_GRAYSCALE)

                if image is not None:
                    images.append(image)
                    labels.append(digit)

        # Convert to numpy arrays
        X = np.array(images, dtype=np.float32) / 255.0  # Normalize to [0, 1]
        X = X.reshape(-1, 28, 28, 1)  # Add channel dimension
        y = np.array(labels, dtype=np.int32)

        print(f"Loaded {len(X)} images")
        return X, y

    def build_model(self):
        """Build CNN model for digit recognition"""
        model = keras.Sequential([
            layers.Conv2D(32, (3, 3), activation='relu', input_shape=(28, 28, 1)),
            layers.MaxPooling2D((2, 2)),
            layers.Conv2D(64, (3, 3), activation='relu'),
            layers.MaxPooling2D((2, 2)),
            layers.Conv2D(64, (3, 3), activation='relu'),
            layers.Flatten(),
            layers.Dense(64, activation='relu'),
            layers.Dropout(0.5),
            layers.Dense(10, activation='softmax')
        ])

        model.compile(
            optimizer='adam',
            loss='sparse_categorical_crossentropy',
            metrics=['accuracy']
        )

        return model

    def test_model(self):
        """Test model on current image"""
        if self.model is None:
            # Try to load existing model
            model_file = os.path.join(self.model_path, "digit_model.h5")
            if os.path.exists(model_file):
                self.model = keras.models.load_model(model_file)
            else:
                messagebox.showerror("Error", "No model available. Train a model first.")
                return

        if self.current_image is None:
            messagebox.showwarning("Warning", "No image to test")
            return

        # Preprocess image
        image_28x28 = cv2.resize(self.current_image, (28, 28))
        image_normalized = image_28x28.astype(np.float32) / 255.0
        image_input = image_normalized.reshape(1, 28, 28, 1)

        # Run inference
        predictions = self.model.predict(image_input, verbose=0)
        predicted_digit = np.argmax(predictions[0])
        confidence = predictions[0][predicted_digit]

        # Show results
        result_text = f"Predicted: {predicted_digit}\n"
        result_text += f"Confidence: {confidence*100:.2f}%\n\n"
        result_text += "All predictions:\n"
        for i, prob in enumerate(predictions[0]):
            result_text += f"  {i}: {prob*100:.2f}%\n"

        messagebox.showinfo("Test Result", result_text)

    def export_tflite(self):
        """Export model to TensorFlow Lite format"""
        if self.model is None:
            # Try to load existing model
            model_file = os.path.join(self.model_path, "digit_model.h5")
            if os.path.exists(model_file):
                self.model = keras.models.load_model(model_file)
            else:
                messagebox.showerror("Error", "No model available. Train a model first.")
                return

        try:
            # Convert to TFLite
            converter = tf.lite.TFLiteConverter.from_keras_model(self.model)

            # Quantize to INT8 for edge deployment
            converter.optimizations = [tf.lite.Optimize.DEFAULT]
            converter.target_spec.supported_types = [tf.int8]

            tflite_model = converter.convert()

            # Save TFLite model
            tflite_file = os.path.join(self.model_path, "digit_model.tflite")
            with open(tflite_file, 'wb') as f:
                f.write(tflite_model)

            messagebox.showinfo(
                "Success",
                f"TFLite model exported successfully!\n\nSaved to:\n{tflite_file}\n\nSize: {len(tflite_model)} bytes"
            )

        except Exception as e:
            messagebox.showerror("Error", f"Export failed:\n{e}")


def main():
    """Main entry point"""
    import argparse

    parser = argparse.ArgumentParser(description='Water Meter Digit Training GUI')
    parser.add_argument('--port', help='Serial port (e.g., /dev/ttyACM0)')
    args = parser.parse_args()

    root = tk.Tk()
    app = DigitTrainingGUI(root)

    # Auto-connect if port specified
    if args.port:
        app.port_var.set(args.port)
        app.toggle_connection()

    root.mainloop()


if __name__ == "__main__":
    main()
