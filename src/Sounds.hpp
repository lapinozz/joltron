#pragma once

#include <stdint.h>

#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

enum class Song : uint8_t
{
    Intro = 1,
    Menu,
    Pong,
    Voting,
    Reaction_Intro,
    Reaction_Start,
    Reaction_01 = Reaction_Start,
    Reaction_02,
    Reaction_03,
    Reaction_04,
    Reaction_End = Reaction_04,
};

class SoundController
{
    SoftwareSerial serial = {3, 4}; // RX, TX
    DFRobotDFPlayerMini player;

public:
    void init()
    {
        serial.begin(9600);

        while (!player.begin(serial, false))
        {
            delay(500);
        }

        player.volume(20); //Set volume value. From 0 to 30
        //player.outputSetting(false, 31);  //Set volume value. From 0 to 30
    }

    void play(Song song)
    {
        player.play(static_cast<uint8_t>(song));
    }

    void pause()
    {
        player.pause();
    }

    void resume()
    {
        player.start();
    }

    void stop()
    {
        player.stop();
    }
};