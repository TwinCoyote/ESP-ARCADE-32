// #include "../services./wifi_service.h"
#include "wifi_service.h"
#include <WiFi.h>
#include "ota/OTA.h"

// static const char *OTA_CURRENT_VERSION = "v1.0.5"; // TODO: Hacer que el valor lo tome de la variable en la nvs.

WiFiService::WiFiService(const char *ssid, const char *password) : _ota(OTAService::readVersion().c_str(), "TwinCoyote", "ESP-ARCADE-32")
{
    _ssid = ssid;
    _password = password;
    _isConnected = false;
    _otaChecked = false;
}

void WiFiService::begin()
{
    printMacAddress();

    bool hasCredentials = false;

    if (_prefs.begin("wifi_creds", false))
    {
        String savedSSID = _prefs.getString("ssid", "");
        String savedPASS = _prefs.getString("pass", "");

        if (savedSSID != "")
        {
            _ssid = savedSSID;
            _password = savedPASS;
            hasCredentials = true;
            Serial.print("Loaded saved WiFi SSID: ");
            Serial.println(_ssid);
        }
        else
        {
            for (int slot = 0; slot < 4 && !hasCredentials; slot++)
            {
                String keySSID = "ssid_" + String(slot);
                String keyPASS = "pass_" + String(slot);
                String NetWorkSaved = _prefs.getString(keySSID.c_str(), "");
                String PASSSaved = _prefs.getString(keyPASS.c_str(), "");

                if (NetWorkSaved != "")
                {
                    _ssid = NetWorkSaved;
                    _password = PASSSaved;
                    hasCredentials = true;
                    Serial.print("Loaded legacy WiFi SSID: ");
                    Serial.println(_ssid);
                }
            }

            if (!hasCredentials)
            {
                Serial.println("No stored WiFi credentials found.");
            }
        }
        _prefs.end();
    }
    else
    {
        Serial.println("Unable to open WiFi credentials storage; skipping auto-connect.");
    }

    if (hasCredentials)
    {
        xTaskCreatePinnedToCore(
            networkTaskProvider,
            "WiFi-Task",
            10240,
            this,
            1,
            &_networkTaskHandle,
            0);
    }
}

void WiFiService::connect(const char *ssid, const char *password)
{
    _ssid = ssid;
    _password = password;

    if (_ssid.length() == 0)
    {
        Serial.println("No SSID provided; skipping WiFi connect.");
        return;
    }

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
        10240,
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

// void WiFiService::networkLoop()
// {
//     WiFi.mode(WIFI_STA);
//     // WiFi.begin(_ssid, _password);
//     WiFi.begin(_ssid.c_str(), _password.c_str());

//     while (true)
//     {
//         update();

//         if (_isConnected && !_otaChecked)
//         {
//             Serial.println("[Core 0] ¡Wi-Fi Listo! Buscando actualizaciones de fondo...");

//             _otaChecked = true;
//             _ota.performUpdate();
//             Serial.println("[Core 0] Consola al día. Volviendo a tareas de red cotidianas.");
//         }
//         vTaskDelay(pdMS_TO_TICKS(100));
//     }

//     int attempts = 0;
//     const int MAX_ATTEMPTS = 20; // ~10 seconds

//     while (attempts < MAX_ATTEMPTS)
//     {
//         update();
//         if (isConnected())
//         {
//             Serial.println("WiFi connected!");
//             break;
//         }
//         vTaskDelay(pdMS_TO_TICKS(500));
//         attempts++;
//     }

//     if (!isConnected())
//     {
//         Serial.println("WiFi connection failed");
//         WiFi.disconnect();
//     }

//     vTaskDelay(pdMS_TO_TICKS(portMAX_DELAY)); // Keep task running but idle
// }

void WiFiService::networkLoop()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid.c_str(), _password.c_str());

    int attempts = 0;
    const int MAX_ATTEMPTS = 20;

    while (attempts < MAX_ATTEMPTS && !isConnected())
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        attempts++;
    }

    if (isConnected())
    {
        Serial.println("\n[Core 0] WiFi connected!");
        // Lee la versión guardada en NVS ahora que el sistema está listo
        String savedVersion = OTAService::readVersion();
        // Serial.print("NVS: savedVersion leida en networkLoop = ");
        // Serial.println(savedVersion);
        _ota.setVersion(savedVersion);
    }
    else
    {
        Serial.println("\n[Core 0] WiFi connection failed. Apagando antena.");
        WiFi.disconnect();
        vTaskDelete(NULL);
    }

    while (true)
    {
        update();

        if (_isConnected && !_otaChecked)
        {
            Serial.println("[Core 0] ¡Wi-Fi Listo! Buscando actualizaciones de fondo...");
            _otaChecked = true;
            _ota.performUpdate();
            Serial.println("[Core 0] Consola al día. Volviendo a tareas de red cotidianas.");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void WiFiService::networkTaskProvider(void *pvParameters)
{
    WiFiService *serviceInstance = (WiFiService *)pvParameters;
    serviceInstance->networkLoop();
}

void WiFiService::connectToNewNetwork(const char *ssid, const char *password)
{
    _ssid = ssid;
    _password = password;

    _prefs.begin("wifi_creds", false);

    _prefs.putString("ssid", _ssid);
    _prefs.putString("pass", _password);

    _prefs.end();
    Serial.println("Nuevas credenciales guardadas con éxito en la Flash.");
    WiFi.disconnect();
    WiFi.begin(_ssid, _password);
}