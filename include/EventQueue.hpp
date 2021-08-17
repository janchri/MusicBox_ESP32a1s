#ifndef EVENTQUEUE_HPP
#define EVENTQUEUE_HPP

#include <Arduino.h>

class EventQueue
{
private:
public:
    QueueHandle_t rfidQueue;
    QueueHandle_t buttonQueue;
    QueueHandle_t currTimestampQueue;
    QueueHandle_t currTrackQueue;
    QueueHandle_t currVolumeQueue;
};

#endif