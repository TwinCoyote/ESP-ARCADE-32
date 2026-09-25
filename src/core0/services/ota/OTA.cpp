#include "OTA.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include "../../../config/debug_log.h"

OTAService::OTAService(String currentVersion, String user, String repo)
{
    _version = currentVersion;
    _user = user;
    _repo = repo;
};

String OTAService::checkLatestVersion()
{
    String url = "https://api.github.com/repos/" + _user + "/" + _repo + "/releases/latest";
    WiFiClientSecure client;
    HTTPClient http;
    client.setInsecure();
    http.begin(client, url);
    http.addHeader("User-Agent", "ESP-ARCADE-OTA");
    http.addHeader("Accept", "application/vnd.github.v3+json");
    int httpCode = http.GET();

    if (httpCode == 200)
    {
        String payload = http.getString();
        DynamicJsonDocument doc(16384);

        DeserializationError error = deserializeJson(doc, payload);
        if (error)
        {
            DEV_PRINTF("OTA JSON error: %s\n", error.c_str());
            http.end();
            return String();
        }

        if (doc.containsKey("tag_name") && !doc["tag_name"].isNull())
        {
            String latestTag = doc["tag_name"];
            http.end();
            return latestTag;
        }

        DEV_PRINTLN("OTA: tag_name no encontrado en GitHub API");
    }
    else
    {
        DEV_PRINTF("OTA HTTP error: %d\n", httpCode);
    }

    http.end();
    return String();
};

bool OTAService::performUpdate()
{
    String latestVersion = checkLatestVersion();
    // DEV_PRINT("OTA: version actual antes de comparar = ");
    // DEV_PRINTLN(_version);
    // DEV_PRINT("OTA: latestVersion desde GitHub = ");
    // DEV_PRINTLN(latestVersion);

    if (latestVersion == _version || latestVersion == "")
    {
        DEV_PRINT("La consola tiene la version mas nueva: ");
        DEV_PRINTLN(_version);
        return false;
    }
    else
    {
        String downloadUrl = "https://github.com/" + _user + "/" + _repo + "/releases/download/" + latestVersion + "/firmware.bin";
        DEV_PRINT("Version actual: ");
        DEV_PRINTLN(_version);
        DEV_PRINTLN(" ");
        DEV_PRINT("Version mas nueva: ");
        DEV_PRINTLN(latestVersion);
        DEV_PRINTLN(" ");
        DEV_PRINTLN(downloadUrl);

        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient http;

        DEV_PRINTLN("¡Descargando nueva versión... No apagues la consola!");
        http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
        http.begin(client, downloadUrl);
        http.addHeader("User-Agent", "ESP-ARCADE-OTA");

        httpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
        httpUpdate.rebootOnUpdate(false);

        t_httpUpdate_return ret = httpUpdate.update(http, _version);
        http.end();
        switch (ret)
        {
        case HTTP_UPDATE_FAILED:
            DEV_PRINTF("Error de OTA (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            break;
        case HTTP_UPDATE_NO_UPDATES:
            DEV_PRINTLN("El servidor dijo que no hay actualizaciones pendientes.");
            break;
        case HTTP_UPDATE_OK:
            DEV_PRINTLN("¡Actualización exitosa!");
            _version = latestVersion;
            saveVersion();
            DEV_PRINTLN("NVS: Guardada la versión. Reiniciando sistema...\n");
            ESP.restart();
            break;
        }
    }

    return false;
};

void OTAService::saveVersion()
{

    Preferences storage;
    if (storage.begin("system", false))
    {
        // DEV_PRINTLN("NVS: storage abierto correctamente para escritura");
        storage.putString("fw_version", _version);
        storage.putString("fw_version_backup", _version);
        // DEV_PRINT("NVS: fw_version guardada como: ");
        // DEV_PRINTLN(_version);
        storage.end();
    }
    else
    {
        DEV_PRINTLN("NVS Error: No se pudo abrir el almacenamiento para guardar la versión.");
    }
}

String OTAService::readVersion()
{
    String ActualVersion = "";
    Preferences storage;
    // DEV_PRINTLN("NVS: Intentando leer fw_version desde storage");
    if (storage.begin("system", true))
    {
        // DEV_PRINTLN("NVS: storage abierto correctamente para lectura");
        if (storage.isKey("fw_version"))
        {
            ActualVersion = storage.getString("fw_version", "");
            DEV_PRINT("NVS: fw_version encontrada: ");
            DEV_PRINTLN(ActualVersion);
        }
        else if (storage.isKey("fw_version_backup"))
        {
            ActualVersion = storage.getString("fw_version_backup", "");
            DEV_PRINT("NVS: fw_version_backup encontrada: ");
            DEV_PRINTLN(ActualVersion);
        }
        else
        {
            DEV_PRINTLN("NVS: fw_version no existe en storage");
        }
        storage.end();
    }
    else
    {
        DEV_PRINTLN("NVS Error: No se pudo abrir el almacenamiento para leer la versión.");
    }
    DEV_PRINT("NVS: Valor devuelto de readVersion(): ");
    DEV_PRINTLN(ActualVersion);
    return ActualVersion;
}

void OTAService::setVersion(String version)
{
    _version = version;
}

String OTAService::extract_version()
{
    return _version;
}