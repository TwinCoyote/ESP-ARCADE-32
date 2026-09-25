// #include "../services./wifi_service.h"
#include "wifi_service.h"
#include <WiFi.h>
#include "ota/OTA.h"
#include "../../config/debug_log.h"

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
            DEV_PRINT("Loaded saved WiFi SSID: ");
            DEV_PRINTLN(_ssid);
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
                    DEV_PRINT("Loaded legacy WiFi SSID: ");
                    DEV_PRINTLN(_ssid);
                }
            }

            if (!hasCredentials)
            {
                DEV_PRINTLN("No stored WiFi credentials found.");
            }
        }
        _prefs.end();
    }
    else
    {
        DEV_PRINTLN("Unable to open WiFi credentials storage; skipping auto-connect.");
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
        DEV_PRINTLN("No SSID provided; skipping WiFi connect.");
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
    DEV_PRINT("MAC: ");
    DEV_PRINT(mac[5], HEX);
    DEV_PRINT(":");
    DEV_PRINT(mac[4], HEX);
    DEV_PRINT(":");
    DEV_PRINT(mac[3], HEX);
    DEV_PRINT(":");
    DEV_PRINT(mac[2], HEX);
    DEV_PRINT(":");
    DEV_PRINT(mac[1], HEX);
    DEV_PRINT(":");
    DEV_PRINTLN(mac[0], HEX);
}

void WiFiService::listNetworks()
{
    DEV_PRINTLN("** Scan Networks **");
    int numSsid = WiFi.scanNetworks();
    if (numSsid == -1)
    {
        DEV_PRINTLN("Couldn't get a WiFi connection");
        return;
    }

    DEV_PRINT("number of available networks:");
    DEV_PRINTLN(numSsid);

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
        DEV_PRINTLN("WEP");
        break;
    case WIFI_AUTH_WPA_PSK:
        DEV_PRINTLN("WPA");
        break;
    case WIFI_AUTH_WPA2_PSK:
        DEV_PRINTLN("WPA2");
        break;
    case WIFI_AUTH_OPEN:
        DEV_PRINTLN("None");
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        DEV_PRINTLN("WPA/WPA2");
        break;
    default:
        DEV_PRINTLN("Unknown");
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
//             DEV_PRINTLN("[Core 0] ¡Wi-Fi Listo! Buscando actualizaciones de fondo...");

//             _otaChecked = true;
//             _ota.performUpdate();
//             DEV_PRINTLN("[Core 0] Consola al día. Volviendo a tareas de red cotidianas.");
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
//             DEV_PRINTLN("WiFi connected!");
//             break;
//         }
//         vTaskDelay(pdMS_TO_TICKS(500));
//         attempts++;
//     }

//     if (!isConnected())
//     {
//         DEV_PRINTLN("WiFi connection failed");
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
        DEV_PRINTLN("\n[Core 0] WiFi connected!");
        // Lee la versión guardada en NVS ahora que el sistema está listo
        String savedVersion = OTAService::readVersion();
        // DEV_PRINT("NVS: savedVersion leida en networkLoop = ");
        // DEV_PRINTLN(savedVersion);
        _ota.setVersion(savedVersion);
    }
    else
    {
        DEV_PRINTLN("\n[Core 0] WiFi connection failed. Apagando antena.");
        WiFi.disconnect();
        vTaskDelete(NULL);
    }

    while (true)
    {
        update();

        if (_isConnected && !_otaChecked)
        {
            DEV_PRINTLN("[Core 0] ¡Wi-Fi Listo! Buscando actualizaciones de fondo...");
            _otaChecked = true;
            _ota.performUpdate();
            DEV_PRINTLN("[Core 0] Consola al día. Volviendo a tareas de red cotidianas.");
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
    DEV_PRINTLN("Nuevas credenciales guardadas con éxito en la Flash.");
    WiFi.disconnect();
    WiFi.begin(_ssid, _password);
}