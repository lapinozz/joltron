#include "voting.hpp"

#include <Arduino.h>

#include "../Games.hpp"

#include "str.hpp"
#include "input.hpp"
#include "Display.hpp"
#include "LedController.hpp"

namespace Voting
{
    static PROGMEM constexpr uint8_t directionMap[GameState::maxPlayerCount][GameState::maxPlayerCount] =
    {
        {2, 0, 3, 1},
        {0, 2, 1, 3},
        {1, 3, 2, 0},
        {3, 1, 0, 2},
    };

    void drawInstructions(GameState& state, Display& display)
    {
        //static constexpr const char* instructionStr = "Vote for someone. The player with the most votes looses. If a player gets all the votes, everyone else looses instead."_PSTR;
        //static constexpr const char* instructionStr = "Vote for someone. If you get the most votes, you lose. If you get all the votes, the other players lose instead."_PSTR;
        static constexpr const char* instructionStr = "If you get the most   votes, you lose.     If  you get all the  votes the others lose instead"_PSTR;
        
        display.selectPlayers(state.playerAlive);

        display.startDraw(0, 1*8, 128, 8*5);
        display.printP(instructionStr);
    }

    void drawDirection(GameState& state, Display& display, uint8_t playerIndex)
    {
        auto& data = state.data.voting;

        const auto vote = data.playerVotes[playerIndex];
        const auto direction = readPgm(directionMap[playerIndex][vote]);

        display.selectScreenfromIndex(playerIndex);

        const auto& glyph = Glyphs::arrows[direction];
        display.draw(glyph, Display::Width / 2 - readPgm(glyph).width / 2, 0);
    }

    void init(GameState& state, Display& display)
    {
        auto& data = state.data.voting;
        data = {};

        for(int8_t x = 0; x < state.maxPlayerCount; x++)
        {
            if(!state.isPlayerAlive(x))
            {
               continue;
            }
            
            auto& vote = data.playerVotes[x];
            do
            {
                vote = random(state.maxPlayerCount);
            }
            while(!state.isPlayerAlive(vote));
        }
    }

    void update(GameState& state, Display& display, Input& input, LedController& ledController)
    {
        auto& data = state.data.voting;

        if(state.phase != state.lastPhase)
        {
            if(state.phase == GameState::Phase::Demo)
            {
                state.advance();
                return;
            }
            else if(state.phase == GameState::Phase::Init)
            {
                init(state, display);
            }
            else if(state.phase == GameState::Phase::Instructions)
            {
                drawInstructions(state, display);
                return;
            }
            else if(state.phase == GameState::Phase::Running)
            {
                for(int8_t x = 0; x < state.maxPlayerCount; x++)
                {
                    if(!state.isPlayerAlive(x))
                    {
                        continue;
                    }

                    drawDirection(state, display, x);
                }

                display.selectPlayers(state.playerAlive);

                display.startDraw(0, Display::Height - Font::charHeight * 3, Display::Width, Font::charHeight * 2);
                display.printP("Press to select next  player"_PSTR);

                display.startDraw(0, Display::Height - Font::charHeight, Display::Width, Font::charHeight);
                display.printP("Hold to confirm"_PSTR);
            }
        }

        if(state.phase != GameState::Phase::Running)
        {
            return;
        }

        display.selectPlayers(state.playerAlive);

        const uint8_t timerWidth = Font::charAdvance * 2;
        display.startDraw(Display::Width / 2 - timerWidth / 2, Font::charHeight * 3, timerWidth, Font::charHeight);

        const int8_t timeLeft = data.votingDuration - (state.phaseDuration / 1000);
        if(timeLeft < 10)
        {
            display.print_UL(0);
        }
        display.print_UL(timeLeft);

        if(timeLeft < 0 || data.voteDone == state.playerAlive)
        {
            state.advance();
        }

        for(int8_t x = 0; x < state.maxPlayerCount; x++)
        {
            if(!state.isPlayerAlive(x) || (data.voteDone & (1 << x)))
            {
               continue;
            }

            if(input.isNewRelease(input.buttonFromIndex(x)))
            {
                const auto prevVote = data.playerVotes[x];
                const auto prevDirection = readPgm(directionMap[x][prevVote]);

                for(uint8_t y = 0; y < state.maxPlayerCount; y++)
                {
                    const auto testDirection = (prevDirection + y + 1) % state.maxPlayerCount;
                    for(uint8_t newVote = 0; newVote < state.maxPlayerCount; newVote++)
                    {
                        if(!state.isPlayerAlive(newVote))
                        {
                            continue;
                        }

                        if(readPgm(directionMap[x][newVote]) == testDirection)
                        {
                            data.playerVotes[x] = newVote;
                            y = 5;
                            break;
                        }
                    }
                }

                drawDirection(state, display, x);
            }
            else if(input.isNewLongPressed(input.buttonFromIndex(x)))
            {
                data.voteDone |= (1 << x);

                display.selectScreenfromIndex(x);

                constexpr const char* msg = "Confirmed!"_PSTR;
                constexpr auto len = strlen_constexpr(msg);
                constexpr auto width = len * Font::charAdvance;

                display.startDraw(Display::Width / 2 - width / 2, 0, width, Font::charHeight);
                display.printP(msg);
            }
        }
    }
}