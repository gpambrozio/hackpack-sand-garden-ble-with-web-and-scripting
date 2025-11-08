#include "HTTPConfigServer.h"
#include "PatternScript.h"
#include <ArduinoJson.h>

static const uint32_t SCRIPT_TRANSFER_TIMEOUT_MS = 5000;

HTTPConfigServer::HTTPConfigServer() {}

HTTPConfigServer::~HTTPConfigServer() {
  end();
  if (_server) {
    delete _server;
    _server = nullptr;
  }
  if (_events) {
    delete _events;
    _events = nullptr;
  }
}

void HTTPConfigServer::begin(ISGConfigListener *listener, uint16_t port) {
  _listener = listener;
  _server = new AsyncWebServer(port);
  _events = new AsyncEventSource("/api/events");

  // Enable CORS for all origins (web client can be served from anywhere)
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

  _setupRoutes();
  _server->begin();

  Serial.printf("[HTTP] Server started on port %d\n", port);
}

void HTTPConfigServer::loop() {
  // Check for script transfer timeout
  if (_scriptActive && _scriptLastChunkMs > 0) {
    uint32_t now = millis();
    if ((now - _scriptLastChunkMs) > SCRIPT_TRANSFER_TIMEOUT_MS) {
      String msg = String("[SCRIPT] ERR timeout after ") + String(now - _scriptLastChunkMs) + "ms";
      notifyStatus(msg);
      if (_listener) _listener->onPatternScriptStatus(msg);
      _resetScriptTransfer("timeout", false);
    }
  }
}

void HTTPConfigServer::end() {
  if (_events) {
    _events->close();
  }
  if (_server) {
    _server->end();
  }
  Serial.println("[HTTP] Server stopped");
}

void HTTPConfigServer::_sendError(AsyncWebServerRequest *request, int code, const String &error) {
  StaticJsonDocument<128> doc;
  doc["error"] = error;
  String json;
  serializeJson(doc, json);
  request->send(code, "application/json", json);
}

void HTTPConfigServer::_setupRoutes() {
  // OPTIONS handler for CORS preflight - catch all API routes
  _server->onNotFound([](AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
      // Only handle OPTIONS for /api/* routes
      String path = request->url();
      if (path.startsWith("/api/")) {
        request->send(200);
      } else {
        request->send(404, "text/plain", "Not Found");
      }
    } else {
      request->send(404, "text/plain", "Not Found");
    }
  });

  // GET /api/state - Return all current values as JSON
  _server->on("/api/state", HTTP_GET, [this](AsyncWebServerRequest *request) {
    this->_handleGetState(request);
  });

  // POST handlers
  _server->on("/api/speed", HTTP_POST, [](AsyncWebServerRequest *request) {},
    NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (index == 0) { this->_handleSetSpeed(request, data, len); }
    });

  _server->on("/api/pattern", HTTP_POST, [](AsyncWebServerRequest *request) {},
    NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (index == 0) { this->_handleSetPattern(request, data, len); }
    });

  _server->on("/api/mode", HTTP_POST, [](AsyncWebServerRequest *request) {},
    NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (index == 0) { this->_handleSetMode(request, data, len); }
    });

  _server->on("/api/run", HTTP_POST, [](AsyncWebServerRequest *request) {},
    NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (index == 0) { this->_handleSetRun(request, data, len); }
    });

  _server->on("/api/command", HTTP_POST, [](AsyncWebServerRequest *request) {},
    NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (index == 0) { this->_handleCommand(request, data, len); }
    });

  _server->on("/api/script/begin", HTTP_POST, [](AsyncWebServerRequest *request) {},
    NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (index == 0) { this->_handleScriptBegin(request, data, len); }
    });

  _server->on("/api/script/chunk", HTTP_POST, [](AsyncWebServerRequest *request) {},
    NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      // Process each chunk as it arrives
      this->_handleScriptChunk(request, data, len);
    });

  _server->on("/api/script/end", HTTP_POST, [this](AsyncWebServerRequest *request) {
    this->_handleScriptEnd(request);
  });

  _server->on("/api/led/effect", HTTP_POST, [](AsyncWebServerRequest *request) {},
    NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (index == 0) { this->_handleLedEffect(request, data, len); }
    });

  _server->on("/api/led/color", HTTP_POST, [](AsyncWebServerRequest *request) {},
    NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (index == 0) { this->_handleLedColor(request, data, len); }
    });

  _server->on("/api/led/brightness", HTTP_POST, [](AsyncWebServerRequest *request) {},
    NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (index == 0) { this->_handleLedBrightness(request, data, len); }
    });

  // Server-Sent Events endpoint
  _events->onConnect([this](AsyncEventSourceClient *client) {
    Serial.printf("[HTTP] SSE client connected, ID: %u\n", client->lastId());
    // Send initial state to new client
    StaticJsonDocument<256> doc;
    doc["speed"] = _speedMultiplier;
    doc["pattern"] = _currentPattern;
    doc["mode"] = _autoMode ? 1 : 0;
    doc["run"] = _runState ? 1 : 0;
    doc["ledEffect"] = _ledEffect;
    doc["ledBrightness"] = _ledBrightness;
    String json;
    serializeJson(doc, json);
    client->send(json.c_str(), "state", millis());
  });

  _server->addHandler(_events);
}

