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
DEVICE_HOST="sand-garden.local"
OTA_PORT="3232"

# Find espota.py dynamically to handle version changes
ESPOTA_SCRIPT=$(find "$HOME/Library/Arduino15/packages/esp32/hardware/esp32/" -name "espota.py" -type f 2>/dev/null | head -n 1)

if [ -z "$ESPOTA_SCRIPT" ]; then
  echo "✗ espota.py not found!"
  echo "  Please ensure ESP32 board support is installed via Arduino IDE."
  exit 1
fi

# Step 0: Check if WebClientHTML.h needs regeneration
echo "Step 0: Checking web client HTML..."

# Verify required files exist
if [ ! -f "web-client.html" ]; then
  echo "✗ web-client.html not found!"
  echo "  This file is required to generate the embedded web client."
  exit 1
fi

if [ ! -f "html_to_header.py" ]; then
  echo "✗ html_to_header.py not found!"
  echo "  This script is required to generate WebClientHTML.h."
  exit 1
fi

# Regenerate if needed
if [ ! -f "WebClientHTML.h" ] || [ "web-client.html" -nt "WebClientHTML.h" ]; then
  echo "  web-client.html has been modified, regenerating WebClientHTML.h..."
  python3 html_to_header.py web-client.html WebClientHTML.h
  if [ $? -eq 0 ]; then
    echo "✓ WebClientHTML.h regenerated"
  else
    echo "✗ Failed to regenerate WebClientHTML.h"
    exit 1
  fi
else
  echo "✓ WebClientHTML.h is up to date"
fi
echo ""

# Step 1: Clean build cache
echo "Step 1: Cleaning build cache..."
if [ "${CLEAR_CACHE:-0}" = "1" ]; then
  echo "  Clearing entire build cache (CLEAR_CACHE=1)..."
  rm -rf "$BUILD_CACHE"/*
  echo "✓ Full build cache cleared"
else
  echo "  Removing stale binaries only..."
  find "$BUILD_CACHE" -name "sand-garden.ino.bin" -type f -delete 2>/dev/null || true
  echo "✓ Stale binaries removed (use CLEAR_CACHE=1 for full cache clear)"
fi
echo ""

# Step 2: Compile firmware
echo "Step 2: Compiling firmware..."
COMPILE_OUTPUT=$("$ARDUINO_CLI" compile \
  --fqbn "$FQBN" \
  --warnings default \
  --build-property compiler.cpp.extra_flags=-fpermissive \
  --export-binaries . 2>&1)
COMPILE_STATUS=$?

if [ $COMPILE_STATUS -ne 0 ]; then
  echo "✗ Compilation failed!"
  echo ""
  echo "Full compilation output:"
  echo "$COMPILE_OUTPUT"
  exit 1
fi

# Show last few lines on success
echo "$COMPILE_OUTPUT" | tail -5

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

# Step 4: Pre-flight checks
echo "Step 4: Pre-flight checks..."

# Check if device is reachable
echo "  Checking device connectivity to $DEVICE_HOST..."
if ping -c 1 -W 2 "$DEVICE_HOST" &>/dev/null; then
  echo "✓ Device is reachable"
else
  echo "⚠ Warning: Cannot reach $DEVICE_HOST"
  echo "  Device may be offline or not on the same network."
  echo "  Upload will be attempted anyway, but may fail."
fi
echo ""

# Step 5: Upload via OTA
echo "Step 5: Uploading via OTA to $DEVICE_HOST..."
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
