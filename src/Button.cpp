#include "Button.hpp"

void Button::eval()
{
    // read the state of the switch/button:
    currentState = digitalRead(_pin);

    if (lastState == HIGH && currentState == LOW)
    {
        // button is pressed
        pressedTime = millis();
        lastState = currentState;
    }

    else if (lastState == LOW && currentState == HIGH)
    { // button is released
        releasedTime = millis();
        lastState = currentState;
        long pressDuration = releasedTime - pressedTime;

        if (pressDuration < SHORT_PRESS_TIME)
        {
            lastState = currentState;
            mode = PressDuration::shortpress;
            xQueueSend(_eventQueue->buttonQueue, &mode, 0);
            return;
        }

        if (pressDuration > LONG_PRESS_TIME && pressDuration < LONG_LONG_PRESS_TIME)
        {
            mode = PressDuration::longpress;
            xQueueSend(_eventQueue->buttonQueue, &mode, 0);
            return;
        }
        if (pressDuration > LONG_LONG_PRESS_TIME)
        {
            mode = PressDuration::longlongpress;
            xQueueSend(_eventQueue->buttonQueue, &mode, 0);
            return;
        }
    }
}