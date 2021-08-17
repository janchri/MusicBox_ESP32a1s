#ifndef BUTTONS_HPP
#define BUTTONS_HPP

#include <Arduino.h>
#include "EventQueue.hpp"

enum PressDuration { nopress, shortpress, longpress, longlongpress };

class Button
{
private:
    
    int lastState = HIGH; // the previous state from the input pin
    int currentState = HIGH;    // the current reading from the input pin
    unsigned long pressedTime = 0;
    unsigned long releasedTime = 0;
    bool isPressing = false;
    bool isLongDetected = false;
    const int SHORT_PRESS_TIME = 1000; // 1000 milliseconds
    const int LONG_PRESS_TIME = 1000;  // 1000 milliseconds
    const int LONG_LONG_PRESS_TIME = 10000;  // 10000 milliseconds
    uint8_t _pin;
    EventQueue *_eventQueue;
    PressDuration mode = PressDuration::nopress;
public:
    Button(EventQueue *eventQueue, uint8_t pin) : _pin(pin), _eventQueue(eventQueue) {};
    void eval();
};

#endif