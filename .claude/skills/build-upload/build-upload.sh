#!/bin/bash
# Sand Garden Firmware Build and OTA Upload Script
# Ensures fresh builds and proper OTA upload to sand-garden.local

set -e  # Exit on error

echo "=========================================="
echo "Sand Garden Firmware Build & Upload"
echo "=========================================="
echo ""

# Configuration
ARDUINO_CLI="/Applications/Arduino IDE.app/Contents/Resources/app/lib/backend/resources/arduino-cli"
FQBN="arduino:esp32:nano_nora"
BUILD_CACHE="$HOME/Library/Caches/arduino/sketches"
ESPOTA_SCRIPT="$HOME/Library/Arduino15/packages/esp32/hardware/esp32/3.3.2/tools/espota.py"
DEVICE_HOST="sand-garden.local"
OTA_PORT="3232"

# Step 1: Clean build cache
echo "Step 1: Cleaning build cache..."
rm -rf "$BUILD_CACHE"/*
echo "✓ Build cache cleaned"
echo ""

# Step 2: Compile firmware
echo "Step 2: Compiling firmware..."
"$ARDUINO_CLI" compile \
  --fqbn "$FQBN" \
  --warnings default \
  --build-property compiler.cpp.extra_flags=-fpermissive \
  --export-binaries . 2>&1 | tail -5

if [ $? -ne 0 ]; then
  echo "✗ Compilation failed!"
  exit 1
fi

echo "✓ Compilation successful"
echo ""

# Step 3: Find the binary
echo "Step 3: Locating binary..."
BINARY_PATH=$(find "$BUILD_CACHE" -name "sand-garden.ino.bin" -type f 2>/dev/null | head -n 1)

if [ -z "$BINARY_PATH" ]; then
  echo "✗ Binary not found!"
  exit 1
fi

echo "✓ Binary found at: $BINARY_PATH"

# Get binary info
BINARY_SIZE=$(ls -lh "$BINARY_PATH" | awk '{print $5}')
BINARY_DATE=$(ls -l "$BINARY_PATH" | awk '{print $6, $7, $8}')
echo "  Size: $BINARY_SIZE"
echo "  Date: $BINARY_DATE"
echo ""

# Step 4: Upload via OTA
echo "Step 4: Uploading via OTA to $DEVICE_HOST..."
python3 "$ESPOTA_SCRIPT" \
  -i "$DEVICE_HOST" \
  -p "$OTA_PORT" \
  -f "$BINARY_PATH"

if [ $? -eq 0 ]; then
  echo ""
  echo "=========================================="
  echo "✓ Upload successful!"
  echo "=========================================="
  echo ""
  echo "The device should now be running the latest firmware."
  echo "Wait a few seconds for the device to reboot, then test your changes."
else
  echo ""
  echo "=========================================="
  echo "✗ Upload failed!"
  echo "=========================================="
  echo ""
  echo "Troubleshooting:"
  echo "  1. Check if device is powered on"
  echo "  2. Verify device is connected to WiFi"
  echo "  3. Try pinging: ping $DEVICE_HOST"
  echo "  4. Use USB upload as fallback:"
  echo "     \"$ARDUINO_CLI\" upload --fqbn $FQBN ."
  exit 1
fi
