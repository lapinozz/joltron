#pragma once

#include "avr/pgmspace.h"

#include "GameDefinition.hpp"
#include "DisplayBuffer.hpp"

#include "str.hpp"
#include "FixedPoint.hpp"

struct GameState;

namespace Reaction
{
    struct Data
    {
    };

    void update(GameState& state, Display& display, Input& input, LedController& ledController);

    constexpr GameDefinition definition
    {
        "Reaction"_PSTR,
        update
    };
}
