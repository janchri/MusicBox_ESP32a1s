#include "EventQueue.hpp"

EventQueue::EventQueue(){
    // Create Queues
    buttonQueue = xQueueCreate(1, sizeof(int));
    if (buttonQueue == NULL)
    {
        Serial.println(F("Failed to create buttonQueue"));
    }
    rfidQueue = xQueueCreate(1, sizeof(char[9]));
    if (rfidQueue == NULL)
    {
        Serial.println(F("Failed to create rfidQueue"));
    }
    currTrackQueue = xQueueCreate(1, sizeof(size_t));
    if (currTrackQueue == NULL)
    {
        Serial.println(F("Failed to create currTrackQueue"));
    }
    currTimestampQueue = xQueueCreate(1, sizeof(uint32_t));
    if (currTimestampQueue == NULL)
    {
        Serial.println(F("Failed to create currTimestampQueue"));
    }
    currVolumeQueue = xQueueCreate(1, sizeof(uint8_t));
    if (currVolumeQueue == NULL)
    {
        Serial.println(F("Failed to create currVolumeQueue"));
    }
}