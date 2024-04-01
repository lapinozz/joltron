#include "pong.hpp"

#include <Arduino.h>

#include "../Games.hpp"

#include "str.hpp"
#include "input.hpp"
#include "Display.hpp"
#include "LedController.hpp"

namespace Pong
{
    constexpr uint8_t fieldHeight = Display::Height;
    constexpr uint8_t fieldWidth = fieldHeight;
    constexpr uint8_t fieldX = (Display::Width - fieldWidth) / 2;
    constexpr uint8_t fieldY = 0;

    constexpr uint8_t ballSize = 2;

    constexpr uint32_t demoDuration = 20000;
    constexpr uint32_t demoGameDuration = 10000;

    void drawField(GameState& state)
    {
        auto& data = state.data.pong;

        for(uint8_t x = 0; x < fieldWidth - 2; x++)
        {
            data.buffer.setPixel(x + 1, 0, true);
            data.buffer.setPixel(x + 1, fieldHeight - 1, true);
        }
        
        for(uint8_t x = 0; x < fieldWidth; x++)
        {
            data.buffer.setPixel(0, x, true);
            data.buffer.setPixel(fieldWidth - 1, x, true);
        }
    }

    void drawBall(GameState& state, Display& display, bool set)
    {
        auto& data = state.data.pong;

        const auto displayBallX = data.ballX.getInteger();
        const auto displayBallY = data.ballY.getInteger();
        
        for(uint8_t x = 0; x < ballSize; x++)
        {
            for(uint8_t y = 0; y < ballSize; y++)
            {
                data.buffer.setPixel(displayBallX + x, displayBallY + y, set);
            }
        }
    }

    void drawPaddles(GameState& state, Display& display, bool set)
    {
        auto& data = state.data.pong;

        for(uint8_t x = 0; x < state.maxPlayerCount; x++)
        {
            for(uint8_t y = 0; y < data.paddleLength; y++)
            {
                const auto position = data.paddlePositions[x].getInteger();

                if(x == 0)
                {
                    data.buffer.setPixel(data.paddleOffset, position + y, set);
                }
                else if(x == 1)
                {
                    data.buffer.setPixel(fieldWidth - data.paddleOffset - 1, position + y, set);
                }
                else if(x == 2)
                {
                    data.buffer.setPixel(position + y, data.paddleOffset, set);
                }
                else if(x == 3)
                {
                    data.buffer.setPixel(position + y, fieldHeight - data.paddleOffset - 1, set);
                }
            }
        }
    }

    void moveBall(GameState& state)
    {
        auto& data = state.data.pong;

        const auto ballPaddleoverlap = [&](FixedPoint ballPosition, FixedPoint paddlePosition)
        {
            return ballPosition + ballSize > paddlePosition && ballPosition < paddlePosition + data.paddleLength;
        };

        const auto deltaX = data.ballVelX * state.deltaTime;
        const auto deltaY = data.ballVelY * state.deltaTime;

        if(data.ballX + deltaX >= fieldWidth - ballSize)
        {
            state.onPlayerDie(1);
        }
        else if(data.ballX + deltaX >= fieldWidth - ballSize - data.paddleOffset && ballPaddleoverlap(data.ballY, data.paddlePositions[1]))
        {
            data.ballVelX = -data.ballVelX;
            data.ballX.setFraction(0);
            data.ballVelY += (data.paddleDirections & (1 << 1)) ? data.paddleBoost : -data.paddleBoost;
            data.bounceCount++;
        }
        else if(deltaX < 0 && (1 - deltaX) >= data.ballX)
        {
            state.onPlayerDie(0);
        }
        else if(deltaX < 0 && (1 - deltaX) + data.paddleOffset >= data.ballX &&  ballPaddleoverlap(data.ballY, data.paddlePositions[0]))
        {
            data.ballVelX = -data.ballVelX;
            data.ballX.setFraction(0);
            data.ballVelY += (data.paddleDirections & (1 << 0)) ? data.paddleBoost : -data.paddleBoost;
            data.bounceCount++;
        }
        else
        {
            data.ballX += deltaX;
        }

        if(data.ballY + deltaY >= fieldWidth - ballSize)
        {
            state.onPlayerDie(3);
        }
        else if(data.ballY + deltaY >= fieldWidth - ballSize - data.paddleOffset && ballPaddleoverlap(data.ballX, data.paddlePositions[3]))
        {
            data.ballVelY = -data.ballVelY;
            data.ballY.setFraction(0);
            data.ballVelX += (data.paddleDirections & (1 << 2)) ? data.paddleBoost : -data.paddleBoost;
            data.bounceCount++;
        }
        else if(deltaY < 0 && (1 - deltaY) >= data.ballY)
        {
            state.onPlayerDie(2);
        }
        else if(deltaY < 0 && (1 - deltaY) + data.paddleOffset >= data.ballY &&  ballPaddleoverlap(data.ballX, data.paddlePositions[2]))
        {
            data.ballVelY = -data.ballVelY;
            data.ballY.setFraction(0);
            data.ballVelX += (data.paddleDirections & (1 << 2)) ? data.paddleBoost : -data.paddleBoost;
            data.bounceCount++;
        }
        else
        {
            data.ballY += deltaY;
        }
    }

