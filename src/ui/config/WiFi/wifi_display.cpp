#include "wifi_display.h"
#include "../../../drivers/display/display.h"
#include "../../../drivers/time/millis.h"

WifiMenu::WifiMenu()
    : selectedNetworkIndex(0),
      currentState(WIFI_STATE_SCANNING),
      lastScanTime(0),
      keyboardPtr(nullptr),
      wifiServicePtr(nullptr)
{
}

WifiMenu::~WifiMenu()
{
    if (keyboardPtr != nullptr)
    {
        delete keyboardPtr;
        keyboardPtr = nullptr;
    }
}

void WifiMenu::init(WiFiService *wifiService)
{
    wifiServicePtr = wifiService;
    currentState = WIFI_STATE_SCANNING;
    selectedNetworkIndex = 0;
    networksList.clear();
    selectedSSID = "";
    enteredPassword = "";
    scanNetworks();
}

void WifiMenu::scanNetworks()
{
    unsigned long now = millis();
    if (now - lastScanTime < SCAN_INTERVAL && !networksList.empty())
    {
        return; // Skip if scanned recently
    }

    Serial.println("Scanning WiFi networks...");
    ClearDisplay();
    SetCustomFont(FONT_SMALL);
    SetMenuFont();
    DrawText(20, 30, "Scanning...");
    // TODO: Poner un bitmap de mi imagen cargando
    ActDisplay();

    networksList.clear();
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    int numNetworks = WiFi.scanNetworks();

    if (numNetworks == -1)
    {
        Serial.println("WiFi scan failed");
        return;
    }

    Serial.print("Found networks: ");
    Serial.println(numNetworks);

    for (int i = 0; i < numNetworks; i++)
    {
        String ssid = WiFi.SSID(i);
        // Avoid duplicates
        bool isDuplicate = false;
        for (const auto &net : networksList)
        {
            if (net == ssid)
            {
                isDuplicate = true;
                break;
            }
        }
        if (!isDuplicate && ssid.length() > 0)
        {
            networksList.push_back(ssid);
            Serial.print("  ");
            Serial.println(ssid);
        }
    }

    lastScanTime = now;

    if (!networksList.empty())
    {
        currentState = WIFI_STATE_SELECT_NETWORK;
        selectedNetworkIndex = 0;
    }
}

void WifiMenu::renderNetworkList()
{
    ClearDisplay();
    SetMenuFont();

    // Title

    DrawText(5, 10, "Select WiFi Network:");

    if (networksList.empty())
    {
        DrawText(5, 30, "No networks found");
        DrawText(5, 50, "Press OK to rescan");
        ActDisplay();
        return;
    }

    // Show up to 4 networks at a time
    int startIdx = (selectedNetworkIndex / 4) * 4;
    int endIdx = (startIdx + 4 < networksList.size()) ? startIdx + 4 : networksList.size();

    for (int i = startIdx; i < endIdx; i++)
    {
        int posY = 22 + (i - startIdx) * 12;

        if (i == selectedNetworkIndex)
        {
            DrawBox(0, posY - 10, 6, 10);
        }

        DrawText(10, posY, networksList[i].c_str());
    }

    // Instructions

    ActDisplay();
    millis(50);
}

void WifiMenu::renderPasswordEntry()
{
    if (keyboardPtr == nullptr)
    {
        keyboardPtr = new VirtualKeyboard(10, 20);
    }

    ClearDisplay();
    SetMenuFont();

    // Show selected SSID
    DrawText(5, 10, "Network: ");
    DrawText(45, 10, selectedSSID.c_str());

    // Draw password prompt
    DrawText(5, 28, "Password:");
    DrawText(5, 40, enteredPassword.c_str());

    // Draw keyboard
    keyboardPtr->render();

    // Instructions
    // DrawText(5, 55, "BACK to cancel");

    ActDisplay();
    millis(50);
}

void WifiMenu::renderConnecting()
{
    ClearDisplay();
    SetMenuFont();

    DrawText(15, 20, "Connecting to:");
    DrawText(10, 35, selectedSSID.c_str());
    DrawText(20, 50, "Please wait...");

    ActDisplay();
    millis(100);
}

void WifiMenu::handleNetworkSelection()
{
    int dir = input.realDirection();

    if (dir == 1) // Right/Down
    {
        selectedNetworkIndex++;
        if (selectedNetworkIndex >= networksList.size())
            selectedNetworkIndex = 0;
    }
    else if (dir == 2) // Left/Up
    {
        if (selectedNetworkIndex == 0)
            selectedNetworkIndex = networksList.size() - 1;
        else
            selectedNetworkIndex--;
    }

    if (isPressed(BTN_OK))
    {
        delay(200); // Debounce
        selectedSSID = networksList[selectedNetworkIndex];
        enteredPassword = "";
        currentState = WIFI_STATE_ENTER_PASSWORD;
        Serial.print("Selected network: ");
        Serial.println(selectedSSID);
    }
}

void WifiMenu::handlePasswordEntry()
{
    bool okDown = isPressed(BTN_OK);
    int dir = input.realDirection();

    if (dir != 5 && keyboardPtr != nullptr)
    {
        keyboardPtr->handleInput(dir);
    }

    if (okDown && keyboardPtr != nullptr)
    {
        keyboardPtr->handleInput(5);
        if (keyboardPtr->consumeSubmit())
        {
            enteredPassword = keyboardPtr->getWord();
            Serial.print("Password entered: ");
            Serial.println(enteredPassword);
            if (enteredPassword.length() > 0)
            {
                currentState = WIFI_STATE_CONNECTING;
                Serial.println("Attempting to connect...");
            }
        }
    }

    if (isPressed(BTN_BACK))
    {
        delay(200); // Debounce
        currentState = WIFI_STATE_SELECT_NETWORK;
        if (keyboardPtr != nullptr)
        {
            delete keyboardPtr;
            keyboardPtr = nullptr;
        }
        Serial.println("Back to network selection");
    }
}

void WifiMenu::update()
{
    switch (currentState)
    {
    case WIFI_STATE_SCANNING:
        scanNetworks();
        break;

    case WIFI_STATE_SELECT_NETWORK:
        handleNetworkSelection();
        break;

    case WIFI_STATE_ENTER_PASSWORD:
        handlePasswordEntry();
        break;

    case WIFI_STATE_CONNECTING:
        // WiFi connection happens in WiFiService
        // For now, just show connecting screen
        if (isConnected())
        {
            currentState = WIFI_STATE_SELECT_NETWORK;
            Serial.println("Connected!");
        }
        break;
    }
}

void WifiMenu::render()
{
    switch (currentState)
    {
    case WIFI_STATE_SCANNING:
    {
        ClearDisplay();
        SetMenuFont();
        DrawText(20, 30, "Scanning...");
        ActDisplay();
        break;
    }

    case WIFI_STATE_SELECT_NETWORK:
        renderNetworkList();
        break;

    case WIFI_STATE_ENTER_PASSWORD:
        renderPasswordEntry();
        break;

    case WIFI_STATE_CONNECTING:
        renderConnecting();
        break;
    }
}

bool WifiMenu::isConnected()
{
    if (wifiServicePtr != nullptr)
    {
        return wifiServicePtr->isConnected();
    }
    return WiFi.status() == WL_CONNECTED;
}

void WifiMenu::reset()
{
    currentState = WIFI_STATE_SCANNING;
    selectedNetworkIndex = 0;
    networksList.clear();
    selectedSSID = "";
    enteredPassword = "";
    lastScanTime = 0;

    if (keyboardPtr != nullptr)
    {
        delete keyboardPtr;
        keyboardPtr = nullptr;
    }
}