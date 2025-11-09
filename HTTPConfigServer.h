// ---------------------------------------------------------------------------------------------------------------------
// HTTPConfigServer.h - HTTP/REST API server for Sand Garden
// Replaces BLE control characteristics with HTTP endpoints
// Uses AsyncWebServer for efficient async operations and Server-Sent Events (SSE) for real-time updates
// ---------------------------------------------------------------------------------------------------------------------
#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <vector>
#include <algorithm>

// LED effects configuration
#define NUM_PATTERN_LED_EFFECTS 14  // Total number of LED effects available

// Callback interface for host sketch to observe updates (same as BLE)
class ISGConfigListener {
public:
  virtual void onSpeedMultiplierChanged(float newValue) = 0;
  virtual void onCurrentPatternChanged(int newPattern) = 0;
  virtual void onAutoModeChanged(bool newAutoMode) = 0;
  virtual void onRunStateChanged(bool newRunState) = 0;
  virtual void onCommandReceived(const String &cmd, const std::string &raw) { (void)cmd; (void)raw; }
  virtual void onPatternScriptReceived(const std::string &script, int slotIndex) { (void)script; (void)slotIndex; }
  virtual void onPatternScriptStatus(const String &msg) { (void)msg; }
  virtual void onLedEffectChanged(uint8_t newEffect) { (void)newEffect; }
  virtual void onLedColorChanged(uint8_t r, uint8_t g, uint8_t b) { (void)r; (void)g; (void)b; }
  virtual void onLedBrightnessChanged(uint8_t brightness) { (void)brightness; }
};

class HTTPConfigServer {
public:
  HTTPConfigServer();
  ~HTTPConfigServer();
  void begin(ISGConfigListener *listener = nullptr, uint16_t port = 80);
  void end();
  void loop();

  // Accessors
  float speedMultiplier() const { return _speedMultiplier; }
  int currentPattern() const { return _currentPattern; }

  // Mutators (will broadcast to connected SSE clients)
  void setSpeedMultiplier(float v);
  void setCurrentPattern(int p);
  void setAutoMode(bool m);
  void setRunState(bool r);
  void setLedEffect(uint8_t e);
  void setLedColor(uint8_t r, uint8_t g, uint8_t b);
  void setLedBrightness(uint8_t brightness);
  void notifyStatus(const String &msg);
  void notifyTelemetry(const String &msg);

private:
  void _setupRoutes();
  void _sendError(AsyncWebServerRequest *request, int code, const String &error);
  void _handleGetState(AsyncWebServerRequest *request);
  void _handleSetSpeed(AsyncWebServerRequest *request, uint8_t *data, size_t len);
  void _handleSetPattern(AsyncWebServerRequest *request, uint8_t *data, size_t len);
  void _handleSetMode(AsyncWebServerRequest *request, uint8_t *data, size_t len);
  void _handleSetRun(AsyncWebServerRequest *request, uint8_t *data, size_t len);
  void _handleCommand(AsyncWebServerRequest *request, uint8_t *data, size_t len);
  void _handleScriptBegin(AsyncWebServerRequest *request, uint8_t *data, size_t len);
  void _handleScriptChunk(AsyncWebServerRequest *request, uint8_t *data, size_t len);
  void _handleScriptEnd(AsyncWebServerRequest *request);
  void _handleLedEffect(AsyncWebServerRequest *request, uint8_t *data, size_t len);
  void _handleLedColor(AsyncWebServerRequest *request, uint8_t *data, size_t len);
  void _handleLedBrightness(AsyncWebServerRequest *request, uint8_t *data, size_t len);
  void _handleReset(AsyncWebServerRequest *request);
  void _handleEvents(AsyncWebServerRequest *request);

  void _resetScriptTransfer(const char *reasonTag, bool notify = true);
  void _finalizeScriptTransfer();
  void _broadcastSSE(const String &event, const String &data);

  AsyncWebServer *_server = nullptr;
  AsyncEventSource *_events = nullptr;
  ISGConfigListener *_listener = nullptr;

  float _speedMultiplier = 1.0f;
  int _currentPattern = 1;
  bool _autoMode = true;
  bool _runState = false;
  uint8_t _ledEffect = 0;
  uint8_t _ledColorR = 255;
  uint8_t _ledColorG = 255;
  uint8_t _ledColorB = 255;
  uint8_t _ledBrightness = 100;

  // Script upload state
  std::string _scriptBuffer;
  size_t _scriptExpectedLen = 0;
  size_t _scriptReceivedLen = 0;
  int _scriptTargetSlot = -1;
  bool _scriptActive = false;
  uint32_t _scriptLastChunkMs = 0;
};