    void movePaddles(GameState& state)
    {
        auto& data = state.data.pong;

        for(uint8_t x = 0; x < state.maxPlayerCount; x++)
        {
            const auto direction = data.paddleDirections & (1 << x);
            auto& position = data.paddlePositions[x];


            if(!state.isPlayerAlive(x) || state.phase == GameState::Phase::Demo)
            {
                const auto ballCenter = (x <= 1 ? data.ballY : data.ballX) + ballSize / 2;
                position = ballCenter - data.paddleLength / 2;
            }
            else
            {
                const auto speed = data.paddleSpeed;
                position += (direction ? speed : -speed) * state.deltaTime;
            }

            if(position < 1)
            {
                position = 1;
            }
            else if(position > fieldWidth - data.paddleLength - 1)
            {
                position = fieldWidth - data.paddleLength - 1;
            }
        }
    }

    void controlPaddles(GameState& state, Input& input)
    {
        auto& data = state.data.pong;

        for(uint8_t x = 0; x < state.maxPlayerCount; x++)
        {
            auto direction = data.paddleDirections & (1 << x);

            if(state.isPlayerAlive(x) && state.phase != GameState::Phase::Demo)
            {
                if(input.isNewPressed(input.buttonFromIndex(x)))
                {
                    direction = !direction;
                }
            }
            else
            {
                direction = !direction;
            }

            if(direction)
            {
                data.paddleDirections |= (1 << x);
            }
            else
            {
                data.paddleDirections &= ~(1 << x);
            }
        }
    }

    void init(GameState& state)
    {
        auto& data = state.data.pong;

        data.ballX = fieldWidth / 2;
        data.ballY = fieldWidth / 2;

        const auto startSpeed = state.phase == GameState::Phase::Demo ? data.ballStartSpeed * 3 : data.ballStartSpeed;

        const auto ballVelX = startSpeed / 3;
        const auto ballVelY = startSpeed - ballVelX;

        data.ballVelX = FixedPoint(0, ballVelX);
        data.ballVelY = FixedPoint(0, ballVelY);

        if(random(0, 2))
        {
            data.ballVelX = -data.ballVelX;
        }

        if(random(0, 2))
        {
            data.ballVelY = -data.ballVelY;
        }

        if(random(0, 2))
        {
            const auto temp = data.ballVelY;
            data.ballVelY = data.ballVelX;
            data.ballVelX = temp;
        }

        if(state.phase != GameState::Phase::Demo)
        {
            for(uint8_t x = 0; x < 8; x++)
            {
                data.ballX = data.ballX - data.ballVelX * FixedPoint(100);
                data.ballY = data.ballY - data.ballVelY * FixedPoint(100);
            }
        }

        data.paddleDirections = 0;

        for(uint8_t x = 0; x < GameState::maxPlayerCount; x++)
        {
            data.paddlePositions[x] = fieldWidth / 2 - data.paddleLength / 2;
        }
    }

