# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an ESP32-based firmware for the CrunchLabs Sand Garden, a kinetic sand art machine with a radial-angular gantry that draws patterns in sand. The project consists of:

- **Firmware (`sand-garden.ino`)**: Arduino sketch for ESP32 (Arduino Nano ESP32)
- **SandScript DSL**: A custom math-expression language for defining motion patterns
- **HTTP control system**: REST API with Server-Sent Events for remote control and script upload
- **BLE WiFi setup**: Minimal Bluetooth service for WiFi credential configuration
- **Web client (`web-client.html`)**: Browser-based pattern preview and device control

## Build & Upload

### Required Libraries
Install these via Arduino IDE Library Manager or Arduino CLI:
```bash
arduino-cli lib install "AccelStepper" "FastLED" "OneButtonTiny" "elapsedMillis" "NimBLE-Arduino" "ESP Async WebServer" "AsyncTCP" "ArduinoJson"
```

**Library descriptions:**
- **AccelStepper**: Stepper motor control with acceleration
- **FastLED**: WS2812B LED strip control
- **OneButtonTiny**: Button debouncing and event handling
- **elapsedMillis**: Non-blocking timing
- **NimBLE-Arduino**: Bluetooth Low Energy (for WiFi setup only)
- **ESP Async WebServer**: Asynchronous HTTP server
- **AsyncTCP**: Async TCP library for ESP32
- **ArduinoJson**: JSON parsing and generation for HTTP API

### Arduino CLI Location
On macOS, if Arduino CLI is not installed separately, you can use the bundled version from Arduino IDE 2.x:
```bash
# arduino-cli bundled with Arduino IDE 2.x (macOS)
/Applications/Arduino\ IDE.app/Contents/Resources/app/lib/backend/resources/arduino-cli

# Or if standalone arduino-cli is installed (via homebrew or other means)
arduino-cli
```

### Compile
```bash
# Using standalone arduino-cli
arduino-cli compile --fqbn arduino:esp32:nano_nora --warnings default --build-property compiler.cpp.extra_flags=-fpermissive --export-binaries .

# Using bundled arduino-cli from Arduino IDE 2.x (macOS)
"/Applications/Arduino IDE.app/Contents/Resources/app/lib/backend/resources/arduino-cli" compile --fqbn arduino:esp32:nano_nora --warnings default --build-property compiler.cpp.extra_flags=-fpermissive --export-binaries .
```

### Upload

#### USB Upload
```bash
# Using standalone arduino-cli
arduino-cli upload --fqbn arduino:esp32:nano_nora .

# Using bundled arduino-cli from Arduino IDE 2.x (macOS)
"/Applications/Arduino IDE.app/Contents/Resources/app/lib/backend/resources/arduino-cli" upload --fqbn arduino:esp32:nano_nora .
```

#### OTA (Over-The-Air) Upload
Once WiFi credentials are configured (see WiFi Setup section below), firmware can be uploaded wirelessly:

**Important**: Arduino CLI uses a build cache that can become stale. Always ensure you're uploading the latest binary by following these steps:

```bash
# Step 1: Clean build cache and compile fresh binary
rm -rf /Users/gustavoambrozio/Library/Caches/arduino/sketches/* && \
"/Applications/Arduino IDE.app/Contents/Resources/app/lib/backend/resources/arduino-cli" compile \
  --fqbn arduino:esp32:nano_nora \
  --warnings default \
  --build-property compiler.cpp.extra_flags=-fpermissive \
  --export-binaries .

# Step 2: Find the latest binary location (sketch hash changes after cache clear)
BINARY_PATH=$(find /Users/gustavoambrozio/Library/Caches/arduino/sketches/ -name "sand-garden.ino.bin" -type f | head -n 1)
echo "Binary location: $BINARY_PATH"

# Step 3: Upload via OTA using espota.py
python3 /Users/gustavoambrozio/Library/Arduino15/packages/esp32/hardware/esp32/3.3.2/tools/espota.py \
  -i sand-garden.local \
  -p 3232 \
  -f "$BINARY_PATH"
```

**Alternative**: If you know the build cache is fresh (just compiled), you can skip the clean step and find the binary directly:

