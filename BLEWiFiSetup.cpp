#include "BLEWiFiSetup.h"

BLEWiFiSetup::BLEWiFiSetup() {}

void BLEWiFiSetup::begin(IWiFiSetupListener *listener) {
  _listener = listener;

  NimBLEDevice::init(SG_DEVICE_NAME);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);

  _server = NimBLEDevice::createServer();
  if (!_serverCallbacks) {
    _serverCallbacks = new ServerCallbacks(this);
  }
  _server->setCallbacks(_serverCallbacks);

  _service = _server->createService(SG_WIFI_SERVICE_UUID);

  // WiFi SSID characteristic (write/read for network name)
  _wifiSSIDChar = _service->createCharacteristic(
    SG_WIFI_SSID_CHAR_UUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  _wifiSSIDChar->setValue("");
  _wifiSSIDChar->setCallbacks(new WiFiSSIDCallbacks(this));

  // WiFi Password characteristic (write-only for security)
  _wifiPasswordChar = _service->createCharacteristic(
    SG_WIFI_PASS_CHAR_UUID,
    NIMBLE_PROPERTY::WRITE);
  _wifiPasswordChar->setValue("");
  _wifiPasswordChar->setCallbacks(new WiFiPasswordCallbacks(this));

  // WiFi Status characteristic (read/notify for connection status)
  _wifiStatusChar = _service->createCharacteristic(
    SG_WIFI_STATUS_CHAR_UUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  _wifiStatusChar->setValue("Disconnected");

  _service->start();

  NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
  NimBLEAdvertisementData advData;
  NimBLEAdvertisementData scanData;

  advData.setFlags(BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP);
  advData.setName(SG_DEVICE_NAME);
  scanData.addServiceUUID(_service->getUUID());
  adv->setAppearance(0x0000);
  adv->setAdvertisementData(advData);
  adv->setScanResponseData(scanData);
  adv->start();

  Serial.println("[BLE] WiFi Setup service started");
}

void BLEWiFiSetup::loop() {
  // Check if we need to restart advertising
  NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
  NimBLEServer *srv = NimBLEDevice::getServer();
  if (!adv || !srv) return;

  bool anyConn = srv->getConnectedCount() > 0;
  if (!anyConn && !adv->isAdvertising()) {
    _restartAdvertising();
  }
}

void BLEWiFiSetup::notifyWiFiStatus(const String &msg) {
  if (_wifiStatusChar) {
    _wifiStatusChar->setValue(std::string(msg.c_str()));
    _wifiStatusChar->notify();
  }
}

void BLEWiFiSetup::_applyWiFiSSIDWrite(const std::string &valRaw) {
  _wifiSSID = String(valRaw.c_str());
  if (_wifiSSIDChar) {
    _wifiSSIDChar->setValue(valRaw);
  }
  Serial.printf("[BLE] WiFi SSID set: %s\n", _wifiSSID.c_str());

  // If both SSID and password are set, trigger WiFi connection
  if (_wifiSSID.length() > 0 && _wifiPassword.length() > 0) {
    if (_listener) {
      _listener->onWiFiCredentialsReceived(_wifiSSID, _wifiPassword);
    }
  }
}

void BLEWiFiSetup::_applyWiFiPasswordWrite(const std::string &valRaw) {
  _wifiPassword = String(valRaw.c_str());
  Serial.println("[BLE] WiFi Password received");

  // If both SSID and password are set, trigger WiFi connection
  if (_wifiSSID.length() > 0 && _wifiPassword.length() > 0) {
    if (_listener) {
      _listener->onWiFiCredentialsReceived(_wifiSSID, _wifiPassword);
    }
  }
}

void BLEWiFiSetup::_restartAdvertising() {
  NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
  if (!adv) return;
  if (!NimBLEDevice::getServer() || NimBLEDevice::getServer()->getConnectedCount() > 0) return;

  uint32_t now = millis();
  if (now - _lastAdvAttemptMs < 2000) return;
  _lastAdvAttemptMs = now;

  Serial.println("[BLE] Restarting advertising");
  adv->start();
}

void BLEWiFiSetup::WiFiSSIDCallbacks::onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) {
  (void)info;
  _parent->_applyWiFiSSIDWrite(c->getValue());
}

void BLEWiFiSetup::WiFiPasswordCallbacks::onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) {
  (void)info;
  _parent->_applyWiFiPasswordWrite(c->getValue());
}

void BLEWiFiSetup::ServerCallbacks::onConnect(NimBLEServer *server, NimBLEConnInfo &connInfo) {
  (void)server; (void)connInfo;
  Serial.printf("[BLE] Client connected: %d\n", connInfo.getConnHandle());
}

void BLEWiFiSetup::ServerCallbacks::onDisconnect(NimBLEServer *server, NimBLEConnInfo &connInfo, int reason) {
  (void)server;
  Serial.printf("[BLE] Client disconnected: reason=%d\n", reason);
  _parent->_restartAdvertising();
}
