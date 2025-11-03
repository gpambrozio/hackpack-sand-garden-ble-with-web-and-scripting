# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an ESP32-based firmware for the CrunchLabs Sand Garden, a kinetic sand art machine with a radial-angular gantry that draws patterns in sand. The project consists of:

- **Firmware (`sand-garden.ino`)**: Arduino sketch for ESP32 (Arduino Nano ESP32)
- **SandScript DSL**: A custom math-expression language for defining motion patterns
- **BLE control system**: Web Bluetooth interface for remote control and script upload
- **Web client (`web-client.html`)**: Browser-based pattern preview and device control

## Build & Upload

### Required Libraries
Install these via Arduino IDE Library Manager or Arduino CLI:
```bash
arduino-cli lib install "AccelStepper" "FastLED" "OneButtonTiny" "elapsedMillis" "NimBLE-Arduino"
```

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
```bash
# Using standalone arduino-cli
arduino-cli upload --fqbn arduino:esp32:nano_nora .

# Using bundled arduino-cli from Arduino IDE 2.x (macOS)
"/Applications/Arduino IDE.app/Contents/Resources/app/lib/backend/resources/arduino-cli" upload --fqbn arduino:esp32:nano_nora .
```

### VS Code Tasks
The repository includes VS Code tasks:
- **Arduino: Compile Sand Garden** (default build task)
- **Arduino: Upload Sand Garden** (compiles first, then uploads)

### Board Configuration
- **Board**: Arduino Nano ESP32 (`arduino:esp32:nano_nora`)
- **Required flag**: `-fpermissive` (compiler.cpp.extra_flags)

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

### BLE Configuration System
**Module files**: `BLEConfigServer.h`, `BLEConfigServer.cpp`

**Service UUID**: `9b6c7e10-3b2c-4d8c-9d7c-5e2a6d1f8b01`

**Characteristics**:
- Speed multiplier (float)
- Pattern selection (int, 1-indexed)
- Mode (0=manual, 1=automatic)
- Run state (0=stopped, 1=running)
- Status (read + notify, for log messages)
- Telemetry (notify + read, for live position streaming)
- Command (write, for generic commands like "SELFTEST")
- Script upload (write, chunked transfer for SandScript source)

**Script upload protocol** (BLEConfigServer.cpp):
1. Client writes `SCRIPT_BEGIN:<length>:<slotIndex>`
2. Client writes chunks `SCRIPT_DATA:<payload>`
3. Client writes `SCRIPT_END`
4. Device compiles and responds via status characteristic

**Listener interface**: `ISGConfigListener` (BLEConfigServer.h:29) allows main sketch to react to BLE events.

### Web Client
Single-file HTML application (`web-client.html`) with:
- Web Bluetooth connectivity (Chrome/Edge only)
- Pattern visualizer (renders simulated paths)
- SandScript editor with live preview
- Device telemetry comparison (simulation vs actual position)
- Built-in SandScript preset library

The web client uses the same SandScript compiler (JavaScript port) as firmware, ensuring preview accuracy.

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

### LED Bar
- 8 RGB LEDs controlled via FastLED library
- Connected to GPIO_NUM_1
- Default max brightness: 40/255
- Used for pattern selection indicator and status feedback

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
- BLE debugging: enable Device Debug Stream in web client

## Key Files Reference

- **sand-garden.ino**: Main firmware (setup, loop, pattern functions, joystick/button handling)
- **PatternScript.h/cpp**: SandScript compiler and runtime
- **BLEConfigServer.h/cpp**: BLE service and characteristics
- **Positions.h**: Shared position struct
- **web-client.html**: Browser-based controller and simulator
- **Readme.md**: User-facing documentation and quick start
- **DSL_IMPLEMENTATION_PLAN.md**: Technical specification for SandScript (grammar, opcodes, memory layout)

## Notes

- The joystick runs on 5V (from USB-C) but ESP32 analog pins expect 3.3V. Current code scales input but adding voltage dividers is recommended for production use (see Readme.md:98).
- Pattern slot 11 is always the active SandScript slot—switching to another pattern and back restarts the script.
- BLE device name is "Sand Garden" (defined in BLEConfigServer.h:26).
- NaN/Infinity in SandScript outputs trigger fault state; device halts and reports fault mask via telemetry.