```bash
# Find the most recently modified binary
BINARY_PATH=$(find /Users/gustavoambrozio/Library/Caches/arduino/sketches/ -name "sand-garden.ino.bin" -type f -exec ls -t {} + | head -n 1)

# Upload via OTA
python3 /Users/gustavoambrozio/Library/Arduino15/packages/esp32/hardware/esp32/3.3.2/tools/espota.py \
  -i sand-garden.local \
  -p 3232 \
  -f "$BINARY_PATH"
```

**Note**:
- `arduino-cli upload --port sand-garden.local` will still use USB if a USB cable is connected. For true OTA upload, use `espota.py` directly or disconnect USB.
- The sketch hash in the cache path (e.g., `CFEAF06617210DBB2CA2FA8CA76E4A7E`) changes when the build cache is cleared, so use the `find` command to locate the latest binary dynamically.

#### Automated Build & Upload (Recommended)

For the easiest workflow, use the Claude Code skill which automates the entire build and OTA upload process:

```bash
bash .claude/skills/build-upload/build-upload.sh
```

**When working with Claude Code**, simply ask:
- "Build and upload the firmware"
- "Deploy the latest changes"
- "Upload to the device"

**What it does:**
1. Cleans build cache (prevents stale binary issues)
2. Compiles firmware with correct flags
3. Dynamically locates the fresh binary
4. Uploads via OTA to sand-garden.local
5. Reports detailed progress and errors

This is the recommended approach as it ensures you always upload a fresh build without cache issues.

### VS Code Tasks
The repository includes VS Code tasks:
- **Arduino: Compile Sand Garden** (default build task)
- **Arduino: Upload Sand Garden** (compiles first, then uploads)

### Board Configuration
- **Board**: Arduino Nano ESP32 (`arduino:esp32:nano_nora`)
- **Required flag**: `-fpermissive` (compiler.cpp.extra_flags)

### WiFi Setup

The firmware supports WiFi connectivity for OTA updates and network features. WiFi credentials are stored persistently in NVS (Non-Volatile Storage) and survive reboots and firmware updates.

#### Configuring WiFi Credentials

1. **Open wifi-setup.html** in Chrome or Edge (requires Web Bluetooth)
2. Click "Connect to Sand Garden via Bluetooth" and pair with the device
3. Enter your WiFi SSID and password
4. Click "Send WiFi Credentials"
5. The device will connect to WiFi and display its IP address

#### WiFi Persistence Implementation

WiFi credentials are stored using the ESP32 Preferences library:
- **Namespace**: `wifi`
- **Keys**: `ssid` (String), `password` (String), `enabled` (bool)
- **Functions**: `loadWiFiCredentials()` and `saveWiFiCredentials()` in sand-garden.ino
- **Auto-connect**: On boot, `setup()` loads credentials and connects if available

#### Network Discovery

Once connected to WiFi:
- **mDNS hostname**: `sand-garden.local`
- **HTTP server**: Port 80 (REST API and web interface)
- **OTA port**: 3232 (ArduinoOTA)
- Device IP can be obtained from Serial output or BLE WiFi status notifications

#### HTTP API Access

After WiFi is configured, the device starts an HTTP server on port 80:
- **Control API**: `http://sand-garden.local/api/*` (or use IP address)
- **Real-time updates**: Server-Sent Events at `/api/events`
- **Web client**: Access the web interface by connecting to the device's IP or hostname

### HTTP API Reference

All POST endpoints accept JSON payloads and return JSON responses. The API supports CORS for web client access.

#### State Management

**GET /api/state** - Get all current device state
```bash
curl http://sand-garden.local/api/state
```
Response:
```json
{
  "speedMultiplier": 1.0,
  "pattern": 1,
  "autoMode": true,
  "running": false,
  "ledEffect": 0,
  "ledColorR": 255,
  "ledColorG": 255,
  "ledColorB": 255,
  "ledBrightness": 100
}
```

#### Pattern Control

**POST /api/speed** - Set speed multiplier (0.01 - 5.0)
```bash
curl -X POST http://sand-garden.local/api/speed \
  -H "Content-Type: application/json" \
  -d '{"value": 1.5}'
```

