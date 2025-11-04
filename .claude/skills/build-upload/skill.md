# Build and Upload Firmware

This skill builds the Sand Garden firmware and uploads it via OTA to the device.

## When to use this skill

Use this skill when you need to:
- Compile and upload firmware changes to the device
- Ensure a clean build without stale cache issues
- Upload via OTA (Over-The-Air) to sand-garden.local

## What this skill does

1. Cleans the Arduino CLI build cache to prevent stale binaries
2. Compiles the firmware with proper flags
3. Dynamically locates the freshly built binary
4. Uploads the binary via OTA to sand-garden.local
5. Reports success or failure

## Instructions

When this skill is invoked, execute the build-upload.sh script in the skill directory:

```bash
bash .claude/skills/build-upload/build-upload.sh
```

The script will:
- Clean the build cache
- Compile the firmware
- Find the latest binary
- Upload via OTA

Monitor the output and report:
- Compilation success/failure
- Binary location
- Upload progress
- Final status

If the upload fails, suggest:
- Checking if the device is powered on
- Verifying WiFi connection
- Trying USB upload as fallback
