// ---------------------------------------------------------------------------------------------------------------------
// BLEWiFiSetup.h - Minimal BLE service for WiFi credential configuration
// Only handles WiFi SSID, Password, and Status characteristics
// ---------------------------------------------------------------------------------------------------------------------
#pragma once
#include <Arduino.h>
#include <NimBLEDevice.h>

// BLE Service & Characteristic UUIDs
#define SG_WIFI_SERVICE_UUID        "9b6c7e10-3b2c-4d8c-9d7c-5e2a6d1f8b01"
#define SG_WIFI_SSID_CHAR_UUID      "9b6c7e19-3b2c-4d8c-9d7c-5e2a6d1f8b01"
#define SG_WIFI_PASS_CHAR_UUID      "9b6c7e1a-3b2c-4d8c-9d7c-5e2a6d1f8b01"
#define SG_WIFI_STATUS_CHAR_UUID    "9b6c7e1b-3b2c-4d8c-9d7c-5e2a6d1f8b01"

#define SG_DEVICE_NAME "Sand Garden"

// Callback interface for WiFi credential events
class IWiFiSetupListener {
public:
  virtual void onWiFiCredentialsReceived(const String &ssid, const String &password) = 0;
};

class BLEWiFiSetup {
public:
  BLEWiFiSetup();
  void begin(IWiFiSetupListener *listener = nullptr);
  void loop();
  void notifyWiFiStatus(const String &msg);

private:
  class WiFiSSIDCallbacks : public NimBLECharacteristicCallbacks {
  public:
    WiFiSSIDCallbacks(BLEWiFiSetup *parent) : _parent(parent) {}
    void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) override;
  private:
    BLEWiFiSetup *_parent;
  };

  class WiFiPasswordCallbacks : public NimBLECharacteristicCallbacks {
  public:
    WiFiPasswordCallbacks(BLEWiFiSetup *parent) : _parent(parent) {}
    void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) override;
  private:
    BLEWiFiSetup *_parent;
  };

  class ServerCallbacks : public NimBLEServerCallbacks {
  public:
    ServerCallbacks(BLEWiFiSetup *parent) : _parent(parent) {}
    void onConnect(NimBLEServer *server, NimBLEConnInfo &connInfo) override;
    void onDisconnect(NimBLEServer *server, NimBLEConnInfo &connInfo, int reason) override;
  private:
    BLEWiFiSetup *_parent;
  };

  void _applyWiFiSSIDWrite(const std::string &valRaw);
  void _applyWiFiPasswordWrite(const std::string &valRaw);
  void _restartAdvertising();

  NimBLEServer *_server = nullptr;
  NimBLEService *_service = nullptr;
  NimBLECharacteristic *_wifiSSIDChar = nullptr;
  NimBLECharacteristic *_wifiPasswordChar = nullptr;
  NimBLECharacteristic *_wifiStatusChar = nullptr;
  ServerCallbacks *_serverCallbacks = nullptr;
  IWiFiSetupListener *_listener = nullptr;

  String _wifiSSID = "";
  String _wifiPassword = "";
  uint32_t _lastAdvAttemptMs = 0;
};
