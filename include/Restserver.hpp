#ifndef RESTSERVER_HPP
#define RESTSERVER_HPP

#define CONFIG_ASYNC_TCP_RUNNING_CORE 0
//#define CONFIG_ASYNC_TCP_USE_WDT 0
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>

#include "Board.hpp"
#include "EventQueue.hpp"
#include "Network.hpp"
#include "Playlist.hpp"
#include "SD_Player.hpp"
#include "Updater.hpp"

class Restserver : public AsyncWebServer
{
private:
    Board *_board;
    Network *_network;
    EventQueue *_eventQueue;
    SD_Player *_sd_player;

public:
    AsyncEventSource *events;
    Updater updater;
    bool isHeavyWorking = false;
    bool updateWebsite = false;
    bool updateFirmware = false;

    Restserver(Board *board, Network *network, EventQueue *eventQueue, SD_Player *sd_player);
    void restServerRouting();

    void handleNotFound(AsyncWebServerRequest *request);

    void manageUpdater(AsyncWebServerRequest *request);

    void manageNetworks(AsyncWebServerRequest *request);
    void addNetwork(AsyncWebServerRequest *request, DynamicJsonDocument &doc);
    void deleteNetwork(AsyncWebServerRequest *request, DynamicJsonDocument &doc);

    void listPlaylists(AsyncWebServerRequest *request);
    void managePlaylist(AsyncWebServerRequest *request, DynamicJsonDocument &doc);
    void deletePlaylist(AsyncWebServerRequest *request, DynamicJsonDocument &doc);

    void handleUploadFiles(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    void listFiles(AsyncWebServerRequest *request);
    void deleteFS(AsyncWebServerRequest *request, DynamicJsonDocument &doc);
    void manageFS(AsyncWebServerRequest *request, DynamicJsonDocument &doc);
};

#endif
