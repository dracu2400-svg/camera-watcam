#!/usr/bin/env python3
"""
AMR Device Provisioning Tool
Configures LoRaWAN credentials and device settings
"""

import serial
import time
import sys
import argparse

class AMRProvisioner:
    def __init__(self, port, baudrate=921600):
        """Initialize provisioner with serial port"""
        try:
            self.ser = serial.Serial(port, baudrate, timeout=2)
            time.sleep(0.5)
            print(f"✓ Connected to {port} at {baudrate} baud")
        except serial.SerialException as e:
            print(f"ERROR: Could not open {port}: {e}")
            sys.exit(1)

    def send_command(self, cmd):
        """Send command and wait for response"""
        self.ser.write(f"{cmd}\r\n".encode())
        time.sleep(0.3)

        # Read response (with timeout)
        response_lines = []
        start_time = time.time()

        while time.time() - start_time < 1.0:
            if self.ser.in_waiting > 0:
                line = self.ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    response_lines.append(line)
                    if 'OK' in line or 'ERROR' in line:
                        break

        response = '\n'.join(response_lines) if response_lines else 'NO RESPONSE'
        return response

    def provision(self, device_id, deveui, appeui, appkey, region='EU868'):
        """Provision a new AMR device"""
        print("\n" + "=" * 60)
        print("Provisioning AMR Device")
        print("=" * 60)

        # Set device ID
        print(f"\n1. Setting Device ID: {device_id}")
        resp = self.send_command(f"CMD:SET_DEVID:{device_id}")
        print(f"   Response: {resp}")

        # Set DevEUI
        print(f"\n2. Setting DevEUI: {deveui}")
        resp = self.send_command(f"CMD:SET_DEVEUI:{deveui}")
        print(f"   Response: {resp}")

        # Set AppEUI
        print(f"\n3. Setting AppEUI: {appeui}")
        resp = self.send_command(f"CMD:SET_APPEUI:{appeui}")
        print(f"   Response: {resp}")

        # Set AppKey
        print(f"\n4. Setting AppKey: {appkey}")
        resp = self.send_command(f"CMD:SET_APPKEY:{appkey}")
        print(f"   Response: {resp}")

        # Set region
        print(f"\n5. Setting Region: {region}")
        resp = self.send_command(f"CMD:SET_REGION:{region}")
        print(f"   Response: {resp}")

        print("\n" + "=" * 60)
        print("✓ Provisioning complete!")
        print("=" * 60)

    def test_reading(self):
        """Test meter reading"""
        print("\n" + "=" * 60)
        print("Testing Meter Reading")
        print("=" * 60)
        print("\nPlace device in front of water meter...")
        time.sleep(1)

        resp = self.send_command("CMD:TEST_READ")
        print(f"\nResult: {resp}")

    def calibrate(self):
        """Calibrate camera"""
        print("\n" + "=" * 60)
        print("Calibrating Camera")
        print("=" * 60)
        print("\nAlign device with water meter digits...")
        time.sleep(1)

        resp = self.send_command("CMD:CALIBRATE")
        print(f"\nResult: {resp}")

    def get_status(self):
        """Get device status"""
        print("\n" + "=" * 60)
        print("Device Status")
        print("=" * 60)

        resp = self.send_command("CMD:STATUS")
        print(f"\n{resp}")

    def join_network(self):
        """Join LoRaWAN network"""
        print("\n" + "=" * 60)
        print("Joining LoRaWAN Network")
        print("=" * 60)
        print("\nAttempting to join... (this may take up to 30 seconds)")

        resp = self.send_command("CMD:JOIN")
        print(f"\nResult: {resp}")

def main():
    parser = argparse.ArgumentParser(
        description='Provision AMR device with LoRaWAN credentials',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Full provisioning
  python3 provision_device.py /dev/ttyACM0 \\
    --device-id 12345678 \\
    --deveui 0123456789ABCDEF \\
    --appeui FEDCBA9876543210 \\
    --appkey 2B7E151628AED2A6ABF7158809CF4F3C

  # Just test reading
  python3 provision_device.py /dev/ttyACM0 --test-only

  # Get status
  python3 provision_device.py /dev/ttyACM0 --status
        """
    )

    parser.add_argument('port', help='Serial port (e.g., /dev/ttyACM0, COM3)')
    parser.add_argument('--device-id', help='Device ID (8 hex digits)')
    parser.add_argument('--deveui', help='Device EUI (16 hex digits)')
    parser.add_argument('--appeui', help='Application EUI (16 hex digits)')
    parser.add_argument('--appkey', help='Application Key (32 hex digits)')
    parser.add_argument('--region', default='EU868',
                       choices=['EU868', 'US915', 'AS923', 'AU915', 'KR920', 'IN865'],
                       help='LoRaWAN region')
    parser.add_argument('--calibrate', action='store_true',
                       help='Calibrate camera')
    parser.add_argument('--test-only', action='store_true',
                       help='Only test meter reading')
    parser.add_argument('--status', action='store_true',
                       help='Get device status')
    parser.add_argument('--join', action='store_true',
                       help='Join LoRaWAN network')
    parser.add_argument('--baudrate', type=int, default=921600,
                       help='Serial baudrate (default: 921600)')

    args = parser.parse_args()

    # Create provisioner
    prov = AMRProvisioner(args.port, args.baudrate)

    # Execute operations
    if args.status:
        prov.get_status()

    elif args.test_only:
        prov.test_reading()

    elif args.calibrate:
        prov.calibrate()

    elif args.join:
        prov.join_network()

    elif args.device_id and args.deveui and args.appeui and args.appkey:
        # Full provisioning
        prov.provision(
            device_id=args.device_id,
            deveui=args.deveui,
            appeui=args.appeui,
            appkey=args.appkey,
            region=args.region
        )

        # Optionally calibrate
        if args.calibrate:
            prov.calibrate()

        # Test reading
        response = input("\nTest meter reading now? [y/N]: ")
        if response.lower() == 'y':
            prov.test_reading()

        # Join network
        response = input("\nJoin LoRaWAN network now? [y/N]: ")
        if response.lower() == 'y':
            prov.join_network()

    else:
        parser.print_help()
        print("\nERROR: Either provide all credentials or use --test-only/--status")
        sys.exit(1)

    print("\n✓ Done!")

if __name__ == "__main__":
    main()
