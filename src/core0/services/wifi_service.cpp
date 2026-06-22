#include "../services./wifi_service.h"
#include <WiFi.h>

WiFiService::WiFiService(const char *ssid, const char *password)
{
    _ssid = ssid;
    _password = password;
    _isConnected = false;
}

void WiFiService::begin()
{
    printMacAddress();
    // Don't start the network task immediately; wait for explicit connect() call
}

void WiFiService::connect(const char *ssid, const char *password)
{
    _ssid = ssid;
    _password = password;

    // Stop existing task if running
    if (_networkTaskHandle != NULL)
    {
        vTaskDelete(_networkTaskHandle);
        _networkTaskHandle = NULL;
    }

    // Start WiFi connection task
    xTaskCreatePinnedToCore(
        networkTaskProvider,
        "WiFi-Task",
        4096,
        this,
        1,
        &_networkTaskHandle,
        0);
}

void WiFiService::disconnect()
{
    if (_networkTaskHandle != NULL)
    {
        vTaskDelete(_networkTaskHandle);
        _networkTaskHandle = NULL;
    }
    WiFi.disconnect();
    _isConnected = false;
}

void WiFiService::printMacAddress()
{
    byte mac[6];

    //* print your MAC address:
    WiFi.macAddress(mac);
    Serial.print("MAC: ");
    Serial.print(mac[5], HEX);
    Serial.print(":");
    Serial.print(mac[4], HEX);
    Serial.print(":");
    Serial.print(mac[3], HEX);
    Serial.print(":");
    Serial.print(mac[2], HEX);
    Serial.print(":");
    Serial.print(mac[1], HEX);
    Serial.print(":");
    Serial.println(mac[0], HEX);
}

void WiFiService::listNetworks()
{
    Serial.println("** Scan Networks **");
    int numSsid = WiFi.scanNetworks();
    if (numSsid == -1)
    {
        Serial.println("Couldn't get a WiFi connection");
        return;
    }

    Serial.print("number of available networks:");
    Serial.println(numSsid);

    _networksLists.clear();
    for (int thisNet = 0; thisNet < numSsid; thisNet++)
    {
        _networksLists.push_back(WiFi.SSID(thisNet));
    }
}

std::vector<String> &WiFiService::getNetworksList()
{
    return _networksLists;
}

void WiFiService::printEncryptionType(int thisType)
{
    switch (thisType)
    {
    case WIFI_AUTH_WEP:
        Serial.println("WEP");
        break;
    case WIFI_AUTH_WPA_PSK:
        Serial.println("WPA");
        break;
    case WIFI_AUTH_WPA2_PSK:
        Serial.println("WPA2");
        break;
    case WIFI_AUTH_OPEN:
        Serial.println("None");
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        Serial.println("WPA/WPA2");
        break;
    default:
        Serial.println("Unknown");
        break;
    }
}

bool WiFiService::isConnected()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        digitalWrite(2, HIGH);
        return true;
    }
    else
    {
        digitalWrite(2, LOW);
        return false;
    }
}

void WiFiService::update()
{
    if (isConnected())
    {
        _isConnected = true;
    }
    else
    {
        _isConnected = false;
    }
}

void WiFiService::networkLoop()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);

    int attempts = 0;
    const int MAX_ATTEMPTS = 20; // ~10 seconds

    while (attempts < MAX_ATTEMPTS)
    {
        update();
        if (isConnected())
        {
            Serial.println("WiFi connected!");
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
        attempts++;
    }

    if (!isConnected())
    {
        Serial.println("WiFi connection failed");
        WiFi.disconnect();
    }

    vTaskDelay(pdMS_TO_TICKS(portMAX_DELAY)); // Keep task running but idle
}

void WiFiService::networkTaskProvider(void *pvParameters)
{
    WiFiService *serviceInstance = (WiFiService *)pvParameters;
    serviceInstance->networkLoop();
}