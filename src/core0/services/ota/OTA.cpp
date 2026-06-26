#include "OTA.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>

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
            Serial.printf("OTA JSON error: %s\n", error.c_str());
            http.end();
            return String();
        }

        if (doc.containsKey("tag_name") && !doc["tag_name"].isNull())
        {
            String latestTag = doc["tag_name"];
            http.end();
            return latestTag;
        }

        Serial.println("OTA: tag_name no encontrado en GitHub API");
    }
    else
    {
        Serial.printf("OTA HTTP error: %d\n", httpCode);
    }

    http.end();
    return String();
};

bool OTAService::performUpdate()
{
    String latestVersion = checkLatestVersion();
    // Serial.print("OTA: version actual antes de comparar = ");
    // Serial.println(_version);
    // Serial.print("OTA: latestVersion desde GitHub = ");
    // Serial.println(latestVersion);

    if (latestVersion == _version || latestVersion == "")
    {
        Serial.print("La consola tiene la version mas nueva: ");
        Serial.println(_version);
        return false;
    }
    else
    {
        String downloadUrl = "https://github.com/" + _user + "/" + _repo + "/releases/download/" + latestVersion + "/firmware.bin";
        Serial.print("Version actual: ");
        Serial.println(_version);
        Serial.println(" ");
        Serial.print("Version mas nueva: ");
        Serial.println(latestVersion);
        Serial.println(" ");
        Serial.println(downloadUrl);

        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient http;

        Serial.println("¡Descargando nueva versión... No apagues la consola!");
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
            Serial.printf("Error de OTA (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("El servidor dijo que no hay actualizaciones pendientes.");
            break;
        case HTTP_UPDATE_OK:
            Serial.println("¡Actualización exitosa!");
            _version = latestVersion;
            saveVersion();
            Serial.println("NVS: Guardada la versión. Reiniciando sistema...\n");
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
        // Serial.println("NVS: storage abierto correctamente para escritura");
        storage.putString("fw_version", _version);
        storage.putString("fw_version_backup", _version);
        // Serial.print("NVS: fw_version guardada como: ");
        // Serial.println(_version);
        storage.end();
    }
    else
    {
        Serial.println("NVS Error: No se pudo abrir el almacenamiento para guardar la versión.");
    }
}

String OTAService::readVersion()
{
    String ActualVersion = "";
    Preferences storage;
    // Serial.println("NVS: Intentando leer fw_version desde storage");
    if (storage.begin("system", true))
    {
        // Serial.println("NVS: storage abierto correctamente para lectura");
        if (storage.isKey("fw_version"))
        {
            ActualVersion = storage.getString("fw_version", "");
            Serial.print("NVS: fw_version encontrada: ");
            Serial.println(ActualVersion);
        }
        else if (storage.isKey("fw_version_backup"))
        {
            ActualVersion = storage.getString("fw_version_backup", "");
            Serial.print("NVS: fw_version_backup encontrada: ");
            Serial.println(ActualVersion);
        }
        else
        {
            Serial.println("NVS: fw_version no existe en storage");
        }
        storage.end();
    }
    else
    {
        Serial.println("NVS Error: No se pudo abrir el almacenamiento para leer la versión.");
    }
    Serial.print("NVS: Valor devuelto de readVersion(): ");
    Serial.println(ActualVersion);
    return ActualVersion;
}

void OTAService::setVersion(String version)
{
    _version = version;
}