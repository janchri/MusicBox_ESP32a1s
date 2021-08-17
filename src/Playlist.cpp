#include "Playlist.hpp"

#define DynJSONCapacity 6144

Playlist::Playlist() : docPlaylist(DynJSONCapacity) //, docTmpPlaylist(100){
{
}

Playlist::Playlist(const String &uuid) : uuid(uuid), docPlaylist(DynJSONCapacity) //, docTmpPlaylist(DynJSONCapacity)
{
    parsePlaylistFromSD("/Music/.config/playlist_" + uuid);
    //parseTmpPlaylist("/Music/.config/tmp_playlist_" + uuid);
    parseTmpPlaylist("/config_tmp_playlist_" + uuid);
}

void Playlist::parsePlaylistFromSD(const String &pathToFile)
{
    pathToPlaylist = pathToFile;
    // Parse existing file or create if not available!
    Serial.println("Parse file: " + pathToPlaylist);
    File playlistFile = SD_MMC.open(pathToPlaylist.c_str());
    DeserializationError errorDocPlaylist = deserializeJson(docPlaylist, playlistFile);
    playlistFile.close();

    if (errorDocPlaylist || !docPlaylist.containsKey("tracks") || !docPlaylist.containsKey("uuid") || !docPlaylist.containsKey("name"))
    {
        docPlaylist.clear();
        docPlaylist["uuid"] = uuid;
        docPlaylist["name"] = uuid;
        docPlaylist.createNestedArray("tracks");
        savePlaylist();
    }
    if (docPlaylist["uuid"] == "")
    {
        destroyPlaylist();
        Serial.println(F("Destorying empty/corrupted playlist"));
    }
}

void Playlist::parseTmpPlaylist(const String &pathToFile)
{
    pathToTmpPlaylist = pathToFile;
    // Parse existing file or create if not available!
    Serial.println("Parse tmp file: " + pathToTmpPlaylist);
    //File tmpPlaylistFile = SD_MMC.open(pathToTmpPlaylist.c_str());
    File tmpPlaylistFile = SPIFFS.open(pathToTmpPlaylist.c_str());
    DeserializationError errorDocTmpPaylistFile = deserializeJson(docTmpPlaylist, tmpPlaylistFile);
    tmpPlaylistFile.close();

    if (errorDocTmpPaylistFile || !docTmpPlaylist.containsKey("curr_track") || !docTmpPlaylist.containsKey("curr_timestamp") || !docTmpPlaylist.containsKey("volume") || !docTmpPlaylist.containsKey("curr_max_timestamp") || docPlaylist["tracks"].size() == 0)
    {
        docTmpPlaylist.clear();
        docTmpPlaylist["curr_track"] = 0;
        docTmpPlaylist["curr_timestamp"] = 0;
        docTmpPlaylist["curr_max_timestamp"] = 0;
        docTmpPlaylist["volume"] = 10;
        saveTmpPlaylist();
    }
}

void Playlist::shrinkToFit()
{
    docPlaylist.shrinkToFit();
    //docTmpPlaylist.shrinkToFit();
}

bool Playlist::destroyPlaylist()
{
    //bool res = SD_MMC.remove(pathToPlaylist.c_str()) && (SD_MMC.remove(pathToTmpPlaylist.c_str()) || !SD_MMC.exists(pathToTmpPlaylist.c_str()));
    bool res = SD_MMC.remove(pathToPlaylist.c_str()) && (SPIFFS.remove(pathToTmpPlaylist.c_str()) || !SPIFFS.exists(pathToTmpPlaylist.c_str()));
    docPlaylist.clear();
    docTmpPlaylist.clear();
    return res;
}

void Playlist::addTracks(const JsonArray &addTracks)
{
    for (auto aTrack : addTracks)
    {
        if (SD_MMC.exists(aTrack.as<String>()) && aTrack.as<String>().indexOf(".mp3") > 0)
        {
            docPlaylist["tracks"].add(aTrack);
            Serial.println(F("Track added..."));
        }
    }
    docPlaylist.garbageCollect();
    savePlaylist();
}