    void drawInstructions(GameState& state, Display& display)
    {
        static constexpr const char* instructionStr = "  You   control   the   paddle on the "_PSTR;
        static constexpr const char* directions[] = 
        {
            "left"_PSTR,
            "right"_PSTR,
            "top"_PSTR,
            "bottom"_PSTR,
        };
        
        static constexpr const char* arrows[] = 
        {
            "<-"_PSTR,
            "->"_PSTR,
            "^|"_PSTR,
            "|v"_PSTR,
        };

        for(int8_t x = 0; x < state.maxPlayerCount; x++)
        {
            if(!state.isPlayerAlive(x))
            {
                continue;
            }

            display.selectScreenfromIndex(x);
            display.startDraw(0, 1*8, 128, 8*6);
            display.printP(instructionStr);
            display.printP(directions[x]);

            if(x == 0 || x == 1)
            {
                const auto width = (Font::charWidth + 1) * 2;
                display.startDraw(Display::Width / 2 - width / 2, 4*8, width, 8);
                display.printP(arrows[x]);
            }
            else
            {
                const auto width = Font::charWidth + 1;
                display.startDraw(Display::Width / 2 - width / 2, 4*8, width, 8*2);
                display.printP(arrows[x]);
            }
        }
    }

    void drawDemo(GameState& state, Display& display)
    {
        static constexpr const char* instruction = "Press the button to  change the direction of your paddle.      If the ball touches  the wall on your sideyou loose"_PSTR;        
        display.startDraw(0, 1*8, 126, 8*6);
        display.printP(instruction);
    }

    void update(GameState& state, Display& display, Input& input, LedController& ledController)
    {
        auto& data = state.data.pong;

        if(state.phase != state.lastPhase)
        {
            if(state.phase == GameState::Phase::Demo || state.phase == GameState::Phase::Running)
            {
                data = {};
                init(state);
                return;
            }
            else if(state.phase == GameState::Phase::Instructions)
            {
                drawInstructions(state, display);
                return;
            }
            else if(state.phase == GameState::Phase::GameResults)
            {
                state.advance();
            }
            else if(state.phase == GameState::Phase::Scores)
            {                
            }
        }

        if(state.phase != GameState::Phase::Running && state.phase != GameState::Phase::Demo)
        {
            return;
        }

        if(state.phase == GameState::Phase::Running && state.playerPresence != state.playerAlive)
        {
            state.advance();
            return;
        }

        display.selectPlayers(state.playerPresence);

        if(state.phase == GameState::Phase::Demo)
        {
            if(state.playCount[state.gameIndex] > 1)
            {
                state.advance();
                return;
            }

            if(state.phaseDuration >= demoDuration)
            {
                state.advance();
                return;
            }

            if(state.phaseDuration >= demoGameDuration)
            {
                if(state.phaseDuration - state.deltaTime < demoGameDuration)
                {
                    display.clearRect();
                }

                drawDemo(state, display);
                return;
            }
        }

        drawField(state);

        drawBall(state, display, false);
        drawPaddles(state, display, false);
        
        if(data.bounceCount / 3 <= data.paddleMaxLength - data.paddleMinLength)
        {
            data.paddleLength = data.paddleMaxLength - data.bounceCount / 3;
        }

        controlPaddles(state, input);

        moveBall(state);
        movePaddles(state);

        drawBall(state, display, true);
        drawPaddles(state, display, true);
        
        display.draw(data.buffer, fieldX, fieldY);
    }
}