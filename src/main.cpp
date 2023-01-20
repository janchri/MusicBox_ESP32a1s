#include <Arduino.h>

#include "Restserver.hpp"
#include "SD_Player.hpp"
#include "Blue_Player.hpp"
#include "Board.hpp"
#include "Rfid.hpp"
#include "Network.hpp"
#include "EventQueue.hpp"
#include "Button.hpp"

Board *board;
//Rfid *rfid;
Network *network;
EventQueue *eventQueue;
//Button *button;
//SD_Player *sd_player;
//Blue_Player *blue_player;
Restserver *restserver;

void setup()
{
  Serial.begin(115200);
  board = new Board();
  network = new Network();
  eventQueue = new EventQueue();
  //rfid = new Rfid(eventQueue);
  //button = new Button(eventQueue,22);
  //sd_player = new SD_Player();
  // blue_player = new Blue_Player(board);
  restserver = new Restserver(board, network, eventQueue);
  //disableCore0WDT();
  //disableCore1WDT();
  //disableLoopWDT();
}


void loop()
{
            yield();
            delay(1001);
  /*sd_player->setPlaylist("1234");
  sd_player->play();
  static int pause_lastms = 0;
  static int stop_lastms = 0;
  static bool isPausing = false;

  if (millis() - pause_lastms > 2000)
  {
    pause_lastms = millis();
    Serial.println("Pause/Resume time: " + String(pause_lastms));
    if (!isPausing){
      //sd_player->pause();
      isPausing = true;
    }
    else {
      //sd_player->resume();
      isPausing = false;
    }
  }

  if (millis() - stop_lastms > 15000)
  {
    stop_lastms = millis();
    Serial.println("Stop time: " + String(stop_lastms));
    sd_player->stop();
  }*/
}