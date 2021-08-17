#include "Updater.hpp"

#define DynJSONCapacity 2048

void update_firmware(void *pvParameter)
{
    Serial.println(F("[Updater-Task] - Updating Firmware"));
    for (;;)
    {
        ((Updater *)pvParameter)->updateFirmware();
        break;
    }
    vTaskDelete(NULL);
}
void update_website(void *pvParameter)
{
    Serial.println(F("[Updater-Task] - Updating Website"));
    for (;;)
    {
        ((Updater *)pvParameter)->updateWebsite();
        break;
    }
    vTaskDelete(NULL);
}

Updater::Updater() : jsonDoc(DynJSONCapacity)
{
    File version_json = SD_MMC.open("/www/pub/version.json", FILE_READ);
    DeserializationError err = deserializeJson(jsonDoc, version_json);
    version_json.close();
    if (DeserializationError::Ok == err)
    {
        _current_website_version = jsonDoc["current_website_version"].as<uint8_t>();

        Serial.println("Version Firmware: " + String(_current_firmware_version));
        Serial.println("Version Website: " + String(_current_website_version));
    }
    else
    {
        Serial.println(F("[Updater] Error reading version_json"));
    }

    File update_json = SD_MMC.open("/config/update.json", FILE_READ);
    err = deserializeJson(jsonDoc, update_json);
    if (DeserializationError::Ok == err)
    {
    }
    else
    {
        Serial.println(F("[Updater] Error reading update_json"));
    }
    update_json.close();
    jsonDoc.shrinkToFit();
}

void Updater::refresh(JsonVariant *root)
{
    downloadHTTP("/config/update.json", "https://raw.githubusercontent.com/janchri/MusicBox_ESP32a1s/main/www/update.json", true); //@FIXME user your own link here?

    jsonDoc = DynamicJsonDocument(DynJSONCapacity);
    File update_json = SD_MMC.open("/config/update.json", FILE_READ);
    DeserializationError err = deserializeJson(jsonDoc, update_json);
    jsonDoc.shrinkToFit();
    update_json.close();
    if (DeserializationError::Ok == err)
    {
        (*root)["new_firmware_version"] = jsonDoc["firmware"][0]["version"].as<uint8_t>();
        (*root)["new_website_version"] = jsonDoc["website"][0]["version"].as<uint8_t>();
    }
    else
    {
        Serial.println(F("[Updater] Error reading update_json"));
        Serial.println(err.c_str());
    }
}

void Updater::startTaskUpdateFirmware()
{
    xTaskCreatePinnedToCore(
        update_firmware,   // Task function.
        "update_firmware", // name of task.
        4096,              // Stack size of task
        this,              // parameter of the task
        1,                 // priority of the task
        NULL,              // Task handle to keep track of created task
        1);                // pin task to core <>
}
void Updater::startTaskUpdateWebsite()
{
    xTaskCreatePinnedToCore(
        update_website,   // Task function.
        "update_website", // name of task.
        4096,             // Stack size of task
        this,             // parameter of the task
        1,                // priority of the task
        NULL,             // Task handle to keep track of created task
        1);               // pin task to core <>
}

void Updater::updateFirmware()
{
    Serial.println(F("[Updater] Firmware Update..."));
    const char *uri_firmware_remote = jsonDoc["firmware"][0]["uri_remote"];
    Serial.println("Firmware uri_remote: " + String(uri_firmware_remote));

    WiFiClientSecure secureclient;
    //client.setCACert(rootCACertificate);
    secureclient.setTimeout(12000 / 1000); // timeout argument is defined in seconds for setTimeout

    t_httpUpdate_return ret = httpUpdate.update(secureclient, uri_firmware_remote);
    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        break;

    case HTTP_UPDATE_NO_UPDATES:
        Serial.println(F("HTTP_UPDATE_NO_UPDATES"));
        break;

    case HTTP_UPDATE_OK:
        Serial.println(F("HTTP_UPDATE_OK"));
        break;
    }
}

void Updater::updateListOfFolder(const JsonArray &list_update_folders)
{
    Serial.println(F("[Updater] update list of folders..."));
    for (JsonObject u_folder : list_update_folders)
    {
        Serial.println("[Updater] make folder: " + u_folder["sd_path"].as<String>());
        SD_MMC.mkdir(u_folder["sd_path"].as<String>());
    }
}

void Updater::updateListOfFiles(const JsonArray &list_update_files)
{
    Serial.println(F("[Updater] update list of files..."));
    for (JsonObject u_file : list_update_files)
    {
        Serial.println("[Updater] " + u_file["uri_local"].as<String>() + ", " + u_file["uri_remote"].as<String>());
        downloadHTTP(u_file["uri_local"].as<const char *>(), u_file["uri_remote"].as<const char *>(), true);
    }
}

void Updater::updateWebsite()
{
    Serial.println(F("[Updater] Website Update..."));
    updateListOfFolder(jsonDoc["list_update_folders"].as<JsonArray>());
    updateListOfFiles(jsonDoc["list_update_files"].as<JsonArray>());
}

bool Updater::downloadHTTP(const String &uri_local_file, const char *uri_remote_file, bool overwrite)
{
    Serial.println(F("[Updater::downloadHTTP]..."));
    bool _retVal = false;
    HTTPClient http;
    File local_file;
    http.begin(uri_remote_file);

    int httpCode = http.GET();
    if (httpCode > 0)
    {
        Serial.println("[HTTP] GET... code: " + httpCode);

        if (httpCode == HTTP_CODE_OK)
        {
            int len = http.getSize();
            local_file = SD_MMC.open(uri_local_file, FILE_READ);

            uint8_t buff[1024] = {0};

            WiFiClient *stream = http.getStreamPtr();

            if ( local_file.size() == len || overwrite  || !SD_MMC.exists(uri_local_file) )
            {
                local_file = SD_MMC.open(uri_local_file, FILE_WRITE);
                while (http.connected() && (len > 0 || len == -1))
                {
                    size_t size = stream->available();

                    if (size)
                    {
                        // read up to 1024 byte
                        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                        // write it to Serial

                        local_file.write(buff, c);
                        //Serial.write(buff, c);

                        if (len > 0)
                        {
                            len -= c;
                        }
                    }
                    delay(1);
                }
            }
            local_file.close();
        }
        Serial.println();
        Serial.println(F("[HTTP] connection closed or file end."));
        _retVal = true;
    }
    else
    {
        Serial.println("[HTTP] GET... failed, error: " + http.errorToString(httpCode));
        _retVal = false;
    }
    http.end();
    return _retVal;
}