**POST /api/pattern** - Set pattern (1-based index)
```bash
curl -X POST http://sand-garden.local/api/pattern \
  -H "Content-Type: application/json" \
  -d '{"value": 3}'
```

**POST /api/mode** - Set auto/manual mode
```bash
curl -X POST http://sand-garden.local/api/mode \
  -H "Content-Type: application/json" \
  -d '{"value": true}'  # true = auto, false = manual
```

**POST /api/run** - Start/stop pattern execution
```bash
curl -X POST http://sand-garden.local/api/run \
  -H "Content-Type: application/json" \
  -d '{"value": true}'  # true = running, false = stopped
```

#### LED Control

**POST /api/led/effect** - Set LED effect (0-9)
```bash
curl -X POST http://sand-garden.local/api/led/effect \
  -H "Content-Type: application/json" \
  -d '{"value": 2}'
```

**POST /api/led/color** - Set LED RGB color (0-255)
```bash
curl -X POST http://sand-garden.local/api/led/color \
  -H "Content-Type: application/json" \
  -d '{"r": 255, "g": 0, "b": 128}'
```

**POST /api/led/brightness** - Set LED brightness (0-255)
```bash
curl -X POST http://sand-garden.local/api/led/brightness \
  -H "Content-Type: application/json" \
  -d '{"value": 150}'
```

#### Commands

**POST /api/command** - Execute command
```bash
curl -X POST http://sand-garden.local/api/command \
  -H "Content-Type: application/json" \
  -d '{"command": "HOME"}'
```

#### SandScript Upload

**POST /api/script/begin** - Start script upload
```bash
curl -X POST http://sand-garden.local/api/script/begin \
  -H "Content-Type: application/json" \
  -d '{"length": 45, "slot": 11}'
```

**POST /api/script/chunk** - Upload script data (can be sent as single request for small scripts)
```bash
curl -X POST http://sand-garden.local/api/script/chunk \
  -H "Content-Type: text/plain" \
  --data-binary @script.txt
```

**POST /api/script/end** - Finalize script upload
```bash
curl -X POST http://sand-garden.local/api/script/end
```

Complete SandScript upload example:
```bash
# Create a simple script
cat > /tmp/script.txt << 'EOF'
next_radius = clamp(radius + 0.2, 0, 10)
next_angle = angle + 34
EOF

# Get script length
SCRIPT_LEN=$(wc -c < /tmp/script.txt)

# Upload script
curl -X POST http://sand-garden.local/api/script/begin \
  -H "Content-Type: application/json" \
  -d "{\"length\": $SCRIPT_LEN, \"slot\": 11}"

curl -X POST http://sand-garden.local/api/script/chunk \
  -H "Content-Type: text/plain" \
  --data-binary @/tmp/script.txt

curl -X POST http://sand-garden.local/api/script/end

# Switch to SandScript pattern (pattern 11)
curl -X POST http://sand-garden.local/api/pattern \
  -H "Content-Type: application/json" \
  -d '{"value": 11}'
```

#### Real-time Updates (Server-Sent Events)

**GET /api/events** - Subscribe to device events
```bash
curl -N http://sand-garden.local/api/events
```

Event types:
- `state` - Initial state snapshot on connect
- `speed` - Speed multiplier changed
- `pattern` - Pattern changed
- `mode` - Auto/manual mode changed
- `run` - Run state changed
- `ledEffect` - LED effect changed
- `ledColor` - LED color changed
- `ledBrightness` - LED brightness changed
- `status` - Status messages
- `telemetry` - Position and motion telemetry

Example SSE stream:
```
event: state
data: {"speed":1.0,"pattern":1,"mode":1,"run":0,"ledEffect":0,"ledBrightness":100}

event: status
data: [SCRIPT] BEGIN len=45 slot=11

event: telemetry
data: r=350 a=1234 fault=0
```

## Code Architecture

### Motion Control System
The gantry uses polar coordinates with two stepper motors:
- **Radial axis**: Controls distance from center (0-10cm, ~7000 steps max)
- **Angular axis**: Controls rotation (360°, 4096 steps per revolution)