void Playlist::deleteTracks(const JsonArray &deleteTracks)
{
    JsonArray tracks = docPlaylist["tracks"];
    for (JsonArray::iterator it = tracks.begin(); it != tracks.end(); ++it)
    {
        for (auto dTrack : deleteTracks)
        {
            if ((*it).as<String>().equals(dTrack.as<String>()))
            {
                tracks.remove(it);
            }
        }
    }
    docPlaylist.garbageCollect();
    savePlaylist();
}

void Playlist::nextTrack()
{
    JsonArray tracks = docPlaylist["tracks"];
    if (tracks.size() > 0)
    {
        Serial.println(F("Next-Track..."));
        docTmpPlaylist["curr_track"] = (docTmpPlaylist["curr_track"].as<uint32_t>() + 1) % tracks.size();
        docTmpPlaylist["curr_timestamp"] = 0;
        docTmpPlaylist["curr_max_timestamp"] = 0;
        //saveTmpPlaylist();
    }
}

bool Playlist::updateCurrTrack(const size_t &track)
{
    JsonArray tracks = docPlaylist["tracks"];
    if (tracks.size() > track && docTmpPlaylist.containsKey("curr_track"))
    {
        Serial.println(F("Next-Track..."));
        docTmpPlaylist["curr_track"] = track;
        return saveTmpPlaylist();
    }
    else
    {
        return false;
    }
}

bool Playlist::updateCurrTimestamp(const uint32_t &timestamp)
{
    if (docTmpPlaylist.containsKey("curr_timestamp"))
    {
        docTmpPlaylist["curr_timestamp"] = timestamp;
        return saveTmpPlaylist();
    }
    else
    {
        return false;
    }
}

bool Playlist::updateCurrMaxTimestamp(const uint32_t &max_timestamp)
{
    if (docTmpPlaylist.containsKey("curr_max_timestamp"))
    {
        docTmpPlaylist["curr_max_timestamp"] = max_timestamp;
        return saveTmpPlaylist();
    }
    else
    {
        return false;
    }
}

bool Playlist::updateVolume(const uint8_t &volume)
{
    if (docTmpPlaylist.containsKey("volume"))
    {
        docTmpPlaylist["volume"] = volume;
        return saveTmpPlaylist();
    }
    else
    {
        return false;
    }
}

void Playlist::reset()
{
    SPIFFS.remove(pathToTmpPlaylist.c_str()) || !SPIFFS.exists(pathToTmpPlaylist.c_str());
    //bool ret = (SPIFFS.remove(pathToTmpPlaylist.c_str()) || !SPIFFS.exists(pathToTmpPlaylist.c_str()));
    //docTmpPlaylist["curr_track"] = 0;
    //resetTrack();
}

void Playlist::resetTrack()
{
    docTmpPlaylist["curr_timestamp"] = 0;
    saveTmpPlaylist();
}

bool Playlist::saveTmpPlaylist()
{
    Serial.println("saveTmpPlaylist() " + pathToTmpPlaylist);
    //File tmpPlaylistWrite = SD_MMC.open(pathToTmpPlaylist.c_str(), FILE_WRITE);
    File tmpPlaylistWrite = SPIFFS.open(pathToTmpPlaylist.c_str(), FILE_WRITE);
    if (serializeJson(docTmpPlaylist, tmpPlaylistWrite) == 0)
    {
        Serial.println(F("Failed to write docTmpPlaylist to fs"));
        return false;
    }
    tmpPlaylistWrite.close();
    Serial.println("TotalBytes: " + (String)SPIFFS.totalBytes());
    Serial.println("UsedBytes: " + (String)SPIFFS.usedBytes());
    return true;
}

bool Playlist::savePlaylist()
{
    Serial.println("savePlaylist() " + pathToPlaylist);
    File playlistWrite = SD_MMC.open(pathToPlaylist.c_str(), FILE_WRITE);
    if (serializeJson(docPlaylist, playlistWrite) == 0)
    {
        Serial.println(F("Failed to write docPlaylist to fs"));
        return false;
    }
    playlistWrite.close();
    return true;
}