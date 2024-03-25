#pragma once

#include <stdint.h>
#include "avr/pgmspace.h"

#include "utils.hpp"
#include "GameDefinition.hpp"

#include "games/pong.hpp"
#include "games/reaction.hpp"

struct GameState
{
    enum Phase : uint8_t
    {
        None,
        Init,
        Title,
        Demo,
        Instructions,
        Countdown,
        Running,
        Scores,
        Finished
    };

    uint32_t now = {};
    uint32_t lastFrame = {};
    uint8_t deltaTime = {};

    uint32_t phaseStart = {};
    uint32_t phaseDuration = {};

    Phase phase;
    Phase lastPhase;

    union GameData
    {
        Pong::Data pong;
        Reaction::Data reaction;
    };

    GameData data;

    static constexpr uint8_t maxPlayerCount = 4; 
    uint8_t playerCount = maxPlayerCount;
    uint8_t playerPresence = 0b0001;
    uint8_t playerAlive = playerPresence;
    uint8_t playerReady = 0;

    void reset()
    {
        phase = Phase::Init;
        lastPhase = Phase::None;
        playerAlive = playerPresence;
        playerReady = 0;
    }

    bool isPlayerPresent(uint8_t index) const
    {
        return playerPresence & (1 << index);
    }

    bool isPlayerAlive(uint8_t index) const
    {
        return playerPresence & playerAlive & (1 << index);
    }

    void onPlayerLoose(uint8_t index)
    {
        playerAlive &= ~(1 << index);
        //while(true)
        {

        }
    }

    void setPlayerReady(uint8_t index)
    {
        playerReady |= (1 << index);
    }

    bool isPlayerReady(uint8_t index) const
    {
        return playerReady & (1 << index);
    }

    bool areAllPlayersReady() const
    {
        return playerReady == playerAlive;
    }

    void nextPhase()
    {
        phase = static_cast<Phase>(static_cast<uint8_t>(phase) + 1);
    }
};

struct GameRunner
{
    static constexpr PROGMEM const GameDefinition definitions[] =
    {
        Pong::definition,
        Reaction::definition,
    };

    static constexpr uint8_t GameCount = sizeof(definitions) / sizeof(definitions[0]);

    GameDefinition definition = {};

    GameState state = {};

    void setGame(uint8_t index)
    {
        definition = readPgm(definitions[index]);
        state.reset();
    }

    void update(Display& display, Input& input, LedController& ledController);
};