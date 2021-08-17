#ifndef PLAYLIST_HPP
#define PLAYLIST_HPP

#include <Arduino.h>

#include <vector>

//#include <SD.h>
#include <SD_MMC.h>
#include <SPIFFS.h>

#include <ArduinoJson.h>

class Playlist
{
private:
    String pathToPlaylist = "";
    String pathToTmpPlaylist = "";

    String uuid;

public:
    Playlist();
    Playlist(const String &uuid);
    ~Playlist()
    {
        /*if(!tmpPlaylistFilename.isEmpty())
            saveTmpPlaylist(); 
        if(!playlistFilename.isEmpty())
            savePlaylist(); */
        Serial.println("Destructor playlist: " + docPlaylist["name"].as<String>());
    }; //
    String getName() const { return docPlaylist["name"].as<String>(); }
    String getUUID() const { return uuid; };
    void updateName(const String &name)
    {
        docPlaylist["name"] = name;
        savePlaylist();
    }

    void parsePlaylistFromSD(const String &pathToFile);
    void parseTmpPlaylist(const String &pathToFile);

    void shrinkToFit();

    bool destroyPlaylist();

    void addTracks(const JsonArray &addTracks);
    void deleteTracks(const JsonArray &deleteTracks);

    void reset();
    void resetTrack();

    bool saveTmpPlaylist();
    bool savePlaylist();

    bool updateVolume(const uint8_t &volume);
    uint8_t getVolume() const { return docTmpPlaylist["volume"].as<uint8_t>(); };

    void nextTrack();
    bool updateCurrTrack(const size_t &track);
    size_t getCurrTrack() const { return docTmpPlaylist["curr_track"].as<size_t>(); };

    bool updateCurrTimestamp(const uint32_t &timestamp);
    uint32_t getCurrTimestamp() const { return docTmpPlaylist["curr_timestamp"].as<uint32_t>(); };

    bool updateCurrMaxTimestamp(const uint32_t &max_timestamp);

    DynamicJsonDocument docPlaylist;
    //DynamicJsonDocument docTmpPlaylist;
    StaticJsonDocument<200> docTmpPlaylist;
};

#endif