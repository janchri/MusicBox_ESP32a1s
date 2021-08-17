#ifndef SD_PLAYER_HPP
#define SD_PLAYER_HPP

#include <Arduino.h>
#include <SD_MMC.h>
#include <HTTPClient.h>

#include <AC101.h>
#include <AudioFileSourceFS.h>
#include <AudioFileSourceBuffer.h>
#include <AudioFileSourceSPIRAMBuffer.h>
#include <AudioGeneratorMP3.h>
#include <AudioGeneratorMP3a.h>
#include <AudioGeneratorFLAC.h>
#include <AudioOutputI2S.h>

#include "Playlist.hpp"


// I2S GPIOs, the names refer on AC101
#define I2S_DSIN      25
#define I2S_BCLK      27
#define I2S_LRC       26
#define I2S_MCLK       0
#define I2S_DOUT      35

// I2C GPIOs
#define IIC_CLK       32
#define IIC_DATA      33
#define GPIO_PA_EN 21

class SD_Player
{
private:
    AC101 ac;

    AudioGenerator *decoder;
    AudioGeneratorMP3a *decoder_mp3a;
    AudioGeneratorFLAC *decoder_flac;

    AudioFileSourceFS *file;
    AudioOutputI2S *out;
    AudioFileSourceBuffer* buff;

    byte* psdRamBuffer = (byte*)ps_malloc(1024);

    bool toggle = false;
    bool isPausing = false;

    Playlist *currPlaylist;

    TaskHandle_t play_handle_t = NULL;
public:
    SD_Player();
    ~SD_Player();

    void setPlaylist(const char* uuid);
    Playlist* getPlaylist() const {return currPlaylist;};

    void setTitle(const char* path);

    void play();
    void pause();
    void resume();
    void stop();
};

#endif