Key conversion constants (sand-garden.ino):
- `STEPS_PER_MM`: 70.0 steps per millimeter of radial travel
- `STEPS_PER_DEG`: ~11.378 steps per degree of angular motion
- `MAX_R_STEPS`: 7000 (soft limit leaving buffer on each end)
- `MAX_SPEED_R_MOTOR` / `MAX_SPEED_A_MOTOR`: 550 steps/sec

The `Positions` struct (Positions.h) stores motor positions in steps:
```cpp
struct Positions {
  long radial;   // radial position in motor steps
  long angular;  // angular position in motor steps
};
```

### Pattern System
Patterns are defined as functions with signature:
```cpp
Positions pattern_X(Positions current, bool restartPattern = false);
```

Built-in patterns (sand-garden.ino:185): `pattern_SimpleSpiral`, `pattern_Cardioids`, `pattern_WavySpiral`, `pattern_RotatingSquares`, `pattern_PentagonSpiral`, `pattern_HexagonVortex`, `pattern_PentagonRainbow`, `pattern_RandomWalk1`, `pattern_RandomWalk2`, `pattern_AccidentalButterfly`, `pattern_SandScript`.

Pattern slot 11 (`SCRIPT_PATTERN_INDEX`) is reserved for dynamic SandScript execution.

### SandScript DSL
A minimal math-expression language for defining motion patterns. Scripts run identically in firmware and browser preview.

**Module files**: `PatternScript.h`, `PatternScript.cpp`

**Input variables** (read-only):
- `radius` (cm), `angle` (degrees, wrapped 0-360)
- `start` (1 on first step, 0 otherwise)
- `rev` (continuous revolution counter: unwrappedAngle / 360)
- `steps` (evaluation counter, increments each step)
- `time` (milliseconds since script start)

**Output variables** (assignable):
- `next_radius`, `next_angle` (absolute target position in cm/degrees)
- `delta_radius`, `delta_angle` (relative increments from current position)

**Supported functions**:
- Trig (degree-based): `sin`, `cos`, `tan`
- Math: `abs`, `sign`, `clamp(v,min,max)`, `pingpong(v,max)`, `min`, `max`, `pow`, `sqrt`, `exp`, `floor`, `ceil`, `round`
- Random: `random()` (returns [0,1), reseeds on `start==1`)

**Operators**: `+`, `-`, `*`, `/`, `%`, unary `-`, parentheses

**Comments**: `#` to end-of-line

**Example script**:
```
# Spiral outward with angle-based modulation
next_radius = clamp(radius + 0.2 + sin(angle * 2) * 0.6, 0, 10)
next_angle = angle + 34
```

**Architecture notes**:
- Scripts compile to RPN bytecode (opcodes defined in PatternScript.h:34-59)
- Variables mapped to indices (PatternScript.h:65-79)
- Execution uses float stack with configurable depth (PSG_MAX_STACK_DEPTH=24)
- Runtime tracks unwrapped angle for `rev` calculation and temporal state for `steps`/`time`
- NaN/Infinity propagation triggers fault state and halts motion

**Limits** (PatternScript.h:26-31):
- Max script chars: 768
- Max tokens: 240
- Max stack depth: 24
- Max assignments: 20
- Max locals: 12

**Error codes**: See `PSGCompileResult` enum (PatternScript.h:6-17)

### HTTP Configuration System
**Module files**: `HTTPConfigServer.h`, `HTTPConfigServer.cpp`

The device exposes a REST API with JSON endpoints for all control operations. Real-time updates are delivered via Server-Sent Events (SSE).

**HTTP API Endpoints**:
- `GET /api/state` - Get all current values (JSON)
- `POST /api/speed` - Set speed multiplier `{value: float}`
- `POST /api/pattern` - Set pattern `{value: int}`
- `POST /api/mode` - Set auto/manual mode `{value: bool}`
- `POST /api/run` - Start/stop pattern `{value: bool}`
- `POST /api/command` - Execute command `{command: string}`
- `POST /api/script/begin` - Start script upload `{length: int, slot: int}`
- `POST /api/script/chunk` - Upload script chunk (raw binary)
- `POST /api/script/end` - Finalize script upload
- `POST /api/led/effect` - Set LED effect `{value: int}`
- `POST /api/led/color` - Set LED color `{r: int, g: int, b: int}`
- `POST /api/led/brightness` - Set LED brightness `{value: int}`
- `GET /api/events` - Server-Sent Events stream (status, telemetry, state updates)

