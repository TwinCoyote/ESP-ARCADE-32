#include "wifi_display.h"
#include "../../../drivers/display/display.h"
#include "../../../drivers/time/millis.h"
#include "../../../assets/images/scanning.h"
#include "../../../config/debug_log.h"

WifiMenu::WifiMenu()
    : selectedNetworkIndex(0),
      currentState(WIFI_STATE_SCANNING),
      lastScanTime(0),
      connectStartTime(0),
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
    connectStartTime = 0;
    scanNetworks();
}

void WifiMenu::scanNetworks()
{
    unsigned long now = millis();
    if (now - lastScanTime < SCAN_INTERVAL && !networksList.empty())
    {
        return; // Skip if scanned recently
    }

    DEV_PRINTLN("Scanning WiFi networks...");
    // Se queda en pantalla mientras WiFi.scanNetworks() busca redes
    DrawBitmap(ScanningBitmap, scanningWidth, scanningHeight);

    networksList.clear();
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    int numNetworks = WiFi.scanNetworks();

    if (numNetworks < 0) // WIFI_SCAN_FAILED es -2 (el -1 solo existe en modo asincrono)
    {
        DEV_PRINTLN("WiFi scan failed");
        numNetworks = 0; // se trata igual que si no hubiera redes
    }

    DEV_PRINT("Found networks: ");
    DEV_PRINTLN(numNetworks);

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
            DEV_PRINT("  ");
            DEV_PRINTLN(ssid);
        }
    }

    lastScanTime = now;

    // Aunque no haya redes se sale del escaneo: la lista vacia muestra "No networks found"
    // y OK vuelve a buscar (antes se quedaba escaneando en bucle)
    currentState = WIFI_STATE_SELECT_NETWORK;
    selectedNetworkIndex = 0;
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
    // DrawText(5, 10, "Network: ");
    // DrawText(45, 10, selectedSSID.c_str());

    // Draw password prompt
    DrawText(5, 10, "Psw:");
    DrawText(30, 12, keyboardPtr->getWord().c_str());

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

void WifiMenu::renderConnectionFailed()
{
    ClearDisplay();
    SetMenuFont();

    DrawText(10, 20, "Connection failed");
    DrawText(5, 35, selectedSSID.c_str());
    DrawText(5, 50, "Press OK or BACK");
    DrawText(5, 60, "to retry");

    ActDisplay();
    millis(100);
}

void WifiMenu::handleNetworkSelection()
{
    // Sin redes solo OK hace algo: volver a buscar (asi tampoco se lee fuera de la lista vacia)
    if (networksList.empty())
    {
        if (isPressed(BTN_OK))
        {
            delay(200); // Debounce
            currentState = WIFI_STATE_SCANNING;
            DEV_PRINTLN("Rescanning WiFi networks...");
        }
        return;
    }

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
        DEV_PRINT("Selected network: ");
        DEV_PRINTLN(selectedSSID);
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
            DEV_PRINT("Password entered: ");
            DEV_PRINTLN(enteredPassword);
            if (enteredPassword.length() > 0)
            {
                currentState = WIFI_STATE_CONNECTING;
                connectStartTime = millis();
                DEV_PRINT("Connecting to SSID: ");
                DEV_PRINTLN(selectedSSID);
                DEV_PRINT("Using password: ");
                DEV_PRINTLN(enteredPassword);
                DEV_PRINTLN("Attempting to connect...");
                if (wifiServicePtr != nullptr)
                {
                    wifiServicePtr->connectToNewNetwork(selectedSSID.c_str(), enteredPassword.c_str());
                }
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
        DEV_PRINTLN("Back to network selection");
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
        if (isConnected())
        {
            currentState = WIFI_STATE_SELECT_NETWORK;
            DEV_PRINTLN("Connected!");
        }
        else if (millis() - connectStartTime >= CONNECT_TIMEOUT)
        {
            currentState = WIFI_STATE_CONNECTION_FAILED;
            DEV_PRINTLN("Connection failed");
        }
        break;
    case WIFI_STATE_CONNECTION_FAILED:
        if (isPressed(BTN_OK) || isPressed(BTN_BACK))
        {
            delay(200);
            currentState = WIFI_STATE_SELECT_NETWORK;
            if (keyboardPtr != nullptr)
            {
                delete keyboardPtr;
                keyboardPtr = nullptr;
            }
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
        DrawBitmap(ScanningBitmap, scanningWidth, scanningHeight);
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
    case WIFI_STATE_CONNECTION_FAILED:
        renderConnectionFailed();
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