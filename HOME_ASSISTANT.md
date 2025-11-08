# Home Assistant Integration

This repository includes a custom Home Assistant integration for the Sand Garden device, allowing you to control patterns, LEDs, and monitor the device directly from your Home Assistant dashboard.

## Quick Start

### Prerequisites

1. **Sand Garden with WiFi configured** - Use `wifi-setup.html` to configure WiFi credentials
2. **Home Assistant** 2023.1 or newer
3. **HACS** installed (recommended) or manual installation

### Installation Options

#### Option 1: HACS (Recommended)

1. Open HACS in Home Assistant
2. Go to **Integrations**
3. Click the **⋮** menu (top right) → **Custom repositories**
4. Add repository URL: `https://github.com/your-repo/hackpack-sand-garden`
5. Select category: **Integration**
6. Click **Add**
7. Search for "Sand Garden"
8. Click **Download**
9. **Restart Home Assistant**

#### Option 2: Manual Installation

1. Copy the `custom_components/sand_garden` directory to your Home Assistant's `custom_components` directory:
   ```
   <config_dir>/custom_components/sand_garden/
   ```

2. Restart Home Assistant

### Configuration

1. Go to **Settings** → **Devices & Services**
2. Click **+ Add Integration**
3. Search for "Sand Garden"
4. Enter hostname: `sand-garden.local` (or IP address)
5. Click **Submit**

## What You Get

The integration creates the following entities:

### Controls

- **🔢 Speed Multiplier** - Adjust pattern speed (0.01 - 5.0x)
- **🎨 Pattern Select** - Choose from 11 built-in patterns
- **💡 LED Strip** - RGB color and brightness control
- **✨ LED Effect** - 10 visual effects (Rainbow, Fire, Ocean, etc.)
- **▶️ Running Switch** - Start/stop pattern execution
- **🔄 Auto Mode Switch** - Enable automatic pattern cycling
- **🏠 Home Button** - Return to origin position

### Real-time Updates

The integration uses Server-Sent Events (SSE) to receive instant updates from your device without polling.

## Features

### Pattern Control

Control which pattern is drawn in the sand:
- Simple Spiral
- Cardioids
- Wavy Spiral
- Rotating Squares
- Pentagon Spiral
- Hexagon Vortex
- Pentagon Rainbow
- Random Walk 1 & 2
- Accidental Butterfly
- SandScript (custom scripts)

### LED Effects

Beautiful LED animations:
- Solid Color
- Rainbow Wave
- Rainbow Cycle
- Theater Chase
- Twinkle
- Fire, Ocean, Lava, Forest
- Party mode

### Automations

Create automations like:
- Start specific patterns at sunset
- Change LED colors based on time of day
- Cycle through patterns hourly
- Stop when away, resume when home

## Example Automation

```yaml
automation:
  - alias: "Evening Sand Garden"
    trigger:
      - platform: sun
        event: sunset
    action:
      - service: select.select_option
        target:
          entity_id: select.sand_garden_pattern
        data:
          option: "Pentagon Rainbow"
      - service: switch.turn_on
        target:
          entity_id: switch.sand_garden_running
      - service: light.turn_on
        target:
          entity_id: light.sand_garden_led_strip
        data:
          rgb_color: [255, 100, 50]
```

## Documentation

For detailed documentation including:
- Full API reference
- Advanced automation examples
- Lovelace dashboard configurations
- Troubleshooting guide

See: [custom_components/sand_garden/README.md](custom_components/sand_garden/README.md)

## Device Requirements

Your Sand Garden device must have:
1. **Firmware with HTTP server** - Use the latest firmware from this repository
2. **WiFi configured** - Use `wifi-setup.html` to set credentials
3. **Network connectivity** - Device accessible at `sand-garden.local` or IP address

## Troubleshooting

### Can't find device

1. Verify WiFi is connected (check Serial output)
2. Try using IP address instead of `sand-garden.local`
3. Ensure Home Assistant and device are on same network

### Entities not updating

1. Check Home Assistant logs for connection errors
2. Reload the integration from **Devices & Services**
3. Verify device is accessible: `curl http://sand-garden.local/api/state`

## Support

For issues or questions:
- Check the detailed [README](custom_components/sand_garden/README.md)
- Review device logs in Home Assistant
- Ensure firmware is up to date

## License

This integration is part of the CrunchLabs Sand Garden hackpack project.
