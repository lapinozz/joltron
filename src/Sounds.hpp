#pragma once

#include <stdint.h>

#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "debug.hpp"
#include "settings.hpp"

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
    Reaction_End = Reaction_03,

    None
};

class SoundController
{
    SoftwareSerial serial = {3, 4}; // RX, TX
    DFRobotDFPlayerMini player;

    bool isInit = false;

    uint8_t volume = -1;
    uint8_t wantVolume;

    bool isLoop = false;
    bool wantLoop = true;

    Song song = Song::None;
    Song wantSong = Song::None;

    enum class State
    {
        Playing,
        Paused,
        Stopped
    };

    State state = State::Stopped;
    State wantState = State::Stopped;

    uint32_t lastUpdate = 0;
    uint32_t updateDelay = 300;

public:
    void init()
    {
        serial.begin(9600);
    }

    void update()
    {
        const auto now = millis();
        if(now - lastUpdate < updateDelay)
        {
            return;
        }

        lastUpdate = now;

        if(debug::fastPath)
        {
            wantVolume = 5;
        }
        else
        {
            wantVolume = settings.volume; 
        }

        if(!isInit)
        {
            isInit = player.begin(serial, false);
            return;
        }

        if(isLoop != wantLoop)
        {
            isLoop = wantLoop;
            if(isLoop)
            {
                player.enableLoop();
            }
            else
            {
                player.disableLoop();
            }

            return;
        }

        if(volume != wantVolume)
        {
            volume = wantVolume;
            player.volume(volume); //Set volume value. From 0 to 30

            return;
        }

        if(wantSong != song)
        {
            song = wantSong;
            state = State::Playing;

            player.play(static_cast<uint8_t>(song));

            return;
        }

        if(wantState != state)
        {
            state = wantState;
            if(state == State::Paused)
            {
                player.pause();
            }
            else if(state == State::Playing)
            {
                player.start();
            }
            else if(state == State::Stopped)
            {
                song = Song::None;
                player.stop();
            }

            return;
        }
    }

    void play(Song song)
    {
        wantSong = song;
        wantState = State::Playing;
    }

    void pause()
    {
        wantState = State::Paused;
    }

    void resume()
    {
        wantState = State::Playing;
    }

    void stop()
    {
        wantState = State::Stopped;
    }
};