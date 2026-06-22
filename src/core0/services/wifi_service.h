#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#include <WiFi.h>
#include <vector>

class WiFiService
{
private:
    const char *_ssid;
    const char *_password;
    bool _isConnected;
    std::vector<String> _networksLists;
    void printEncryptionType(int thisType);
    void printMacAddress();
    TaskHandle_t _networkTaskHandle = NULL;
    static void networkTaskProvider(void *pvParameters);
    void networkLoop();

public:
    WiFiService(const char *ssid = "", const char *password = "");

    void begin();
    void connect(const char *ssid, const char *password);
    void disconnect();

    void update();

    bool isConnected();

    void listNetworks();
    std::vector<String> &getNetworksList();
};

#endif
