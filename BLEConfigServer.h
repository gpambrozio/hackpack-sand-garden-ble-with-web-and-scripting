// ---------------------------------------------------------------------------------------------------------------------
// BLEConfigServer.h - BLE service abstraction for Sand Garden
// Adds a generic COMMAND characteristic to trigger actions (e.g. LED self-test) at runtime.
// ---------------------------------------------------------------------------------------------------------------------
#pragma once
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <vector>
#include <algorithm>

// BLE Service & Characteristic UUIDs (randomly generated v4 UUIDs)
// Single service exposes configuration for Sand Garden
#define SG_SERVICE_UUID             "9b6c7e10-3b2c-4d8c-9d7c-5e2a6d1f8b01"
#define SG_SPEED_CHAR_UUID          "9b6c7e11-3b2c-4d8c-9d7c-5e2a6d1f8b01"  // float speed multiplier
#define SG_PATTERN_CHAR_UUID        "9b6c7e12-3b2c-4d8c-9d7c-5e2a6d1f8b01"  // int current pattern index
#define SG_STATUS_CHAR_UUID         "9b6c7e13-3b2c-4d8c-9d7c-5e2a6d1f8b01"  // status / log lines (read + notify)
#define SG_MODE_CHAR_UUID           "9b6c7e14-3b2c-4d8c-9d7c-5e2a6d1f8b01"  // 0 manual, 1 automatic
#define SG_RUN_CHAR_UUID            "9b6c7e15-3b2c-4d8c-9d7c-5e2a6d1f8b01"  // 0 stopped, 1 running
#define SG_TELEMETRY_CHAR_UUID      "9b6c7e16-3b2c-4d8c-9d7c-5e2a6d1f8b01"  // telemetry snapshots / streaming
// Generic command channel (ASCII). Client writes commands like "SELFTEST"; device responds via status/telemetry.
#define SG_COMMAND_CHAR_UUID        "9b6c7e17-3b2c-4d8c-9d7c-5e2a6d1f8b01"
// SandScript bulk transfer characteristic (write-only chunks)
#define SG_SCRIPT_CHAR_UUID         "9b6c7e18-3b2c-4d8c-9d7c-5e2a6d1f8b01"
// WiFi SSID characteristic (write/read for WiFi network name)
#define SG_WIFI_SSID_CHAR_UUID      "9b6c7e19-3b2c-4d8c-9d7c-5e2a6d1f8b01"
// WiFi Password characteristic (write-only for WiFi password)
#define SG_WIFI_PASS_CHAR_UUID      "9b6c7e1a-3b2c-4d8c-9d7c-5e2a6d1f8b01"
// WiFi Status characteristic (read/notify for connection status and IP)
#define SG_WIFI_STATUS_CHAR_UUID    "9b6c7e1b-3b2c-4d8c-9d7c-5e2a6d1f8b01"
// LED Effect characteristic (write/read for pattern LED effect selection, 0 to NUM_PATTERN_LED_EFFECTS-1)
#define SG_LED_EFFECT_CHAR_UUID     "9b6c7e1c-3b2c-4d8c-9d7c-5e2a6d1f8b01"
// LED Color characteristic (write/read for solid color RGB, format: "R,G,B")
#define SG_LED_COLOR_CHAR_UUID      "9b6c7e1d-3b2c-4d8c-9d7c-5e2a6d1f8b01"
// LED Brightness characteristic (write/read for pattern LED brightness, 0-255)
#define SG_LED_BRIGHTNESS_CHAR_UUID "9b6c7e1e-3b2c-4d8c-9d7c-5e2a6d1f8b01"

// LED effect constants
#define NUM_PATTERN_LED_EFFECTS 10  // Total number of LED effects (0-9)

// Name of the BLE peripheral
#define SG_DEVICE_NAME "Sand Garden"

// Callback interface for host sketch to observe updates
class ISGConfigListener {
public:
  virtual void onSpeedMultiplierChanged(float newValue) = 0;
  virtual void onCurrentPatternChanged(int newPattern) = 0;
  virtual void onAutoModeChanged(bool newAutoMode) = 0;
  virtual void onRunStateChanged(bool newRunState) = 0;
  // Optional override for generic commands (uppercased token). Default no-op keeps backward compatibility.
  // 'cmd' is the uppercase trimmed command token; 'raw' holds the original payload (could include arguments later).
  virtual void onCommandReceived(const String &cmd, const std::string &raw) { (void)cmd; (void)raw; }
  // Called when a completed SandScript payload is received (after SCRIPT_END). Default no-op.
  virtual void onPatternScriptReceived(const std::string &script, int slotIndex) { (void)script; (void)slotIndex; }
  // Optional progress messages (BEGIN/CHUNK/END/ABORT) for UI logging. Default ignore.
  virtual void onPatternScriptStatus(const String &msg) { (void)msg; }
  // Called when WiFi credentials are received via BLE. Default no-op.
  virtual void onWiFiCredentialsReceived(const String &ssid, const String &password) { (void)ssid; (void)password; }
  // Called when LED effect is changed via BLE. Default no-op.
  virtual void onLedEffectChanged(uint8_t newEffect) { (void)newEffect; }
  // Called when LED color is changed via BLE. Default no-op.
  virtual void onLedColorChanged(uint8_t r, uint8_t g, uint8_t b) { (void)r; (void)g; (void)b; }
  // Called when LED brightness is changed via BLE. Default no-op.
  virtual void onLedBrightnessChanged(uint8_t brightness) { (void)brightness; }
};

