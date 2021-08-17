#ifndef RFID_HPP
#define RFID_HPP

#include <SPI.h>
#include <MFRC522.h>

#include "EventQueue.hpp"

//VSPI
#define RFID_RST    99 //
#define RFID_CS     5 //
#define RFID_SCK    18 //
#define RFID_MISO   19 //
#define RFID_MOSI   23 //

class Rfid
{
private:
    EventQueue *_eventQueue;
    MFRC522 *mfrc522;

public:
    Rfid(EventQueue *eventQueue);
    void rfidInfo();
    bool rfidScanner();
};

#endif