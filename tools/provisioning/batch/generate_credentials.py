#!/usr/bin/env python3
"""
Generate LoRaWAN credentials for batch device provisioning.

This script generates unique credentials (DevEUI, AppEUI, AppKey) for
a batch of devices and outputs them to a CSV file for use in manufacturing.
"""

import argparse
import csv
import secrets
import sys
from datetime import datetime


def generate_dev_eui(prefix: str, index: int) -> str:
    """
    Generate a unique DevEUI.

    Args:
        prefix: 12-character hex prefix (e.g., "70B3D57ED005")
        index: Device index (will be formatted as 4 hex chars)

    Returns:
        16-character DevEUI string
    """
    if len(prefix) != 12:
        raise ValueError(f"Prefix must be 12 hex characters, got {len(prefix)}")

    # Format index as 4 hex characters (supports up to 65535 devices)
    index_hex = f"{index:04X}"

    return f"{prefix}{index_hex}"


def generate_app_eui() -> str:
    """
    Generate Application EUI (typically same for all devices in an application).

    Returns:
        16-character AppEUI string
    """
    # Using all zeros for now (can be customized)
    return "0000000000000000"


def generate_app_key() -> str:
    """
    Generate a secure random Application Key.

    Returns:
        32-character AppKey string
    """
    # Generate 16 random bytes and convert to hex
    random_bytes = secrets.token_bytes(16)
    return random_bytes.hex().upper()


def generate_serial_number(batch_number: str, index: int) -> str:
    """
    Generate a serial number for the device.

    Args:
        batch_number: Batch identifier
        index: Device index

    Returns:
        Serial number string
    """
    return f"WM-{batch_number}-{index:04d}"


def generate_device_name(index: int) -> str:
    """
    Generate a device name.

    Args:
        index: Device index

    Returns:
        Device name string
    """
    return f"WM-Device-{index:04d}"


def generate_credentials_batch(count: int,
                               dev_eui_prefix: str,
                               batch_number: str,
                               firmware_version: str,
                               output_file: str) -> None:
    """
    Generate a batch of device credentials and write to CSV.

    Args:
        count: Number of devices to generate
        dev_eui_prefix: 12-character DevEUI prefix
        batch_number: Batch identifier
        firmware_version: Firmware version string
        output_file: Output CSV file path
    """
    print(f"Generating credentials for {count} devices...")
    print(f"DevEUI prefix: {dev_eui_prefix}")
    print(f"Batch number: {batch_number}")
    print(f"Firmware version: {firmware_version}")
    print(f"Output file: {output_file}")
    print()

    # Common AppEUI for all devices
    app_eui = generate_app_eui()

    # Generate credentials for each device
    devices = []
    for i in range(1, count + 1):
        device = {
            'serial_number': generate_serial_number(batch_number, i),
            'dev_eui': generate_dev_eui(dev_eui_prefix, i),
            'app_eui': app_eui,
            'app_key': generate_app_key(),
            'device_name': generate_device_name(i),
            'batch_number': batch_number,
            'firmware_version': firmware_version,
            'manufacturing_date': datetime.now().strftime('%Y-%m-%d'),
            'status': 'ready'
        }
        devices.append(device)

        # Print progress
        if i % 10 == 0 or i == count:
            print(f"Generated: {i}/{count} devices", end='\r')

    print()  # New line after progress

    # Write to CSV file
    print(f"\nWriting to {output_file}...")
    with open(output_file, 'w', newline='') as csvfile:
        fieldnames = [
            'serial_number',
            'dev_eui',
            'app_eui',
            'app_key',
            'device_name',
            'batch_number',
            'firmware_version',
            'manufacturing_date',
            'status'
        ]

        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()

        for device in devices:
            writer.writerow(device)

    print(f"✓ Successfully generated {count} device credentials")
    print(f"✓ Saved to: {output_file}")
    print()
    print("Sample credentials (first device):")
    print(f"  Serial Number: {devices[0]['serial_number']}")
    print(f"  DevEUI:        {devices[0]['dev_eui']}")
    print(f"  AppEUI:        {devices[0]['app_eui']}")
    print(f"  AppKey:        {devices[0]['app_key']}")
    print()
    print("Next steps:")
    print(f"  1. Review credentials in {output_file}")
    print("  2. Import to your LoRaWAN network server")
    print("  3. Use batch_provisioner.py to flash devices")


def validate_dev_eui_prefix(prefix: str) -> bool:
    """
    Validate DevEUI prefix format.

    Args:
        prefix: DevEUI prefix to validate

    Returns:
        True if valid, raises ValueError if not
    """
    if len(prefix) != 12:
        raise ValueError(f"DevEUI prefix must be 12 characters, got {len(prefix)}")

    try:
        int(prefix, 16)
    except ValueError:
        raise ValueError(f"DevEUI prefix must be hexadecimal, got '{prefix}'")

    return True


def main():
    parser = argparse.ArgumentParser(
        description='Generate LoRaWAN credentials for batch device provisioning',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Generate 100 devices for production batch
  %(prog)s --count 100 --output batch_prod_001.csv \\
           --batch-number PROD-001 --dev-eui-prefix 70B3D57ED005

  # Generate 10 devices for testing
  %(prog)s --count 10 --output batch_test_001.csv \\
           --batch-number TEST-001 --dev-eui-prefix 70B3D57EDFFF \\
           --firmware-version 1.0.0-rc1

DevEUI Format:
  DevEUI is 16 hex characters (8 bytes)
  Format: [12-char prefix][4-char index]
  Example: 70B3D57ED0050001
           ^^^^^^^^^^^^---- Prefix (your organization)
                       ^^^^- Device index (auto-generated)
        """
    )

    parser.add_argument(
        '--count',
        type=int,
        required=True,
        help='Number of devices to generate (1-65535)'
    )

    parser.add_argument(
        '--output',
        type=str,
        required=True,
        help='Output CSV file path (e.g., batch_production_001.csv)'
    )

    parser.add_argument(
        '--batch-number',
        type=str,
        required=True,
        help='Batch identifier (e.g., PROD-BATCH-001)'
    )

    parser.add_argument(
        '--dev-eui-prefix',
        type=str,
        required=True,
        help='12-character hex DevEUI prefix (e.g., 70B3D57ED005)'
    )

    parser.add_argument(
        '--firmware-version',
        type=str,
        default='1.0.0',
        help='Firmware version (default: 1.0.0)'
    )

    parser.add_argument(
        '--app-eui',
        type=str,
        default='0000000000000000',
        help='Application EUI (default: 0000000000000000)'
    )

    args = parser.parse_args()

    # Validate inputs
    if args.count < 1 or args.count > 65535:
        print("ERROR: Count must be between 1 and 65535", file=sys.stderr)
        sys.exit(1)

    try:
        validate_dev_eui_prefix(args.dev_eui_prefix)
    except ValueError as e:
        print(f"ERROR: {e}", file=sys.stderr)
        sys.exit(1)

    # Generate credentials
    try:
        generate_credentials_batch(
            count=args.count,
            dev_eui_prefix=args.dev_eui_prefix,
            batch_number=args.batch_number,
            firmware_version=args.firmware_version,
            output_file=args.output
        )
    except Exception as e:
        print(f"ERROR: Failed to generate credentials: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()
