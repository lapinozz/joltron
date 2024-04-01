#include "reaction.hpp"

#include <Arduino.h>

#include "../Games.hpp"

#include "str.hpp"
#include "input.hpp"
#include "Display.hpp"
#include "LedController.hpp"

namespace Reaction
{
    void update(GameState& state, Display& display, Input& input, LedController& ledController)
    {
        auto& data = state.data.reaction;

        if(state.phase != state.lastPhase)
        {
            if(state.phase == GameState::Phase::Demo)
            {
                state.advance();
                return;
            }

            else if(state.phase == GameState::Phase::Instructions)
            {
                //drawInstructions(state, display);
                return;
            }
        }

        if(state.phase != GameState::Phase::Running)
        {
            return;
        }

        display.selectPlayers(state.playerPresence);
    }
}