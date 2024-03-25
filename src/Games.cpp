#include "Games.hpp"

#include "str.hpp"
#include "input.hpp"
#include "Display.hpp"
#include "LedController.hpp"

void GameRunner::update(Display& display, Input& input, LedController& ledController)
{
    state.now = millis();
    state.deltaTime = state.now - state.lastFrame;
    
    const auto phase = state.phase;

    if(state.lastPhase != phase)
    {
        state.phaseStart = state.now;

        display.selectScreen(Display::Screen::Players);
        display.clearRect();
    }

    state.phaseDuration = state.now - state.phaseStart;

    if(state.playerAlive == 0)
    {
        state.reset();
    }

    if(definition.update)
    {
        definition.update(state, display, input, ledController);
    }

    if(phase == GameState::Phase::Init)
    {
        state.nextPhase();
    }
    else if(phase == GameState::Phase::Title)
    {
        constexpr auto titleDuration = 2000;

        display.selectPlayers(state.playerPresence);

        const auto title = definition.title;
        const auto len = strlen_P(title);
        const auto msgWidth = len * (Font::charWidth + 1);
        display.startDraw(Display::Width / 2 - msgWidth / 2, 4 * 8, msgWidth, 8);
        display.printP(title);

        if(state.phaseDuration >= titleDuration)
        {
            state.nextPhase();
        }
    }
    else if(phase == GameState::Phase::Demo)
    {

    }
    else if(phase == GameState::Phase::Instructions)
    {
        for(uint8_t x = 0; x < state.maxPlayerCount; x++)
        {
            if(!state.isPlayerPresent(x))
            {
                continue;
            }

            if(state.lastPhase != phase)
            {
                display.selectScreenfromIndex(x);
                display.startDraw(0, Display::Height - 8, Display::Width, 8);
                display.printP("Press button to ready"_PSTR);
            }

            if(input.isNewPressed(input.buttonFromIndex(x)))
            {
                state.setPlayerReady(x);

                display.selectScreenfromIndex(x);
                display.clearRect(0, Display::Height - 8, Display::Width, 8);

                constexpr auto msg = "READY!"_PSTR;
                const auto msgWidth = strlen_constexpr(msg) * (Font::charWidth + 1);
                display.startDraw(Display::Width / 2 - msgWidth / 2, Display::Height - 8, msgWidth, 8);
                display.printP(msg);
            }
        }

        if(state.areAllPlayersReady())
        {
            state.nextPhase();
        }
    }
    else if(phase == GameState::Phase::Countdown)
    {
        display.selectPlayers(state.playerPresence);

        if(state.phaseDuration < 1000)
        {
            display.draw(Glyphs::big3, Display::Width / 2 - 32 / 2, Display::Height / 2 - 48 / 2);
        }
        else if(state.phaseDuration < 2000)
        {
            display.draw(Glyphs::big2, Display::Width / 2 - 32 / 2, Display::Height / 2 - 48 / 2);
        }
        else if(state.phaseDuration < 3000)
        {
            display.draw(Glyphs::big1, Display::Width / 2 - 32 / 2, Display::Height / 2 - 48 / 2);
        }
        else
        {
            state.nextPhase();
        }
    }

    state.lastPhase = phase;
    state.lastFrame = state.now;
}