**Script upload protocol**:
1. POST to `/api/script/begin` with `{length: <bytes>, slot: <index>}`
2. POST script data to `/api/script/chunk` (can be sent in one request)
3. POST to `/api/script/end` to finalize
4. Device compiles and responds via SSE status events

**Listener interface**: `ISGConfigListener` (HTTPConfigServer.h:17) allows main sketch to react to HTTP API calls.

**CORS**: All endpoints support CORS with `Access-Control-Allow-Origin: *` for easy web client access.

### BLE WiFi Setup (Minimal)
**Module files**: `BLEWiFiSetup.h`, `BLEWiFiSetup.cpp`

BLE is now used **only for WiFi credential configuration**. This minimal service reduces complexity and memory usage.

**Service UUID**: `9b6c7e10-3b2c-4d8c-9d7c-5e2a6d1f8b01`

**Characteristics**:
- WiFi SSID (write/read): `9b6c7e19-3b2c-4d8c-9d7c-5e2a6d1f8b01`
- WiFi Password (write-only): `9b6c7e1a-3b2c-4d8c-9d7c-5e2a6d1f8b01`
- WiFi Status (read/notify): `9b6c7e1b-3b2c-4d8c-9d7c-5e2a6d1f8b01`

**Listener interface**: `IWiFiSetupListener` (BLEWiFiSetup.h:23) allows main sketch to handle credential updates.

### Web Client
Single-file HTML application (`web-client.html`) with:
- HTTP connectivity (works in all modern browsers)
- Real-time updates via Server-Sent Events (SSE)
- Pattern visualizer (renders simulated paths)
- SandScript editor with live preview
- Device telemetry comparison (simulation vs actual position)
- Built-in SandScript preset library

The web client uses the same SandScript compiler (JavaScript port) as firmware, ensuring preview accuracy.

**Access methods**:
1. **Served from device** (recommended): Navigate to `http://sand-garden.local/` in any browser
   - The device serves the web client directly from its HTTP server
   - Automatically connects to the device API (same origin)
   - Requires WiFi setup (see WiFi Setup section)

2. **Local file**: Open `web-client.html` directly in a browser
   - Useful for development or when device isn't accessible
   - Requires manually entering device hostname/IP to connect

**Updating the embedded web client**:

The device serves `web-client.html` embedded in flash memory via `WebClientHTML.h`. When you modify `web-client.html`, regenerate the header file:

```bash
# Regenerate with HTML minification (default, recommended)
python3 html_to_header.py web-client.html WebClientHTML.h

# Regenerate without minification (for debugging)
python3 html_to_header.py web-client.html WebClientHTML.h --no-minify
```

**HTML Minification**: The script automatically minifies the HTML to reduce firmware size (~23% reduction):
- Removes HTML comments
- Removes whitespace between tags
- Collapses multiple spaces
- Preserves `<script>` and `<style>` content formatting
- Reduces firmware size by ~38KB

The build-upload skill automatically detects changes to `web-client.html` and regenerates `WebClientHTML.h` before compilation.

**Manual check** (if needed):
```bash
# Check if web-client.html is newer than WebClientHTML.h
[ web-client.html -nt WebClientHTML.h ] && echo "Regeneration needed" || echo "Up to date"
```

## Important Implementation Details

### Unit Conversions
Scripts work in human-friendly units (cm/degrees), firmware converts to motor steps:
- `stepsPerCm = 70.0` (radial: 1cm = 70 steps)
- `stepsPerDeg = 11.377` (angular: 1° ≈ 11.378 steps)
- Conversion happens in `evalPatternScript()` (PatternScript.cpp)

### Continuous Revolution Tracking
The `rev` input tracks total revolutions beyond 360° wrapping:
```cpp
// Pseudo-code from DSL_IMPLEMENTATION_PLAN.md:248
delta = angleDegWrapped - prevAngleDegWrapped;
if (delta > 180) delta -= 360;
else if (delta < -180) delta += 360;
unwrappedAngleDeg += delta;
rev = unwrappedAngleDeg / 360.0f;
```
This enables mirror/ping-pong patterns using `sin(rev * 180)` idioms.