void HTTPConfigServer::_handleGetState(AsyncWebServerRequest *request) {
  StaticJsonDocument<512> doc;
  doc["speedMultiplier"] = _speedMultiplier;
  doc["pattern"] = _currentPattern;
  doc["autoMode"] = _autoMode;
  doc["running"] = _runState;
  doc["ledEffect"] = _ledEffect;
  doc["ledColorR"] = _ledColorR;
  doc["ledColorG"] = _ledColorG;
  doc["ledColorB"] = _ledColorB;
  doc["ledBrightness"] = _ledBrightness;

  String json;
  serializeJson(doc, json);
  request->send(200, "application/json", json);
}

void HTTPConfigServer::_handleSetSpeed(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
  StaticJsonDocument<128> doc;
  DeserializationError error = deserializeJson(doc, data, len);
  if (error) {
    request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  if (!doc.containsKey("value")) {
    request->send(400, "application/json", "{\"error\":\"Missing value field\"}");
    return;
  }

  float newValue = doc["value"];
  setSpeedMultiplier(newValue);
  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

void HTTPConfigServer::_handleSetPattern(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
  StaticJsonDocument<128> doc;
  DeserializationError error = deserializeJson(doc, data, len);
  if (error) {
    request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  if (!doc.containsKey("value")) {
    request->send(400, "application/json", "{\"error\":\"Missing value field\"}");
    return;
  }

  int newValue = doc["value"];
  setCurrentPattern(newValue);
  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

void HTTPConfigServer::_handleSetMode(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
  StaticJsonDocument<128> doc;
  DeserializationError error = deserializeJson(doc, data, len);
  if (error) {
    request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  if (!doc.containsKey("value")) {
    request->send(400, "application/json", "{\"error\":\"Missing value field\"}");
    return;
  }

  bool newValue = doc["value"];
  setAutoMode(newValue);
  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

void HTTPConfigServer::_handleSetRun(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
  StaticJsonDocument<128> doc;
  DeserializationError error = deserializeJson(doc, data, len);
  if (error) {
    request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  if (!doc.containsKey("value")) {
    request->send(400, "application/json", "{\"error\":\"Missing value field\"}");
    return;
  }

  bool newValue = doc["value"];
  setRunState(newValue);
  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

void HTTPConfigServer::_handleCommand(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, data, len);
  if (error) {
    request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  if (!doc.containsKey("command")) {
    request->send(400, "application/json", "{\"error\":\"Missing command field\"}");
    return;
  }

  String cmd = doc["command"].as<String>();
  cmd.trim();
  cmd.toUpperCase();

  notifyStatus(String("[CMD] RX ") + cmd);
  if (_listener) {
    _listener->onCommandReceived(cmd, cmd.c_str());
  }

  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

void HTTPConfigServer::_handleScriptBegin(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, data, len);
  if (error) {
    request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  if (!doc.containsKey("length")) {
    request->send(400, "application/json", "{\"error\":\"Missing length field\"}");
    return;
  }

  size_t expected = doc["length"];
  int slot = doc.containsKey("slot") ? doc["slot"].as<int>() : -1;

  if (expected <= 0 || expected > PSG_MAX_SCRIPT_CHARS) {
    String errMsg = String("{\"error\":\"Invalid length: ") + String(expected) + "\"}";
    request->send(400, "application/json", errMsg);
    return;
  }

  if (_scriptActive || _scriptReceivedLen > 0) {
    _resetScriptTransfer("preempt", false);
  }

  _scriptBuffer.clear();
  _scriptBuffer.reserve(expected);
  _scriptExpectedLen = expected;
  _scriptReceivedLen = 0;
  _scriptTargetSlot = slot;
  _scriptActive = true;
  _scriptLastChunkMs = millis();

  String msg = String("[SCRIPT] BEGIN len=") + String(expected) + " slot=" + String(slot);
  notifyStatus(msg);
  if (_listener) _listener->onPatternScriptStatus(msg);

  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

void HTTPConfigServer::_handleScriptChunk(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
  if (!_scriptActive) {
    request->send(400, "application/json", "{\"error\":\"No active script transfer\"}");
    return;
  }

  if (_scriptReceivedLen + len > _scriptExpectedLen) {
    String errMsg = String("{\"error\":\"Chunk overflow\"}");
    request->send(400, "application/json", errMsg);
    _resetScriptTransfer("overflow", true);
    return;
  }

  _scriptBuffer.append((char*)data, len);
  _scriptReceivedLen += len;
  _scriptLastChunkMs = millis();

  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

void HTTPConfigServer::_handleScriptEnd(AsyncWebServerRequest *request) {
  if (!_scriptActive) {
    request->send(400, "application/json", "{\"error\":\"No active script transfer\"}");
    return;
  }

  if (_scriptReceivedLen != _scriptExpectedLen) {
    String errMsg = String("{\"error\":\"Size mismatch recv=") + String(_scriptReceivedLen) +
                    " exp=" + String(_scriptExpectedLen) + "\"}";
    request->send(400, "application/json", errMsg);
    _resetScriptTransfer("size", true);
    return;
  }

  _finalizeScriptTransfer();
  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

void HTTPConfigServer::_handleLedEffect(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
  StaticJsonDocument<128> doc;
  DeserializationError error = deserializeJson(doc, data, len);
  if (error) {
    _sendError(request, 400, "Invalid JSON");
    return;
  }

  if (!doc.containsKey("value")) {
    _sendError(request, 400, "Missing value field");
    return;
  }

  uint8_t newValue = doc["value"];
  if (newValue >= NUM_PATTERN_LED_EFFECTS) {
    _sendError(request, 400, "Invalid effect value");
    return;
  }
  setLedEffect(newValue);
  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

void HTTPConfigServer::_handleLedColor(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, data, len);
  if (error) {
    request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  if (!doc.containsKey("r") || !doc.containsKey("g") || !doc.containsKey("b")) {
    request->send(400, "application/json", "{\"error\":\"Missing r, g, or b field\"}");
    return;
  }

  uint8_t r = doc["r"];
  uint8_t g = doc["g"];
  uint8_t b = doc["b"];
  setLedColor(r, g, b);
  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

void HTTPConfigServer::_handleLedBrightness(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
  StaticJsonDocument<128> doc;
  DeserializationError error = deserializeJson(doc, data, len);
  if (error) {
    request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  if (!doc.containsKey("value")) {
    request->send(400, "application/json", "{\"error\":\"Missing value field\"}");
    return;
  }

  uint8_t newValue = doc["value"];
  setLedBrightness(newValue);
  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

void HTTPConfigServer::setSpeedMultiplier(float v) {
  if (v <= 0) v = 0.01f;
  if (fabsf(v - _speedMultiplier) < 0.0001f) return;
  _speedMultiplier = v;

  StaticJsonDocument<128> doc;
  doc["speed"] = _speedMultiplier;
  String json;
  serializeJson(doc, json);
  _broadcastSSE("speed", json);

  if (_listener) _listener->onSpeedMultiplierChanged(_speedMultiplier);
}

void HTTPConfigServer::setCurrentPattern(int p) {
  if (p < 1) p = 1;
  if (p == _currentPattern) return;
  _currentPattern = p;

  StaticJsonDocument<128> doc;
  doc["pattern"] = _currentPattern;
  String json;
  serializeJson(doc, json);
  _broadcastSSE("pattern", json);

  if (_listener) _listener->onCurrentPatternChanged(_currentPattern);
}

void HTTPConfigServer::setAutoMode(bool m) {
  if (_autoMode == m) return;
  _autoMode = m;

  StaticJsonDocument<128> doc;
  doc["mode"] = _autoMode ? 1 : 0;
  String json;
  serializeJson(doc, json);
  _broadcastSSE("mode", json);

  if (_listener) _listener->onAutoModeChanged(_autoMode);
}

void HTTPConfigServer::setRunState(bool r) {
  if (_runState == r) return;
  _runState = r;

  StaticJsonDocument<128> doc;
  doc["run"] = _runState ? 1 : 0;
  String json;
  serializeJson(doc, json);
  _broadcastSSE("run", json);

  if (_listener) _listener->onRunStateChanged(_runState);
}

void HTTPConfigServer::setLedEffect(uint8_t e) {
  // Validate range
  if (e >= NUM_PATTERN_LED_EFFECTS) {
    Serial.printf("[HTTP] Invalid LED effect: %d (max %d)\n", e, NUM_PATTERN_LED_EFFECTS - 1);
    return;
  }
  if (_ledEffect == e) return;
  _ledEffect = e;

  StaticJsonDocument<128> doc;
  doc["ledEffect"] = _ledEffect;
  String json;
  serializeJson(doc, json);
  _broadcastSSE("ledEffect", json);

  if (_listener) _listener->onLedEffectChanged(_ledEffect);
}

void HTTPConfigServer::setLedColor(uint8_t r, uint8_t g, uint8_t b) {
  if (_ledColorR == r && _ledColorG == g && _ledColorB == b) return;
  _ledColorR = r;
  _ledColorG = g;
  _ledColorB = b;

  StaticJsonDocument<128> doc;
  doc["r"] = r;
  doc["g"] = g;
  doc["b"] = b;
  String json;
  serializeJson(doc, json);
  _broadcastSSE("ledColor", json);

  if (_listener) _listener->onLedColorChanged(r, g, b);
}

void HTTPConfigServer::setLedBrightness(uint8_t brightness) {
  if (_ledBrightness == brightness) return;
  _ledBrightness = brightness;

  StaticJsonDocument<128> doc;
  doc["ledBrightness"] = _ledBrightness;
  String json;
  serializeJson(doc, json);
  _broadcastSSE("ledBrightness", json);

  if (_listener) _listener->onLedBrightnessChanged(_ledBrightness);
}

void HTTPConfigServer::notifyStatus(const String &msg) {
  _broadcastSSE("status", msg);
}

void HTTPConfigServer::notifyTelemetry(const String &msg) {
  _broadcastSSE("telemetry", msg);
}

void HTTPConfigServer::_broadcastSSE(const String &event, const String &data) {
  if (_events) {
    _events->send(data.c_str(), event.c_str(), millis());
  }
}

void HTTPConfigServer::_resetScriptTransfer(const char *reasonTag, bool notify) {
  bool hadProgress = _scriptActive || _scriptReceivedLen > 0;
  if (notify && hadProgress) {
    String msg = String("[SCRIPT] RESET reason=") + (reasonTag ? reasonTag : "?");
    notifyStatus(msg);
    if (_listener) _listener->onPatternScriptStatus(msg);
  }
  _scriptBuffer.clear();
  _scriptExpectedLen = 0;
  _scriptReceivedLen = 0;
  _scriptTargetSlot = -1;
  _scriptActive = false;
  _scriptLastChunkMs = 0;
}

void HTTPConfigServer::_finalizeScriptTransfer() {
  if (!_scriptActive) {
    notifyStatus("[SCRIPT] ERR finalize inactive");
    if (_listener) _listener->onPatternScriptStatus("[SCRIPT] ERR finalize inactive");
    return;
  }

  std::string script;
  script.swap(_scriptBuffer);
  size_t len = _scriptExpectedLen;
  int slot = _scriptTargetSlot;

  _scriptExpectedLen = 0;
  _scriptReceivedLen = 0;
  _scriptTargetSlot = -1;
  _scriptActive = false;
  _scriptLastChunkMs = 0;

  String msg = String("[SCRIPT] READY len=") + String(len);
  notifyStatus(msg);
  if (_listener) {
    _listener->onPatternScriptStatus(msg);
    _listener->onPatternScriptReceived(script, slot);
  }
}
