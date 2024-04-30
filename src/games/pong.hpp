#pragma once

#include "avr/pgmspace.h"

#include "GameDefinition.hpp"
#include "DisplayBuffer.hpp"

#include "str.hpp"
#include "FixedPoint.hpp"

struct GameState;

namespace Pong
{
    struct Data
    {
        FixedPoint ballX = 64 / 2;
        FixedPoint ballY = 64 / 2;

        uint8_t ballStartSpeed = 6; 

        FixedPoint ballVelX = FixedPoint(0, 5);
        FixedPoint ballVelY = FixedPoint(0, 5);

        int8_t paddleMaxLength = 20;
        int8_t paddleMinLength = 8;
        int8_t paddleLength = paddleMaxLength;

        int8_t paddleOffset = 3;
        FixedPoint paddleSpeed = FixedPoint(0, 10);
        FixedPoint paddleBoost = FixedPoint(0, 2);
        int8_t paddleDirections = {};
        FixedPoint paddlePositions[4] = {};

        uint8_t bounceCount = 0;

        DisplayBuffer<64, 64> buffer;
    };

    void update(GameState& state, Display& display, Input& input, LedController& ledController, SoundController& SoundController);

    constexpr GameDefinition definition
    {
        "Pong"_PSTR,
        update
    };
}
