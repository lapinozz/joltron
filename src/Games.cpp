#include "Games.hpp"

#include "str.hpp"
#include "input.hpp"
#include "Display.hpp"
#include "LedController.hpp"

void GameRunner::update(Display& display, Input& input, LedController& ledController)
{
    if(state.gameIndex < 0)
    {
        return;
    }

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

    if(definition.update)
    {
        definition.update(state, display, input, ledController);
    }

    if(phase == GameState::Phase::Init)
    {
        state.playCount[state.gameIndex]++;
        state.advance();
    }
    else if(phase == GameState::Phase::Title)
    {
        display.selectPlayers(state.playerPresence);

        const auto title = definition.title;
        const auto len = strlen_P(title);
        const auto msgWidth = len * (Font::charWidth + 1);
        display.startDraw(Display::Width / 2 - msgWidth / 2, 2 * 8, msgWidth, 8);
        display.printP(title);

        display.startDraw(Display::Width / 2 - (Font::charWidth + 1) * 4, 4 * 8, 50, 8);
        display.printP("lvl. "_PSTR);
        display.print_L((long)state.playCount[state.gameIndex]);

        static constexpr const char* difficultyStrs[] = 
        {
            "EASY"_PSTR,
            "NORMAL"_PSTR,
            "HARD"_PSTR,
        };

        const auto difficulty = state.difficulty;
        if(difficulty != GameState::Difficulty::None)
        {
            display.startDraw(Display::Width / 2 - (Font::charWidth + 1) * 3, 6 * 8, 50, 8);
            display.printP(difficultyStrs[static_cast<int32_t>(difficulty)]);
        }

        constexpr auto titleDuration = 2000;
        if(state.phaseDuration >= titleDuration)
        {
            state.advance();
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
            state.advance();
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
            state.advance();
        }
    }
    else if(phase == GameState::Phase::Zaps)
    {
        if(state.lastPhase != phase)
        {
            for(int8_t x = 0; x < GameState::maxPlayerCount; x++)
            {
                if(state.isPlayerDead(x))
                {
                    //input.zap(x, 50);

                    state.scores[x]++;

                    display.selectScreenfromIndex(x);

                    constexpr auto msg = "ZAP!"_PSTR;
                    const auto msgWidth = strlen_constexpr(msg) * (Font::charWidth + 1);
                    display.startDraw(Display::Width / 2 - msgWidth / 2, 4 * 8, msgWidth, 8);
                    display.printP(msg);
                }
            }
        }

        constexpr auto zapsDuration = 1000;
        if(state.phaseDuration >= zapsDuration)
        {
            state.advance();
        }
    }
    else if(phase == GameState::Phase::GameResults)
    {
    }
    else if(phase == GameState::Phase::Scores)
    {
        if(state.lastPhase != phase)
        {            
            state.playerPresence = 0b1111;

            display.selectPlayers(state.playerPresence);

            display.startDraw(22, 0, Display::Width - 22, 8);
            display.printP("Player"_PSTR);

            display.startDraw(95 - Font::charAdvance * 4 / 2 + Font::charAdvance / 2, 0, Display::Width - 95, 8);
            display.printP("Zaps"_PSTR);

            uint8_t playerOrder[GameState::maxPlayerCount] = {};
            uint8_t playerCount = 0;
            for(int8_t x = 0; x < GameState::maxPlayerCount; x++)
            {
                if(state.isPlayerPresent(x))
                {
                    playerOrder[playerCount++] = x;
                }
            }

            sort(playerOrder, state.scores, playerCount);

            uint8_t ranking = 0;
            for(int8_t x = 0; x < playerCount; x++)
            {
                const auto playerIndex = playerOrder[x];
                const auto name = state.names[playerIndex];
                const auto score = state.scores[playerIndex];

                if(x == 0 || score != state.scores[playerOrder[x - 1]])
                {
                    ranking++;
                }

                display.startDraw(0, 8 + 2 * 8 * x, Display::Width - 0, 8);
                display.print('#');
                display.printSpace();
                display.print('0' + ranking);
                
                display.startDraw(22 + 15, 8 + 2 * 8 * x, Font::charAdvance, 8);
                display.print(name);

                display.startDraw(95, 8 + 2 * 8 * x, Display::Width - 95, 8);
                display.print_UL(score);
            }
        }
    }
    else if(phase == GameState::Phase::Finished)
    {
    }

    state.lastPhase = phase;
    state.lastFrame = state.now;
}