### Motor Control
- `AccelStepper` library manages both motors
- Speed scaling applied to angular axis based on radius (prevents excessive acceleration at center)
- Homing sequence on boot: crash radial axis to 0, then offset by `HOMING_BUFFER`
- Manual joystick mode uses analog pins A2 (angular) and A3 (radial) with threshold-based dead zone

### LED Strips
The device has two independent WS2812B LED strips controlled via FastLED library:

**Status LED Bar** (8 LEDs):
- Connected to GPIO_NUM_1
- Default max brightness: 40/255
- Used for pattern selection indicator and status feedback
- Managed by `LedDisplay` class

**Pattern LED Strip** (39 LEDs):
- Connected to D1 pin (GPIO 43 / PATTERN_LED_DATA_PIN)
- Default max brightness: 100/255
- Displays moving rainbow effect using HSV color space
- Updates at 50 FPS for smooth animation
- Managed by `PatternLedDisplay` class

Both strips use per-strip brightness control and FastLED.show() is called once per loop cycle to coordinate updates.

## Common Development Tasks

### Adding a New Built-in Pattern
1. Declare function prototype (e.g., `Positions pattern_NewPattern(Positions current, bool restart);`)
2. Add to `patterns[]` array (sand-garden.ino:185)
3. Implement function at end of sand-garden.ino
4. Pattern index is 1-based for UI (array is 0-indexed internally)

### Adding Embedded SandScript Presets
Edit `kSandScriptPresets` table in sand-garden.ino. Each entry is `{name, source}` where source is a C string literal with escaped newlines. See sand-garden.ino:130-136 and Readme.md:126-158 for examples.

### Modifying SandScript Limits
Edit constants in PatternScript.h (lines 26-31). Increase carefully—larger limits consume more RAM. After changing limits, test script compilation and verify stack depth doesn't overflow.

### Extending SandScript Functions
1. Add opcode to `PSGOp` enum (PatternScript.h:34)
2. Implement opcode execution in `evalPatternScript()` (PatternScript.cpp)
3. Add function name to compiler/parser
4. Update web client JavaScript compiler to match
5. Document in Readme.md and DSL_IMPLEMENTATION_PLAN.md

### Testing Changes
- Compile locally with arduino-cli or VS Code task
- Use web-client.html to preview SandScript without hardware
- For firmware changes, upload to device and monitor Serial output
- HTTP debugging: Use browser DevTools Network tab to inspect API calls and SSE stream
- Test WiFi setup: Use wifi-setup.html with Web Bluetooth to configure credentials

## Key Files Reference

- **sand-garden.ino**: Main firmware (setup, loop, pattern functions, joystick/button handling)
- **PatternScript.h/cpp**: SandScript compiler and runtime
- **HTTPConfigServer.h/cpp**: HTTP REST API server with SSE
- **BLEWiFiSetup.h/cpp**: Minimal BLE service for WiFi credential setup
- **Positions.h**: Shared position struct
- **web-client.html**: Browser-based controller and simulator (source file)
- **WebClientHTML.h**: Embedded web client in flash memory (auto-generated from web-client.html)
- **html_to_header.py**: Script to regenerate WebClientHTML.h from web-client.html
- **wifi-setup.html**: WiFi credential configuration tool (BLE-based)
- **Readme.md**: User-facing documentation and quick start
- **DSL_IMPLEMENTATION_PLAN.md**: Technical specification for SandScript (grammar, opcodes, memory layout)

## Notes

- The joystick runs on 5V (from USB-C) but ESP32 analog pins expect 3.3V. Current code scales input but adding voltage dividers is recommended for production use (see Readme.md:98).
- Pattern slot 11 is always the active SandScript slot—switching to another pattern and back restarts the script.
- **WiFi required**: The HTTP server only starts after WiFi is connected. Use wifi-setup.html to configure credentials via BLE first.
- BLE device name is "Sand Garden" (defined in BLEWiFiSetup.h:15).
- NaN/Infinity in SandScript outputs trigger fault state; device halts and reports fault mask via telemetry.
- HTTP server uses port 80; ensure no firewall blocks access on your network.
