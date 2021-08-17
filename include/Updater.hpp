#ifndef UPDATE_HPP
#define UPDATE_HPP

#define DEFINE_WIRBELWIND_VERSION 0

#include <Arduino.h>
#include <ArduinoJson.h>
//#include <SD.h>
#include <SD_MMC.h>

#include <HTTPClient.h>

#include <WiFiClientSecure.h>
#include <HTTPUpdate.h>

enum EnumUpdater { website, firmware, list_of_files };

class Updater
{
private:

public:
    uint8_t _current_firmware_version = DEFINE_WIRBELWIND_VERSION;
    uint8_t _current_website_version = 0;
    DynamicJsonDocument jsonDoc;

    Updater();
    void refresh(JsonVariant *root);

    void startTaskUpdateFirmware();
    void startTaskUpdateWebsite();

    void updateFirmware();
    void updateWebsite();

    void updateListOfFiles(const JsonArray &doc_update_files);
    void updateListOfFolder(const JsonArray &doc_update_files);

    bool downloadHTTP(const String &uri_local_file, const char* uri_remote_file, bool overwrite = false);
};
#endif