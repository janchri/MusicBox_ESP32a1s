#include "SD_Player.hpp"
const char song[] PROGMEM = "Batman:d=8,o=5,b=180:d,d,c#,c#,c,c,c#,c#,d,d,c#,c#,c,c,c#,c#,d,d#,c,c#,c,c,c#,c#,f,p,4f";
void play_task(void *pvParameter)
{
    Serial.println(F("[SD_Player]] - Start Task"));
    for (;;)
    {
        ((SD_Player *)pvParameter)->play();
        yield();
        delay(101);
    }
    Serial.println(F("[SD_Player]] - Stop Task"));
    vTaskDelete(NULL);
}

SD_Player::SD_Player()
{
    Serial.println(F("[SD-Player] - Constructor"));
    // Setup Audio-Codec - Define I2C GPIOs
    while (not ac.begin(IIC_DATA, IIC_CLK))
    {
        Serial.printf("Failed!\n");
        delay(1000);
    }
    Serial.printf("OK\n");

    ac.SetMode(AC101::MODE_ADC_DAC);
    ac.SetVolumeSpeaker(20);
    ac.SetVolumeHeadphone(20);

    pinMode(GPIO_PA_EN, OUTPUT);
    digitalWrite(GPIO_PA_EN, HIGH);

    audioLogger = &Serial;
    out = new AudioOutputI2S();
    out->SetPinout(I2S_BCLK /*bclkPin*/, I2S_LRC /*lrcPin*/, I2S_DSIN /*dsinPin*/);
    
    file = new AudioFileSourceFS(SD_MMC);
    decoder_mp3a = new AudioGeneratorMP3a();
    decoder_flac = new AudioGeneratorFLAC();
}

SD_Player::~SD_Player(){
}

void SD_Player::setPlaylist(const char *uuid)
{
    if( !(decoder && decoder->isRunning()) ){
        Serial.println("Set Playlist");
        if(toggle)
            setTitle("/t.mp3");
        else 
            setTitle("/b.mp3");
        toggle=!toggle;
    }
}

void logMemory(){
    log_d("Used PSRAM: %d", ESP.getPsramSize() - ESP.getFreePsram());
    log_d("Total Heap: %d", ESP.getHeapSize());
    log_d("Free Heap: %d", ESP.getFreeHeap());
}

void SD_Player::setTitle(const char *path)
{
    logMemory();
    Serial.println("Set Title");
    file->open(path);
    buff = new AudioFileSourceBuffer(file, psdRamBuffer, 1024);
    //buff = new AudioFileSourceBuffer(file, 1024);
    if (String(path).endsWith(".mp3")){
        decoder = decoder_mp3a;
    }
    if (String(path).endsWith(".flac")){
        decoder = decoder_flac;
    }
    decoder->begin(file, out);
    logMemory();
}

void SD_Player::play()
{
    if(decoder && decoder->isRunning()){
        if (!decoder->loop())
        {
            Serial.println("Stopping");
            decoder->stop();
        } else {
            /*running...*/
        }
    }
}

void SD_Player::pause(){
    Serial.println(F("Pausing"));
    isPausing = true;
    decoder->stop();
}

void SD_Player::resume(){
    Serial.println(F("Resuming"));
    isPausing = false;
    decoder->begin(buff,out);
}

void SD_Player::stop(){
    Serial.println("File-pos: "+String(buff->getPos()));
    decoder->stop();
    buff->close();
}