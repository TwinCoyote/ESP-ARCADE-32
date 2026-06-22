#ifndef WIFI_DISPLAY_H
#define WIFI_DISPLAY_H

#include <Arduino.h>
#include <vector>
#include "../../../drivers/display/display.h"
#include "../../../drivers/input/buttons.h"
#include "../../keyboard.h"
#include "../../../core0/services/wifi_service.h"

enum WifiMenuState
{
    WIFI_STATE_SCANNING,
    WIFI_STATE_SELECT_NETWORK,
    WIFI_STATE_ENTER_PASSWORD,
    WIFI_STATE_CONNECTING
};

class WifiMenu
{
private:
    std::vector<String> networksList;
    unsigned int selectedNetworkIndex;
    WifiMenuState currentState;
    String selectedSSID;
    String enteredPassword;
    VirtualKeyboard *keyboardPtr;
    WiFiService *wifiServicePtr;
    unsigned long lastScanTime;
    const unsigned long SCAN_INTERVAL = 10000; // Rescan every 10 seconds

    void scanNetworks();
    void renderNetworkList();
    void renderPasswordEntry();
    void renderConnecting();
    void handleNetworkSelection();
    void handlePasswordEntry();

public:
    WifiMenu();
    ~WifiMenu();

    void init(WiFiService *wifiService);
    void update();
    void render();
    bool isConnected();
    void reset();
};

#endif