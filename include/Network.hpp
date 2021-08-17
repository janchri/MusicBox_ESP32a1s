#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <SPIFFS.h>
#include <SD_MMC.h>

#include <WiFiMulti.h>

#include <WiFi.h>
#include <WiFiAP.h>
#include <ESPmDNS.h>

#include <ArduinoJson.h>

#define ASSID "music.Box"
#define CONFIG_WIFI_FILENAME "/config_wifis.json"
#define CONFIG_WIFI_SIZE 1024

class Network
{
private:
    WiFiMulti wifiMulti;
    TaskHandle_t wifi_task_t = NULL;
public:
    DynamicJsonDocument config_wifis_json;
    
    Network() : Network(CONFIG_WIFI_FILENAME) {};
    Network(const char* config_wifi_filename);
    void connect(const char *ssid, const char *password);
    void auto_connect();
    void start_wifi_task();
    void stop_wifi_task();
    void stop();

    void add(const String &ssid, const String &pwd);
    void remove(size_t pos);
    void listActiveWifis(JsonVariant &root);
    void listSavedWifis(JsonVariant &root);
    void save();
};

#endif