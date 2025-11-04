# Build and Upload Skill

This Claude Code skill automates the process of building and uploading the Sand Garden firmware via OTA.

## Usage

### Via Claude Code

Simply ask Claude to build and upload:
```
Build and upload the firmware
```

Or invoke the skill directly:
```
/build-upload
```

### Manual Usage

You can also run the script directly:
```bash
bash .claude/skills/build-upload/build-upload.sh
```

## What it does

1. **Cleans build cache** - Removes old cached builds to prevent stale binaries
2. **Compiles firmware** - Builds fresh binary with correct flags
3. **Locates binary** - Dynamically finds the newly built binary
4. **Uploads via OTA** - Sends binary to sand-garden.local over WiFi

## Requirements

- Arduino CLI installed (bundled with Arduino IDE 2.x)
- ESP32 board support installed
- espota.py script available
- Device connected to WiFi with mDNS hostname: sand-garden.local

## Troubleshooting

If upload fails:
- Ensure device is powered on
- Check WiFi connection
- Verify mDNS resolution: `ping sand-garden.local`
- Try USB upload as fallback

## Configuration

Edit `build-upload.sh` to change:
- `DEVICE_HOST` - Device hostname or IP
- `OTA_PORT` - OTA port (default: 3232)
- Path to Arduino CLI or espota.py
