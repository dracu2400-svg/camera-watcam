#!/usr/bin/env python3
"""
AMR Field Testing Tool
Monitors device output and collects meter reading data
"""

import serial
import time
import csv
import argparse
from datetime import datetime
from pathlib import Path

class AMRTester:
    def __init__(self, port, baudrate=921600):
        """Initialize tester with serial port"""
        try:
            self.ser = serial.Serial(port, baudrate, timeout=1)
            print(f"✓ Connected to {port} at {baudrate} baud")
            print()
        except serial.SerialException as e:
            print(f"ERROR: Could not open {port}: {e}")
            exit(1)

        self.results = []
        self.start_time = None

    def monitor(self, duration_minutes=60, output_file=None):
        """Monitor device for specified duration"""
        print("=" * 70)
        print(f"AMR Field Test - Monitoring for {duration_minutes} minutes")
        print("=" * 70)
        print()

        self.start_time = time.time()
        end_time = self.start_time + (duration_minutes * 60)

        print(f"Start time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"End time:   {datetime.fromtimestamp(end_time).strftime('%Y-%m-%d %H:%M:%S')}")
        print()
        print("Monitoring device output...")
        print("-" * 70)

        try:
            while time.time() < end_time:
                # Read line from serial
                if self.ser.in_waiting > 0:
                    line = self.ser.readline().decode('utf-8', errors='ignore').strip()

                    if line:
                        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                        elapsed = int(time.time() - self.start_time)

                        # Print with timestamp
                        print(f"[{timestamp}] {line}")

                        # Parse reading results
                        if line.startswith("READ:"):
                            self._parse_reading(line, timestamp, elapsed)

                        # Parse other important messages
                        elif "Error" in line or "Failed" in line:
                            self._log_event("ERROR", line, timestamp, elapsed)
                        elif "Join" in line and "success" in line.lower():
                            self._log_event("JOIN_SUCCESS", line, timestamp, elapsed)

                # Small delay to prevent CPU spinning
                time.sleep(0.01)

        except KeyboardInterrupt:
            print("\n\n✓ Monitoring stopped by user")

        # Save results
        self.save_results(output_file)

        # Print summary
        self.print_summary()

    def _parse_reading(self, line, timestamp, elapsed):
        """Parse meter reading from line"""
        try:
            # Format: "READ:12345678,95"
            parts = line[5:].split(',')
            if len(parts) >= 2:
                reading = parts[0]
                confidence = parts[1].rstrip('%')

                self.results.append({
                    'timestamp': timestamp,
                    'elapsed_sec': elapsed,
                    'type': 'READING',
                    'reading': reading,
                    'confidence': confidence,
                    'raw': line
                })

                print(f"  → Captured reading: {reading} (confidence: {confidence}%)")

        except Exception as e:
            print(f"  → Warning: Could not parse reading: {e}")

    def _log_event(self, event_type, line, timestamp, elapsed):
        """Log general event"""
        self.results.append({
            'timestamp': timestamp,
            'elapsed_sec': elapsed,
            'type': event_type,
            'reading': '',
            'confidence': '',
            'raw': line
        })

    def save_results(self, output_file=None):
        """Save results to CSV"""
        if not self.results:
            print("\nNo readings captured!")
            return

        if output_file is None:
            output_file = f"field_test_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"

        # Ensure output directory exists
        Path(output_file).parent.mkdir(parents=True, exist_ok=True)

        with open(output_file, 'w', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=[
                'timestamp', 'elapsed_sec', 'type', 'reading', 'confidence', 'raw'
            ])
            writer.writeheader()
            writer.writerows(self.results)

        print(f"\n✓ Results saved to: {output_file}")

    def print_summary(self):
        """Print test summary"""
        if not self.results:
            return

        print("\n" + "=" * 70)
        print("Test Summary")
        print("=" * 70)

        # Count by type
        readings = [r for r in self.results if r['type'] == 'READING']
        errors = [r for r in self.results if r['type'] == 'ERROR']
        joins = [r for r in self.results if r['type'] == 'JOIN_SUCCESS']

        print(f"\nTotal readings:      {len(readings)}")
        print(f"Total errors:        {len(errors)}")
        print(f"Join successes:      {len(joins)}")

        if readings:
            # Calculate statistics
            confidences = [float(r['confidence']) for r in readings if r['confidence']]
            if confidences:
                avg_conf = sum(confidences) / len(confidences)
                min_conf = min(confidences)
                max_conf = max(confidences)

                print(f"\nConfidence Statistics:")
                print(f"  Average: {avg_conf:.1f}%")
                print(f"  Minimum: {min_conf:.1f}%")
                print(f"  Maximum: {max_conf:.1f}%")

            # Show latest reading
            latest = readings[-1]
            print(f"\nLatest Reading:")
            print(f"  Time:       {latest['timestamp']}")
            print(f"  Reading:    {latest['reading']}")
            print(f"  Confidence: {latest['confidence']}%")

def main():
    parser = argparse.ArgumentParser(
        description='Field test AMR device and collect readings',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Monitor for 60 minutes (default)
  python3 field_test.py /dev/ttyACM0

  # Monitor for 2 hours
  python3 field_test.py /dev/ttyACM0 --duration 120

  # Save to specific file
  python3 field_test.py /dev/ttyACM0 --output test_results.csv
        """
    )

    parser.add_argument('port', help='Serial port (e.g., /dev/ttyACM0, COM3)')
    parser.add_argument('--duration', type=int, default=60,
                       help='Test duration in minutes (default: 60)')
    parser.add_argument('--output', help='Output CSV file (default: auto-generated)')
    parser.add_argument('--baudrate', type=int, default=921600,
                       help='Serial baudrate (default: 921600)')

    args = parser.parse_args()

    # Create tester
    tester = AMRTester(args.port, args.baudrate)

    # Run test
    try:
        tester.monitor(
            duration_minutes=args.duration,
            output_file=args.output
        )
    except Exception as e:
        print(f"\nERROR: {e}")
        import traceback
        traceback.print_exc()
        exit(1)

if __name__ == "__main__":
    main()
