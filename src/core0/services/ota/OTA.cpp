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
    if (latestVersion == _version || latestVersion == "")
    {
        return false;
    }
    String downloadUrl = "https://github.com/" + _user + "/" + _repo + "/releases/download/" + latestVersion + "/firmware.bin";
    Serial.println(downloadUrl);
    WiFiClientSecure client;
    client.setInsecure();
    httpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    Serial.println("¡Descargando nueva versión... No apagues la consola!");
    t_httpUpdate_return ret = httpUpdate.update(client, downloadUrl);

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
        break;
    }
    return false;
};
