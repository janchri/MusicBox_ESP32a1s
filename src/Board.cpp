#include "Board.hpp"

Board::Board(){
    // SD-Card
    while(!SD_MMC.begin()){
        Serial.println("...");
        delay(100);
    } 
    uint8_t cardType = SD_MMC.cardType();
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);

    if (cardType == CARD_NONE)
    {
        Serial.println(F("No SD card attached"));
        return;
    }

    Serial.print(F("SD Card Type: "));
    if (cardType == CARD_MMC)
    {
        Serial.println(F("MMC"));
    }
    else if (cardType == CARD_SD)
    {
        Serial.println(F("SDSC"));
    }
    else if (cardType == CARD_SDHC)
    {
        Serial.println(F("SDHC"));
    }
    else
    {
        Serial.println(F("UNKNOWN"));
    }
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    // SPIFFS
    if (!SPIFFS.begin(true))
    {
        Serial.println(F("An Error has occurred while mounting SPIFFS"));
        while (1)
            ;
    }
    Serial.println("SPIFFS-TotalBytes: " + (String)SPIFFS.totalBytes());
    Serial.println("SPIFFS-UsedBytes: " + (String)SPIFFS.usedBytes());
}