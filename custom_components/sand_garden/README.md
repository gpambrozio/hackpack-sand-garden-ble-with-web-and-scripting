# Sand Garden Home Assistant Integration

A custom Home Assistant integration for the CrunchLabs Sand Garden - a kinetic sand art machine with radial-angular gantry that draws mesmerizing patterns in sand.

## Features

This integration provides full control of your Sand Garden device through Home Assistant:

### Entities

- **Light** - LED Strip with integrated controls:
  - RGB color picker (for Solid Color effect)
  - Brightness slider (0-255)
  - Effect selector (14 visual effects)
- **Number** - Speed multiplier adjustment (0.01 - 5.0)
- **Select** - Pattern selection (11 built-in patterns)
- **Switch** - Auto mode toggle
- **Switch** - Running state (start/stop pattern execution)
- **Button** - Home command (return to origin)

### Real-time Updates

The integration uses Server-Sent Events (SSE) for real-time state updates, ensuring Home Assistant always reflects the current device state without polling.

## Requirements

1. **Sand Garden Device** with WiFi configured and connected to your network
2. **Home Assistant** 2023.1 or newer
3. **HACS** (Home Assistant Community Store) for easy installation

## Installation

### Via HACS (Recommended)

1. Open HACS in your Home Assistant instance
2. Click on "Integrations"
3. Click the three dots in the top right corner
4. Select "Custom repositories"
5. Add this repository URL and select "Integration" as the category
6. Click "Add"
7. Search for "Sand Garden" in HACS
8. Click "Download"
9. Restart Home Assistant

### Manual Installation

1. Copy the `custom_components/sand_garden` directory to your Home Assistant `custom_components` directory
2. Restart Home Assistant

## Configuration

### 1. Ensure WiFi is Configured

Before adding the integration, make sure your Sand Garden device is connected to WiFi:

1. Open `wifi-setup.html` in Chrome or Edge (requires Web Bluetooth)
2. Click "Connect to Sand Garden via Bluetooth"
3. Enter your WiFi SSID and password
4. Click "Send WiFi Credentials"
5. The device will connect and be accessible at `sand-garden.local`

### 2. Add the Integration

1. Go to **Settings** → **Devices & Services**
2. Click **+ Add Integration**
3. Search for "Sand Garden"
4. Enter the hostname or IP address:
   - Use `sand-garden.local` (mDNS hostname)
   - Or use the IP address shown in the device's serial output
5. Click **Submit**

The integration will automatically discover all available entities.

## Usage

### Controlling Patterns

**Select a Pattern:**
```yaml
service: select.select_option
target:
  entity_id: select.sand_garden_pattern
data:
  option: "Wavy Spiral"
```

**Available Patterns:**
- Simple Spiral
- Cardioids
- Wavy Spiral
- Rotating Squares
- Pentagon Spiral
- Hexagon Vortex
- Pentagon Rainbow
- Random Walk 1
- Random Walk 2
- Accidental Butterfly
- SandScript (custom user scripts)

**Start/Stop Execution:**
```yaml
# Start
service: switch.turn_on
target:
  entity_id: switch.sand_garden_running

# Stop
service: switch.turn_off
target:
  entity_id: switch.sand_garden_running
```

**Adjust Speed:**
```yaml
service: number.set_value
target:
  entity_id: number.sand_garden_speed_multiplier
data:
  value: 1.5
```

### Controlling LEDs

All LED controls are integrated into the Light entity.

**Set LED Effect, Color, and Brightness:**
```yaml
service: light.turn_on
target:
  entity_id: light.sand_garden_led_strip
data:
  effect: "Breathing Pulse"
  rgb_color: [255, 0, 128]
  brightness: 200
```

**Available LED Effects:**
- Rainbow - Moving rainbow wave across the strip
- Full Rainbow - Whole strip cycles through rainbow colors
- Color Waves - Sine wave color patterns
- Twinkle - Random sparkling stars
- Theater Chase - Marquee-style chase effect
- Palette Cycle - Cycling through color palettes
- Confetti - Random colored dots
- Comet - Comet/meteor with fading tail
- Breathing Pulse - Calming breathing effect
- Rotating Wedge - Colored wedge sweeps around the circle
- Bidirectional Chase - Two dots chase in opposite directions
- Color Segments - Rotating colored pie segments
- Solid Color - Single solid color (use with rgb_color)
- Off - Turn off all LEDs

**Turn Off LEDs:**
```yaml
service: light.turn_off
target:
  entity_id: light.sand_garden_led_strip
```

