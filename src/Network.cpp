#include "Network.hpp"
void wifi_task(void *pvParameter)
{
    Serial.println(F("WiFi - Start Task"));
    for (;;)
    {
        if (((WiFiMulti *)pvParameter)->run() == WL_CONNECTED)
        {
            if (!MDNS.begin(ASSID))
            {
                Serial.println(F("Error starting mDNS"));
            }
        }
        yield();
        delay(5000);
    }

    Serial.println(F("WiFi - End Task"));
    vTaskDelete(NULL);
}

Network::Network(const char *config_wifi_filename) : config_wifis_json(CONFIG_WIFI_SIZE)
{
    //WiFi.persistent(true);

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ASSID, NULL, 1, 0, 1);

    File config_wifis_file = SPIFFS.open(config_wifi_filename, FILE_READ);
    DeserializationError err_config_wifis_json = deserializeJson(config_wifis_json, config_wifis_file);
    config_wifis_file.close();
    if (err_config_wifis_json || !config_wifis_json.containsKey("list_saved_wifis") || !config_wifis_json.containsKey("enable_wifi_task"))
    {
        config_wifis_json.clear();
        config_wifis_json.createNestedArray("list_saved_wifis");
        config_wifis_json["enable_wifi_task"] = false;
        save();
    }

    JsonArray list_saved_wifis = config_wifis_json["list_saved_wifis"];
    for (JsonObject s_wifis : list_saved_wifis)
    {
        wifiMulti.addAP(s_wifis["SSID"].as<const char *>(), s_wifis["PWD"].as<const char *>());
    }

    if (config_wifis_json["enable_wifi_task"].as<bool>())
        start_wifi_task();
}

void Network::auto_connect()
{
    if (wifiMulti.run() == WL_CONNECTED)
    {
        if (!MDNS.begin(ASSID))
        {
            Serial.println(F("Error starting mDNS"));
        }
    }
}

void Network::start_wifi_task()
{
    if (wifi_task_t == NULL)
    {
        xTaskCreatePinnedToCore(
            wifi_task,    // Task function.
            "wifi_task",  // name of task.
            2048,         // Stack size of task
            &wifiMulti,   // parameter of the task
            1,            // priority of the task
            &wifi_task_t, // Task handle to keep track of created task
            0);           // pin task to core <>
    }

    config_wifis_json["enable_wifi_task"] = true;
    save();
}

void Network::stop_wifi_task()
{
    if (wifi_task_t != NULL)
    {
        vTaskDelete(wifi_task_t);
        wifi_task_t = NULL;
    }
    config_wifis_json["enable_wifi_task"] = false;
    save();
}

void Network::connect(const char *ssid, const char *password)
{
    WiFi.begin(ssid, password);

    int cnt = 0;
    while ((WiFi.status() != WL_CONNECTED) && cnt < 10)
    {
        delay(100);
        Serial.print(F("."));
        cnt++;
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        delay(100);
    }
}

void Network::add(const String &ssid, const String &pwd)
{
    wifiMulti.addAP(ssid.c_str(), pwd.c_str());
    if (config_wifis_json.containsKey("list_saved_wifis"))
    {
        Serial.println(F("Adding new Network"));
        JsonObject n_wifi = config_wifis_json["list_saved_wifis"].createNestedObject();
        n_wifi["SSID"] = ssid;
        n_wifi["PWD"] = pwd;
        save();
    }
}

void Network::remove(size_t pos)
{
    if (config_wifis_json.containsKey("list_saved_wifis"))
    {
        JsonArray ssids = config_wifis_json["list_saved_wifis"];
        ssids.remove(pos);
        save();
    }
}

void Network::listActiveWifis(JsonVariant &root)
{
    Serial.println(F("Listing active wifis..."));
    int numSsid = WiFi.scanNetworks();
    if (numSsid == -1)
    {
        Serial.println(F("Couldn't get a wifi connection"));
        while (true)
            ;
    }
    auto active_wifis = root.createNestedArray("list_active_wifis");
    for (int thisNet = 0; thisNet < numSsid; thisNet++)
    {
        JsonObject wifi = active_wifis.createNestedObject();
        wifi["IDX"] = thisNet;
        wifi["SSID"] = WiFi.SSID(thisNet);
        wifi["RSSI"] = WiFi.RSSI(thisNet);
        wifi["ENC"] = WiFi.encryptionType(thisNet);
    }
}

void Network::listSavedWifis(JsonVariant &root)
{
    Serial.println(F("Listing saved wifis..."));
    //serializeJsonPretty(config_wifis_json, Serial);
    root["list_saved_wifis"].set(config_wifis_json["list_saved_wifis"]);
}

void Network::save()
{
    //config_wifis_json.garbageCollect();
    Serial.println("Saving...");
    //serializeJsonPretty(config_wifis_json, Serial);

    File config_wifis_file = SPIFFS.open(CONFIG_WIFI_FILENAME, FILE_WRITE);
    if (serializeJson(config_wifis_json, config_wifis_file) == 0)
    {
        Serial.println(F("Failed to write config_wifis_file to fs"));
    }
    else
    {
        Serial.println(F("File saved/created"));
    }
    config_wifis_file.close();
}