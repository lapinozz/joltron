#include "voting.hpp"

#include <Arduino.h>

#include "../Games.hpp"

#include "str.hpp"
#include "input.hpp"
#include "Display.hpp"
#include "LedController.hpp"
#include "Sounds.hpp"
#include "debug.hpp"

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

    void computeDeaths(GameState& state)
    {
        auto& data = state.data.voting;

        if(state.playerCount < 1)
        {
            return;
        }

        uint8_t playerOrder[GameState::maxPlayerCount] = {};
        uint8_t playerVoteCount[GameState::maxPlayerCount] = {};
        uint8_t playerCount = 0;
        for(int8_t x = 0; x < GameState::maxPlayerCount; x++)
        {
            if(state.isPlayerPresent(x))
            {
                playerOrder[playerCount++] = x;
                playerVoteCount[data.playerVotes[x]]++;
            }
        }

        sort(playerOrder, playerVoteCount, playerCount);
        reverseArray(playerOrder, playerCount);

        if(playerVoteCount[playerOrder[0]] == playerVoteCount[playerOrder[1]])
        {
            return;
        }

        if(playerVoteCount[playerOrder[0]] == playerCount)
        {        
            for(int8_t x = 1; x < playerCount; x++)
            {
                state.onPlayerDie(playerOrder[x]);
            }
        }
        else
        {
            state.onPlayerDie(playerOrder[0]);
        }
    }

    void showResults(GameState& state, Display& display)
    {
        auto& data = state.data.voting;

        constexpr uint8_t heightMap[] = {Font::charHeight * 1, Font::charHeight * 4, Font::charHeight * 7, Font::charHeight * 4};
        constexpr uint8_t widthMap[] = {Display::Width / 2, Display::Width / 2 + 24, Display::Width / 2, Display::Width / 2 - 24};

        for(int8_t y = 0; y < state.maxPlayerCount; y++)
        {
            if(!state.isPlayerPresent(y))
            {
                continue;
            }

            display.selectScreenfromIndex(y);

            for(int8_t x = 0; x < state.maxPlayerCount; x++)
            {
                if(!state.isPlayerPresent(x))
                {
                    continue;
                }
                const auto direction = readPgm(directionMap[y][x]);

                display.startDraw(widthMap[direction] - 2, heightMap[direction], Font::charAdvance, Font::charHeight);
                display.print(state.names[x]);

                const auto vote = data.playerVotes[x];
                const auto voteDirection = readPgm(directionMap[x][vote]);

                if(direction == 0)
                {
                    if(voteDirection == 0 || voteDirection == 2)
                    {
                        display.draw(Glyphs::arrows[2 - voteDirection], widthMap[direction] - 4, heightMap[direction] + 8);
                    }
                    else if(voteDirection == 3)
                    {
                        display.draw(Glyphs::arrowsDiag[1], widthMap[direction] - 4 + 8, heightMap[direction] + 8);
                    }
                    else if(voteDirection == 1)
                    {
                        display.draw(Glyphs::arrowsDiag[2], widthMap[direction] - 4 - 8, heightMap[direction] + 8);
                    }
                }
                else if(direction == 2)
                {
                    if(voteDirection == 0 || voteDirection == 2)
                    {
                        display.draw(Glyphs::arrows[voteDirection], widthMap[direction] - 4, heightMap[direction] - 8);
                    }
                    else if(voteDirection == 3)
                    {
                        display.draw(Glyphs::arrowsDiag[3], widthMap[direction] - 4 - 8, heightMap[direction] - 8);
                    }
                    else if(voteDirection == 1)
                    {
                        display.draw(Glyphs::arrowsDiag[0], widthMap[direction] - 4 + 8, heightMap[direction] - 8);
                    }
                }
                else if(direction == 1)
                {
                    if(voteDirection == 0 || voteDirection == 2)
                    {
                        display.draw(Glyphs::arrows[(2 - voteDirection) + 1], widthMap[direction] - 4 - 9, heightMap[direction]);
                    }
                    else if(voteDirection == 1)
                    {
                        display.draw(Glyphs::arrowsDiag[3], widthMap[direction] - 4 - 8, heightMap[direction] - 8);
                    }
                    else if(voteDirection == 3)
                    {
                        display.draw(Glyphs::arrowsDiag[2], widthMap[direction] - 4 - 8, heightMap[direction] + 8);
                    }
                }
                else if(direction == 3)
                {
                    if(voteDirection == 0 || voteDirection == 2)
                    {
                        display.draw(Glyphs::arrows[voteDirection + 1], widthMap[direction] - 4 + 9, heightMap[direction]);
                    }
                    else if(voteDirection == 1)
                    {
                        display.draw(Glyphs::arrowsDiag[1], widthMap[direction] - 4 + 8, heightMap[direction] + 8);
                    }
                    else if(voteDirection == 3)
                    {
                        display.draw(Glyphs::arrowsDiag[0], widthMap[direction] - 4 + 8, heightMap[direction] - 8);
                    }
                }
            }
        }
    }

    void update(GameState& state, Display& display, Input& input, LedController& ledController, SoundController& soundController)
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
                soundController.play(Song::Voting);
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
            else if(state.phase == GameState::Phase::GameResults)
            {
                showResults(state, display);
            }
        }

        if(state.phase == GameState::Phase::GameResults)
        {
            if (state.phaseDuration > 4000)
            {
                state.advance();
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

        if constexpr(debug::fastPath)
        {
            //data.voteDone = state.playerAlive;
        }

        if(timeLeft < 0 || data.voteDone == state.playerAlive)
        {
            computeDeaths(state);
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