class BLEConfigServer {
public:
  BLEConfigServer();
  void begin(ISGConfigListener *listener = nullptr);
  void loop();

  // Accessors
  float speedMultiplier() const { return _speedMultiplier; }
  int currentPattern() const { return _currentPattern; }

  // Mutators (will also update BLE characteristics + notify clients)
  void setSpeedMultiplier(float v);
  void setCurrentPattern(int p);
  void setAutoMode(bool m);
  void setRunState(bool r);
  void setLedEffect(uint8_t e);
  void setLedColor(uint8_t r, uint8_t g, uint8_t b);
  void setLedBrightness(uint8_t brightness);
  void notifyStatus(const String &msg);
  void notifyTelemetry(const String &msg);
  void notifyWiFiStatus(const String &msg);
  // Manual controls (invoked via command characteristic tokens or future API)
  void restartAdvertising(const char *reasonTag = nullptr);
  void disconnectAll(const char *reasonTag = nullptr);

private:
  class SpeedCallbacks : public NimBLECharacteristicCallbacks {
  public:
    SpeedCallbacks(BLEConfigServer *parent) : _parent(parent) {}
    void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) override; // signature per NimBLE-Arduino 2.x
  private:
    BLEConfigServer *_parent;
  };
  class PatternCallbacks : public NimBLECharacteristicCallbacks {
  public:
    PatternCallbacks(BLEConfigServer *parent) : _parent(parent) {}
    void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) override;
  private:
    BLEConfigServer *_parent;
  };
  class ModeCallbacks : public NimBLECharacteristicCallbacks {
  public:
    ModeCallbacks(BLEConfigServer *parent) : _parent(parent) {}
    void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) override;
  private:
    BLEConfigServer *_parent;
  };
  class RunCallbacks : public NimBLECharacteristicCallbacks {
  public:
    RunCallbacks(BLEConfigServer *parent) : _parent(parent) {}
    void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) override;
  private:
    BLEConfigServer *_parent;
  };
  class CommandCallbacks : public NimBLECharacteristicCallbacks {
  public:
    CommandCallbacks(BLEConfigServer *parent) : _parent(parent) {}
    void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) override;
  private:
    BLEConfigServer *_parent;
  };
  class ScriptCallbacks : public NimBLECharacteristicCallbacks {
  public:
    ScriptCallbacks(BLEConfigServer *parent) : _parent(parent) {}
    void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) override;
  private:
    BLEConfigServer *_parent;
  };
  class WiFiSSIDCallbacks : public NimBLECharacteristicCallbacks {
  public:
    WiFiSSIDCallbacks(BLEConfigServer *parent) : _parent(parent) {}
    void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) override;
  private:
    BLEConfigServer *_parent;
  };
  class WiFiPasswordCallbacks : public NimBLECharacteristicCallbacks {
  public:
    WiFiPasswordCallbacks(BLEConfigServer *parent) : _parent(parent) {}
    void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) override;
  private:
    BLEConfigServer *_parent;
  };
  class LedEffectCallbacks : public NimBLECharacteristicCallbacks {
  public:
    LedEffectCallbacks(BLEConfigServer *parent) : _parent(parent) {}
    void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) override;
  private:
    BLEConfigServer *_parent;
  };
  class LedColorCallbacks : public NimBLECharacteristicCallbacks {
  public:
    LedColorCallbacks(BLEConfigServer *parent) : _parent(parent) {}
    void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) override;
  private:
    BLEConfigServer *_parent;
  };
  class LedBrightnessCallbacks : public NimBLECharacteristicCallbacks {
  public:
    LedBrightnessCallbacks(BLEConfigServer *parent) : _parent(parent) {}
    void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) override;
  private:
    BLEConfigServer *_parent;
  };

  void _applySpeedWrite(const std::string &valRaw);
  void _applyPatternWrite(const std::string &valRaw);
  void _applyModeWrite(const std::string &valRaw);
  void _applyRunWrite(const std::string &valRaw);
  void _applyCommandWrite(const std::string &valRaw);
  void _applyWiFiSSIDWrite(const std::string &valRaw);
  void _applyWiFiPasswordWrite(const std::string &valRaw);
  void _applyLedEffectWrite(const std::string &valRaw);
  void _applyLedColorWrite(const std::string &valRaw);
  void _applyLedBrightnessWrite(const std::string &valRaw);
  void _handleScriptCommand(const std::string &token, const std::string &payload);
  void _resetScriptTransfer(const char *reasonTag, bool notify = true);
  void _finalizeScriptTransfer();

  NimBLEServer *_server = nullptr;
  NimBLEService *_service = nullptr;
  NimBLECharacteristic *_speedChar = nullptr;
  NimBLECharacteristic *_patternChar = nullptr;
  NimBLECharacteristic *_statusChar = nullptr;    // notify/read
  NimBLECharacteristic *_modeChar = nullptr;      // read/write/notify
  NimBLECharacteristic *_runChar = nullptr;       // read/write/notify
  NimBLECharacteristic *_telemetryChar = nullptr; // notify/read
  NimBLECharacteristic *_commandChar = nullptr;   // write/read
  NimBLECharacteristic *_scriptChar = nullptr;    // write-only chunk sink
  NimBLECharacteristic *_wifiSSIDChar = nullptr;     // WiFi SSID (write/read)
  NimBLECharacteristic *_wifiPasswordChar = nullptr; // WiFi password (write-only)
  NimBLECharacteristic *_wifiStatusChar = nullptr;   // WiFi status (read/notify)
  NimBLECharacteristic *_ledEffectChar = nullptr;    // LED effect (write/read, 0 to NUM_PATTERN_LED_EFFECTS-1)
  NimBLECharacteristic *_ledColorChar = nullptr;     // LED color (write/read, "R,G,B")
  NimBLECharacteristic *_ledBrightnessChar = nullptr; // LED brightness (write/read, 0-255)

  ISGConfigListener *_listener = nullptr;

  float _speedMultiplier = 1.0f; // default neutral multiplier
  int _currentPattern = 1;       // maps to sketch patterns (1-indexed like existing code)
  bool _autoMode = true;
  bool _runState = false;
  uint8_t _ledEffect = 0;        // current LED effect (0 to NUM_PATTERN_LED_EFFECTS-1)
  uint8_t _ledColorR = 255;      // LED solid color red (0-255)
  uint8_t _ledColorG = 255;      // LED solid color green (0-255)
  uint8_t _ledColorB = 255;      // LED solid color blue (0-255)
  uint8_t _ledBrightness = 100;  // LED brightness (0-255, default 100)

  // Server connection lifecycle callbacks (restart advertising after disconnect)
  class ServerCallbacks : public NimBLEServerCallbacks {
  public:
    ServerCallbacks(BLEConfigServer *parent) : _parent(parent) {}
    void onConnect(NimBLEServer *server, NimBLEConnInfo &connInfo) override {
      (void)server; (void)connInfo;
      // Optional: could lower TX power or adjust parameters here.
      if (_parent && _parent->_statusChar) {
        _parent->notifyStatus(String("[BLE] CONNECT conn=") + String(connInfo.getConnHandle()));
      }
      if (_parent) {
        uint16_t h = connInfo.getConnHandle();
        bool found=false; for(auto v : _parent->_connHandles){ if(v==h){found=true;break;} }
        if(!found) _parent->_connHandles.push_back(h);
      }
    }
    void onDisconnect(NimBLEServer *server, NimBLEConnInfo &connInfo, int reason) override {
      (void)server;
      if (_parent && _parent->_statusChar) {
        _parent->notifyStatus(String("[BLE] DISCONNECT reason=") + String(reason));
      }
      if (_parent) {
        // Remove handle from list
        uint16_t h = connInfo.getConnHandle();
        auto &vec = _parent->_connHandles;
        vec.erase(std::remove(vec.begin(), vec.end(), h), vec.end());
        _parent->restartAdvertising("disc");
      }
    }
    void onMTUChange(uint16_t MTU, NimBLEConnInfo &connInfo) override {
      (void)connInfo;
      if (_parent && _parent->_statusChar) {
        _parent->notifyStatus(String("[BLE] MTU=") + String(MTU));
      }
    }
  private:
    BLEConfigServer *_parent;
  };
  ServerCallbacks *_serverCallbacks = nullptr; // keep pointer to manage lifetime

  // Internal helpers
  // (moved) _applyCommandWrite declared only once here (removed earlier duplicate)
  void _watchdog();

  std::vector<uint16_t> _connHandles; // active connections
  uint32_t _lastAdvAttemptMs = 0;     // throttle advertising restart attempts
  std::string _scriptBuffer;
  size_t _scriptExpectedLen = 0;
  size_t _scriptReceivedLen = 0;
  int _scriptTargetSlot = -1;
  bool _scriptActive = false;
  uint32_t _scriptLastChunkMs = 0;
  bool _scriptProgressDirty = false;
  uint32_t _scriptLastProgressNotifyMs = 0;

  // WiFi credential storage
  String _wifiSSID = "";
  String _wifiPassword = "";
};
