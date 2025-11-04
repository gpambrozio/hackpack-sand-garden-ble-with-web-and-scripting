---
name: build-upload
description: Builds Sand Garden firmware and uploads it via OTA to sand-garden.local. Use this skill when the user asks to build and upload, deploy changes, upload firmware, test changes on the device, or compile and deploy. Automatically handles build cache management and provides detailed progress reporting with pre-flight device checks.
---

# Build and Upload Firmware

This skill builds the Sand Garden firmware and uploads it via OTA to the device.

## When to use this skill

Use this skill when you need to:
- Compile and upload firmware changes to the device
- Ensure a clean build without stale cache issues
- Upload via OTA (Over-The-Air) to sand-garden.local

## What this skill does

1. **Cleans stale binaries** - Removes old sand-garden.ino.bin files from build cache (optional: full cache clear with `CLEAR_CACHE=1`)
2. **Compiles the firmware** - Builds with proper flags for ESP32 Nano
3. **Dynamically locates the binary** - Finds freshly built binary regardless of cache hash changes
4. **Pre-flight checks** - Verifies device connectivity before upload
5. **Uploads via OTA** - Sends binary to sand-garden.local over WiFi
6. **Reports detailed progress** - Shows compilation output, binary info, and upload status

## Instructions

When this skill is invoked, execute the build-upload.sh script in the skill directory:

```bash
bash .claude/skills/build-upload/build-upload.sh
```

**Optional: Clear entire build cache** (forces full recompile, slower but guaranteed fresh):
```bash
CLEAR_CACHE=1 bash .claude/skills/build-upload/build-upload.sh
```

The script will:
- Remove stale binaries (or full cache if CLEAR_CACHE=1)
- Compile the firmware with correct flags
- Find the latest binary dynamically
- Check device connectivity
- Upload via OTA to sand-garden.local

Monitor the output and report:
- Compilation success/failure
- Binary location
- Upload progress
- Final status

If the upload fails, suggest:
- Checking if the device is powered on
- Verifying WiFi connection
- Trying USB upload as fallback
