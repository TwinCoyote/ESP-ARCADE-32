#ifndef OTA_SERVICE_H
#define OTA_SERVICE_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>

class OTAService
{
private:
    String _version;
    String _user;
    String _repo;

    void configureSecureClient(WiFiClientSecure &client);

public:
    OTAService(String currentVersion, String user, String repo);
    String checkLatestVersion();
    bool performUpdate();
};

#endif