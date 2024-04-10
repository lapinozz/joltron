#pragma once

#include <stdint.h>
#include "avr/pgmspace.h"

#include "utils.hpp"
#include "GameDefinition.hpp"

#include "games/pong.hpp"
#include "games/reaction.hpp"

static constexpr PROGMEM const GameDefinition gameDefinitions[] =
{
    Pong::definition,
    Reaction::definition,
};

static constexpr uint8_t GameCount = sizeof(gameDefinitions) / sizeof(gameDefinitions[0]);

struct GameState
{
    enum class Phase : int8_t
    {
        None = -1,
        Init,
        Title,
        Demo,
        Instructions,
        Countdown,
        Running,
        Zaps,
        GameResults,
        Scores,
        Finished
    };

    enum class Difficulty : int8_t
    {
        None = -1,
        Easy,
        Normal,
        Hard
    };

    uint8_t deltaTime = {};
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
    uint8_t playerCount = 0;
    uint8_t playerPresence = 0;
    uint8_t playerAlive = playerPresence;
    uint8_t playerReady = 0;
    uint8_t scores[maxPlayerCount] = {};
    char names[maxPlayerCount] = {'A', 'B', 'C', 'D'};

    int8_t gameIndex = -1;
    uint8_t playCount[GameCount] = {};
    
    Difficulty difficulty = Difficulty::None;

    void init()
    {
        phase = Phase::Init;
        lastPhase = Phase::None;
        playerAlive = playerPresence;
        playerReady = 0;
        difficulty = Difficulty::None;
    }

    void playerJoin(uint8_t index)
    {
        if(isPlayerPresent(index))
        {
            return;
        }

        playerPresence |= (1 << index);
        names[index] = 'A' + playerCount;
        playerCount++;
    }

    bool isPlayerPresent(uint8_t index) const
    {
        return playerPresence & (1 << index);
    }

    bool isPlayerAlive(uint8_t index) const
    {
        return playerPresence & playerAlive & (1 << index);
    }

    bool isPlayerDead(uint8_t index) const
    {
        return playerPresence & ~playerAlive & (1 << index);
    }

    void onPlayerDie(uint8_t index)
    {
        playerAlive &= ~(1 << index);
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

    void advance()
    {
        phaseDuration = 0;
        phase = static_cast<Phase>(static_cast<uint8_t>(phase) + 1);
    }
};

struct GameRunner
{
    GameDefinition definition = {};
    GameState state = {};

    void setGame(int8_t index)
    {
        if(index >= 0)
        {
            definition = readPgm(gameDefinitions[index]);
        }
        else
        {
            definition = {};
        }

        state.init();
        state.gameIndex = index;
    }

    void update(uint8_t deltaTime, Display& display, Input& input, LedController& ledController);
};