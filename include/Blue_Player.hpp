#ifndef BLUE_PLAYER_HPP
#define BLUE_PLAYER_HPP

#include <BluetoothA2DPSink.h>

#include "Board.hpp"

#define GPIO_PA_EN 21

class Blue_Player
{
private:
    BluetoothA2DPSink a2dp_sink;
    Board *board;
public:
    Blue_Player(Board *board);
    void listen();
    bool isConnected();
};

#endif