#include "Rfid.hpp"

void rfid_task(void *pvParameter)
{
    Serial.println(F("RFID - Start Task"));
    for (;;)
    {
        ((Rfid *)pvParameter)->rfidScanner();
        yield();
        delay(1000);
    }
}

Rfid::Rfid(EventQueue *eventQueue) : _eventQueue(eventQueue){
    mfrc522 = new MFRC522(RFID_CS, RFID_RST);
    SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_CS);
    SPI.setFrequency(10000000);

    xTaskCreatePinnedToCore(
        rfid_task,   // Task function.
        "rfid_task", // name of task.
        2048,        // Stack size of task
        this,        // parameter of the task
        1,           // priority of the task
        NULL,        // Task handle to keep track of created task
        0);          // pin task to core <>
}

void Rfid::rfidInfo()
{
    mfrc522->PCD_DumpVersionToSerial();
}

bool Rfid::rfidScanner()
{
    //if (!mfrc522->PICC_IsNewCardPresent()) return;
    char n_currCardID[9];
    byte bufferATQA[2];
    byte bufferSize = sizeof(bufferATQA);
    mfrc522->PICC_WakeupA(bufferATQA, &bufferSize);
    if (!mfrc522->PICC_ReadCardSerial())
    {
        n_currCardID[0] = '\0';
        Serial.println(String(n_currCardID));
        xQueueSend(_eventQueue->rfidQueue, &n_currCardID, 0);
        return false; // Verify if the NUID has been readed
    }
    char *ptr = &n_currCardID[0];
    for (int i = 0; i < mfrc522->uid.size; i++)
    {
        ptr += sprintf(ptr, "%02x", mfrc522->uid.uidByte[i]);
    }

    mfrc522->PICC_HaltA();
    mfrc522->PCD_StopCrypto1();
    Serial.println(String(n_currCardID));
    xQueueSend(_eventQueue->rfidQueue, &n_currCardID, 0);
    return true;
}