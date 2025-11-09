"""Constants for the Sand Garden integration."""

DOMAIN = "sand_garden"

# Configuration
CONF_HOST = "host"
DEFAULT_NAME = "Sand Garden"

# API endpoints
API_STATE = "/api/state"
API_SPEED = "/api/speed"
API_PATTERN = "/api/pattern"
API_MODE = "/api/mode"
API_RUN = "/api/run"
API_COMMAND = "/api/command"
API_LED_EFFECT = "/api/led/effect"
API_LED_COLOR = "/api/led/color"
API_LED_BRIGHTNESS = "/api/led/brightness"
API_RESET = "/api/reset"
API_EVENTS = "/api/events"

# Patterns (1-based index as per device API)
PATTERNS = {
    1: "Simple Spiral",
    2: "Cardioids",
    3: "Wavy Spiral",
    4: "Rotating Squares",
    5: "Pentagon Spiral",
    6: "Hexagon Vortex",
    7: "Pentagon Rainbow",
    8: "Random Walk 1",
    9: "Random Walk 2",
    10: "Accidental Butterfly",
    11: "SandScript",
}

# LED Effects (0-based index as per device)
LED_EFFECTS = {
    0: "Rainbow",
    1: "Full Rainbow",
    2: "Color Waves",
    3: "Twinkle",
    4: "Theater Chase",
    5: "Palette Cycle",
    6: "Confetti",
    7: "Comet",
    8: "Breathing Pulse",
    9: "Rotating Wedge",
    10: "Bidirectional Chase",
    11: "Color Segments",
    12: "Solid Color",
    13: "Off",
}

# Commands
COMMANDS = ["HOME", "STOP"]

# Speed limits
SPEED_MIN = 0.01
SPEED_MAX = 5.0
SPEED_STEP = 0.01

# Update intervals
SCAN_INTERVAL = 30  # seconds (fallback if SSE disconnects)
