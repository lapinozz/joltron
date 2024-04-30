#pragma once

#include "avr/pgmspace.h"

#include "GameDefinition.hpp"
#include "DisplayBuffer.hpp"

#include "str.hpp"
#include "FixedPoint.hpp"

struct GameState;

namespace Voting
{
    struct Data
    {
        uint8_t playerVotes[4] = {};
        uint8_t voteDone = {};

        int8_t votingDuration = 30; 
    };

    void update(GameState& state, Display& display, Input& input, LedController& ledController, SoundController& SoundController);

    constexpr GameDefinition definition
    {
        "Voting"_PSTR,
        update
    };
}
