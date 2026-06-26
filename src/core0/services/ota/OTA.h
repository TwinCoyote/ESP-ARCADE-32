#ifndef OTA_SERVICE_H
#define OTA_SERVICE_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <Preferences.h>

class OTAService
{
private:
    String _version;
    String _user;
    String _repo;
    // String newVersion;

    void configureSecureClient(WiFiClientSecure &client);

public:
    /**
     * @brief Funcion para configurar el OTA
     * @param currentVersion La version Actual que tiene la consola
     * @param user nombre de usuario en github
     * @param repo repositorio del proyecto en github
     */
    OTAService(String currentVersion, String user, String repo);
    String checkLatestVersion();
    bool performUpdate();
    void saveVersion();
    static String readVersion();
    void setVersion(String version);
    Preferences prefs;
};

#endif