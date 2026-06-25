#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#include <WiFi.h>
#include <vector>
#include <Preferences.h>
#include "ota/OTA.h"

class WiFiService
{
private:
    // const char *_ssid;
    // const char *_password;
    String _ssid;
    String _password;
    bool _isConnected;
    std::vector<String> _networksLists;
    Preferences _prefs;
    void printEncryptionType(int thisType);
    void printMacAddress();
    TaskHandle_t _networkTaskHandle = NULL;
    static void networkTaskProvider(void *pvParameters);
    void networkLoop();
    bool _otaChecked;
    OTAService _ota;

public:
    WiFiService(const char *ssid = "", const char *password = "");

    void begin();
    void connect(const char *ssid, const char *password);
    void disconnect();

    void update();

    bool isConnected();

    void listNetworks();
    void connectToNewNetwork(const char *ssid, const char *password);
    std::vector<String> &getNetworksList();
};

#endif
