#include "Restserver.hpp"

Restserver::Restserver(Board *board, Network *network, EventQueue *eventQueue) : AsyncWebServer(80), _board(board), _network(network),_eventQueue(eventQueue)
{
    // Set server routing
    restServerRouting();

    // Set not found response
    onNotFound([this](AsyncWebServerRequest *request)
               { handleNotFound(request); });

    // Handle CORS
    DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), F("*"));
    DefaultHeaders::Instance().addHeader(F("Access-Control-Max-Age"), F("600"));
    DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,PATCH,GET,OPTIONS"));
    DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), F("*"));
    // Start server
    begin();
    Serial.println(F("HTTP server started"));
}

// Define routing
void Restserver::restServerRouting()
{
    /*events = new AsyncEventSource("/events");
    events->onConnect([](AsyncEventSourceClient *client)
                      {
                          if (client->lastId())
                          {
                              Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
                          }
                          //send event with message "hello!", id current millis
                          // and set reconnect delay to 1 second
                          client->send("hello!", NULL, millis(), 1000);
                      });
    addHandler(events);*/

    onFileUpload([this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
                 { handleUploadFiles(request, filename, index, data, len, final); });

    on("/networks", HTTP_GET, [this](AsyncWebServerRequest *request)
       { manageNetworks(request); });
    on("/files", HTTP_GET, [this](AsyncWebServerRequest *request)
       { listFiles(request); });
    on("/playlist", HTTP_GET, [this](AsyncWebServerRequest *request)
       { listPlaylists(request); });
    on("/update", HTTP_GET, [this](AsyncWebServerRequest *request)
       { manageUpdater(request); });

    /*onRequestBody([this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
                  {
                      DynamicJsonDocument jsonDoc(ESP.getMaxAllocHeap());
                      if (DeserializationError::Ok == deserializeJson(jsonDoc, (const char *)data))
                      {
                          jsonDoc.shrinkToFit();
                          serializeJsonPretty(jsonDoc, Serial);
                          if (request->url() == "/networks")
                          {
                              if (request->method() == HTTP_POST)
                                  addNetwork(request, jsonDoc);
                              if (request->method() == HTTP_PATCH)
                                  deleteNetwork(request, jsonDoc);
                          }
                          if (request->url() == "/files")
                          {
                              if (request->method() == HTTP_PATCH)
                                  deleteFS(request, jsonDoc);
                              if (request->method() == HTTP_POST)
                                  manageFS(request, jsonDoc);
                          }
                          if (request->url() == "/playlist")
                          {
                              if (request->method() == HTTP_POST)
                                  managePlaylist(request, jsonDoc);
                              if (request->method() == HTTP_PATCH)
                                  deletePlaylist(request, jsonDoc);
                          }
                      }
                      else
                      {
                          request->send(400, "application/json", "error");
                      }
                  });

    //esp_wifi_set_ps(WIFI_PS_NONE);
    serveStatic("/", SD_MMC, "/www/pub/").setCacheControl("max-age=604800,public"); // 7 days store
    */
}

void Restserver::manageUpdater(AsyncWebServerRequest *request)
{
    AsyncJsonResponse *response = new AsyncJsonResponse(false, 128);
    JsonVariant root = response->getRoot();
    response->addHeader("Server", "WirbelWind.Box");
    if (request->hasArg(F("website")))
    {
        updateWebsite = true;
        //updater.startTaskUpdateWebsite();
        //updater.updateWebsite();
    }
    if (request->hasArg(F("firmware")))
    {
        updateFirmware = true;
        //updater.startTaskUpdateFirmware();
        //updater.updateFirmware();
    }
    if (request->hasArg(F("refresh")))
    {
        //updater.refresh(&root);
        updater.refresh(&root);
    }
    root["current_firmware_version"] = updater._current_firmware_version;
    root["current_website_version"] = updater._current_website_version;
    response->setLength();
    request->send(response);
}

void Restserver::manageNetworks(AsyncWebServerRequest *request)
{
    AsyncJsonResponse *response = new AsyncJsonResponse(false, 1024);
    response->addHeader("Server", "WirbelWind.Box");
    if (request->arg("list") == F("active"))
    {
        _network->listActiveWifis(response->getRoot());
    }
    else if (request->arg("list") == F("saved"))
    {
        _network->listSavedWifis(response->getRoot());
    }
    else if (request->hasArg("enable_wifi_task"))
    {
        if (request->arg("enable_wifi_task")==F("true")){
            _network->start_wifi_task();
        }
        if (request->arg("enable_wifi_task")==F("false")){
            _network->stop_wifi_task();
        }
        JsonVariant root = response->getRoot();
        root["enable_wifi_task"] =_network->config_wifis_json["enable_wifi_task"];
    }
    else 
    {
        request->send(400, F("application/json"), F("Missing list=[active|saved]"));
        return;
    }
    response->setLength();
    request->send(response);
}

void Restserver::addNetwork(AsyncWebServerRequest *request, DynamicJsonDocument &doc)
{
    if (doc.containsKey("SSID") && doc.containsKey("PWD"))
    {
        AsyncJsonResponse *response = new AsyncJsonResponse(false, 1024);
        response->addHeader("Server", "WirbelWind.Box");
        _network->add(doc["SSID"].as<String>(), doc["PWD"].as<String>());
        _network->listSavedWifis(response->getRoot());
        response->setLength();
        request->send(response);
    }
    else
    {
        request->send(400, F("application/json"), F("No ssid/pwd given"));
    }
}

void Restserver::deleteNetwork(AsyncWebServerRequest *request, DynamicJsonDocument &doc)
{
    if (doc.containsKey("idx"))
    {
        int idx = doc["idx"].as<int>();
        AsyncJsonResponse *response = new AsyncJsonResponse(false, 1024);
        response->addHeader("Server", "WirbelWind.Box");
        _network->remove(idx);
        _network->listSavedWifis(response->getRoot());
        response->setLength();
        request->send(response);
    }
    else
    {
        request->send(400, F("application/json"), F("No idx given"));
    }
}

// Files
void Restserver::handleUploadFiles(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    String logmessage;
    String path = request->url();

    if (!index)
    {
        logmessage = "Upload Start: " + String(filename);
        isHeavyWorking = true;
        // open the file on first call and store the file handle in the request object
        request->_tempFile = SD_MMC.open(path + "/" + filename, "w");
        Serial.println(logmessage);
    }

    if (len)
    {
        // stream the incoming chunk to the opened file
        request->_tempFile.write(data, len);
        logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
        Serial.println(logmessage);
    }

    if (final)
    {
        logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
        // close the file handle as the upload is now done
        request->_tempFile.close();
        Serial.println(logmessage);
        isHeavyWorking = false;
        request->send(200, F("application/json"), F("Sucess"));
    }
}

void Restserver::listFiles(AsyncWebServerRequest *request)
{
    if (request->hasArg("path"))
    {
        File dir = SD_MMC.open(request->arg("path"));
        File entry = dir.openNextFile();
        int limit = 100;
        if (!request->arg("limit").isEmpty())
        {
            limit = request->arg("limit").toInt();
        }
        int offset = 0;
        if (!request->arg("offset").isEmpty())
        {
            offset = request->arg("offset").toInt();
        }

        //const size_t capacity = limit * 200;
        AsyncJsonResponse *response = new AsyncJsonResponse(true, ESP.getMaxAllocHeap());
        response->addHeader("Server", "WirbelWind.Box");
        JsonArray files = response->getRoot();

        while (entry && limit > 0)
        {
            if (offset > 0)
            {
                offset--;
            }
            else
            {
                String path = entry.name();
                JsonObject file = files.createNestedObject();
                if (entry.isDirectory())
                {
                    file["type"] = "d";
                }
                else
                {
                    file["type"] = "f";
                    file["size"] = entry.size();
                }
                file["path"] = path;
                limit--;
            }
            entry = dir.openNextFile();
        }
        if (limit <= 0 && entry)
        {
            //files.add(F("...")); // FIXME
        }
        response->setLength();
        request->send(response);

        entry.close();
        dir.close();
    }
    else
    {
        request->send(400, F("application/json"), F("No path specified"));
    }
}

void Restserver::manageFS(AsyncWebServerRequest *request, DynamicJsonDocument &doc)
{
    if (doc.containsKey("create_folder"))
    {
        if (SD_MMC.mkdir(doc["create_folder"].as<const char *>()))
        {
            listFiles(request);
        }
        else
        {
            request->send(400, F("application/json"), F("No folder created"));
        }
    }
    if (doc.containsKey("rename"))
    {
        request->send(400, F("application/json"), F("no support so far"));
    }
}

void Restserver::deleteFS(AsyncWebServerRequest *request, DynamicJsonDocument &doc)
{
    if (doc.containsKey("delete_file"))
    {
        if (SD_MMC.remove(doc["delete_file"].as<const char *>()))
        {
            listFiles(request);
        }
        else
        {
            request->send(400, F("application/json"), F("No files removed"));
        }
    }
    if (doc.containsKey("delete_folder"))
    {
        if (SD_MMC.rmdir(doc["delete_folder"].as<const char *>()))
        {
            listFiles(request);
        }
        else
        {
            request->send(400, F("application/json"), F("No folder removed"));
        }
    }
}
// Playlists
void Restserver::listPlaylists(AsyncWebServerRequest *request)
{
    File dir = SD_MMC.open("/Music/.config");
    File entry = dir.openNextFile();

    int limit = 100;
    if (!request->arg("limit").isEmpty())
    {
        limit = request->arg("limit").toInt();
    }
    int offset = 0;
    if (!request->arg("offset").isEmpty())
    {
        limit = request->arg("offset").toInt();
    }
    String filter;
    if (!request->arg("filter").isEmpty())
    {
        filter = request->arg("filter");
    }

    //const size_t capacity = limit * 200;
    AsyncJsonResponse *response = new AsyncJsonResponse(true, ESP.getMaxAllocHeap());
    response->addHeader("Server", "WirbelWind.Box");
    JsonArray playlists = response->getRoot();
    while (entry && limit > 0)
    {
        if (offset > 0)
        {
            offset--;
        }
        else
        {
            String path = entry.name();
            if (path.indexOf("/playlist") > 0)
            {
                Playlist playlist;
                playlist.parsePlaylistFromSD(path);
                Serial.println(playlist.getName());
                if (filter.isEmpty())
                {
                    //playlists.add(playlist.docPlaylist);
                    auto entry = playlists.createNestedObject();
                    entry["uuid"] = playlist.docPlaylist["uuid"];
                    entry["name"] = playlist.docPlaylist["name"];
                    limit--;
                }
                else if (playlist.getName().indexOf(filter) >= 0)
                {
                    //playlists.add(playlist.docPlaylist);
                    auto entry = playlists.createNestedObject();
                    entry["uuid"] = playlist.docPlaylist["uuid"];
                    entry["name"] = playlist.docPlaylist["name"];
                    limit--;
                }
            }
        }
        entry = dir.openNextFile();
    }
    if (limit <= 0 && entry)
    {
        //files.add(F("...")); // FIXME
    }
    response->setLength();
    request->send(response);
    entry.close();
    dir.close();
}

void Restserver::managePlaylist(AsyncWebServerRequest *request, DynamicJsonDocument &doc)
{
    if (doc.containsKey("uuid"))
    {
        String uuid = doc["uuid"].as<String>();
        Playlist playlist(uuid);

        if (doc.containsKey("add"))
        {
            playlist.addTracks(doc["add"].as<JsonArray>());
        }

        if (doc.containsKey("name"))
        {
            playlist.updateName(doc["name"].as<String>());
        }
        if (doc.containsKey("curr_timestamp"))
        {
            uint32_t curr_timestamp = doc["curr_timestamp"].as<uint32_t>();
            xQueueSend(_eventQueue->currTimestampQueue, &curr_timestamp, 0);
            playlist.updateCurrTimestamp(curr_timestamp);
        }
        if (doc.containsKey("curr_track"))
        {
            size_t curr_track = doc["curr_track"].as<size_t>();
            xQueueSend(_eventQueue->currTrackQueue, &curr_track, 0);
            playlist.updateCurrTrack(curr_track);
        }
        if (doc.containsKey("volume"))
        {
            uint8_t volume = doc["volume"].as<uint8_t>();
            xQueueSend(_eventQueue->currVolumeQueue, &volume, 0);
            playlist.updateVolume(volume % 21);
        }
        if (doc.containsKey("delete"))
        {
            playlist.deleteTracks(doc["delete"].as<JsonArray>());
        }

        if (doc.containsKey("reset"))
        {
            playlist.reset();
        }

        AsyncJsonResponse *response = new AsyncJsonResponse(true, ESP.getMaxAllocHeap());
        response->getRoot().add(playlist.docPlaylist);
        response->getRoot().add(playlist.docTmpPlaylist);
        response->addHeader("Server", "WirbelWind.Box");
        response->setLength();
        request->send(response);
    }
    else
    {
        request->send(400, F("application/json"), F("No UUID specified"));
    }
}

void Restserver::deletePlaylist(AsyncWebServerRequest *request, DynamicJsonDocument &doc)
{
    if (doc.containsKey("uuid"))
    {
        String uuid = doc["uuid"].as<String>();
        Playlist playlist(uuid);
        if (playlist.destroyPlaylist())
        {
            listPlaylists(request);
        }
        else
        {
            request->send(400, F("application/json"), F("Failed to destroy playlist"));
        }
    }
    else
    {
        request->send(400, F("application/json"), F("No UUID specified"));
    }
}

// Manage not found URL
void Restserver::handleNotFound(AsyncWebServerRequest *request)
{
    if (request->method() == HTTP_OPTIONS)
    {
        // Setup preflag for put request
        request->send(204);
    }
    else
    {
        request->send(404, "application/json", "Not found");
    }
}