### Running Commands

**Return to Home Position:**
```yaml
service: button.press
target:
  entity_id: button.sand_garden_home
```

## Automation Examples

### Automatically Start a Pattern at Sunset

```yaml
automation:
  - alias: "Sand Garden - Evening Pattern"
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
```

### Change LED Color Based on Time of Day

```yaml
automation:
  - alias: "Sand Garden - Dynamic LED Colors"
    trigger:
      - platform: time
        at: "06:00:00"
      - platform: time
        at: "12:00:00"
      - platform: time
        at: "18:00:00"
      - platform: time
        at: "22:00:00"
    action:
      - choose:
          - conditions:
              - condition: time
                after: "06:00:00"
                before: "12:00:00"
            sequence:
              - service: light.turn_on
                target:
                  entity_id: light.sand_garden_led_strip
                data:
                  rgb_color: [255, 200, 100]  # Warm morning
          - conditions:
              - condition: time
                after: "12:00:00"
                before: "18:00:00"
            sequence:
              - service: light.turn_on
                target:
                  entity_id: light.sand_garden_led_strip
                data:
                  rgb_color: [255, 255, 255]  # Bright day
          - conditions:
              - condition: time
                after: "18:00:00"
                before: "22:00:00"
            sequence:
              - service: light.turn_on
                target:
                  entity_id: light.sand_garden_led_strip
                data:
                  rgb_color: [255, 100, 50]  # Warm evening
          - conditions:
              - condition: time
                after: "22:00:00"
            sequence:
              - service: light.turn_on
                target:
                  entity_id: light.sand_garden_led_strip
                data:
                  rgb_color: [50, 50, 200]  # Cool night
```

### Cycle Through Patterns Every Hour

```yaml
automation:
  - alias: "Sand Garden - Hourly Pattern Rotation"
    trigger:
      - platform: time_pattern
        hours: "/1"
    action:
      - service: select.select_next
        target:
          entity_id: select.sand_garden_pattern
```

### Stop When Away, Resume When Home

```yaml
automation:
  - alias: "Sand Garden - Stop When Away"
    trigger:
      - platform: state
        entity_id: group.all_persons
        to: "not_home"
    action:
      - service: switch.turn_off
        target:
          entity_id: switch.sand_garden_running

  - alias: "Sand Garden - Resume When Home"
    trigger:
      - platform: state
        entity_id: group.all_persons
        to: "home"
    action:
      - service: switch.turn_on
        target:
          entity_id: switch.sand_garden_running
```

## Lovelace Dashboard Example

```yaml
type: vertical-stack
cards:
  - type: entities
    title: Sand Garden
    entities:
      - entity: switch.sand_garden_running
        name: Running
      - entity: switch.sand_garden_auto_mode
        name: Auto Mode
      - entity: select.sand_garden_pattern
        name: Pattern
      - entity: number.sand_garden_speed_multiplier
        name: Speed
      - entity: button.sand_garden_home
        name: Home Position
  - type: light
    entity: light.sand_garden_led_strip
    name: LED Strip
  - type: entities
    entities:
      - entity: select.sand_garden_led_effect
        name: LED Effect
```

## Troubleshooting

### Integration Not Connecting

1. **Verify WiFi Connection:**
   - Ensure your Sand Garden is connected to WiFi
   - Check the device serial output for IP address
   - Try pinging `sand-garden.local` or the IP address

2. **Check Network:**
   - Ensure Home Assistant and Sand Garden are on the same network
   - Verify no firewall is blocking port 80

3. **Test HTTP API:**
   ```bash
   curl http://sand-garden.local/api/state
   ```
   Should return JSON with device state

### Entities Not Updating

1. **SSE Connection:**
   - The integration uses Server-Sent Events for real-time updates
   - Check Home Assistant logs for SSE connection errors
   - Integration will fall back to polling every 30 seconds if SSE fails

2. **Restart Integration:**
   - Go to **Settings** → **Devices & Services**
   - Find "Sand Garden"
   - Click the three dots → **Reload**

### Device Not Responding

1. **Restart Device:**
   - Power cycle the Sand Garden
   - Wait for it to reconnect to WiFi

2. **Check Logs:**
   - Go to **Settings** → **System** → **Logs**
   - Filter by "sand_garden"

## Development & Support

- **Repository:** [GitHub Repository URL]
- **Issues:** Report issues on GitHub
- **Documentation:** See CLAUDE.md for device API details

## License

This integration is provided as-is for use with the CrunchLabs Sand Garden.
