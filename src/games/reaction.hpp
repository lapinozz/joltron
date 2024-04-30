#pragma once

#include "avr/pgmspace.h"

#include "GameDefinition.hpp"
#include "DisplayBuffer.hpp"

#include "str.hpp"
#include "FixedPoint.hpp"

struct GameState;

namespace Reaction
{
    enum class ElementType
    {
        Sound,
        Leds,
        Shapes,

        Count
    };

    struct NumberCondition
    {
        enum class Comparaison : uint8_t
        {
            Equal,
            Less,
            AtLeast,
        };

        uint8_t number;
        Comparaison comparaison;
    };

    enum class ElementFulfillment
    {
        True,
        False,
        Any
    };

    struct SoundElement
    {
        bool wantPlaying{};
    };

    struct ShapesElement
    {
        enum class Shape
        {
            Square,
            Triangle,
            Circle,

            Count
        };

        static constexpr uint8_t maxShapeCount{4};

        uint8_t shapeCount{};
        
        Shape targetShape{};
        NumberCondition condition{};
    };

    struct LedsElement
    {
        enum class Colors
        {
            Red,
            Green,
            Blue,
            Yellow,
            Purple,

            Count
        };
        
        Colors targetColor{};
        NumberCondition condition{};
    };

    struct Data
    {
        uint8_t playerReacts{};
        uint8_t playerReactCorrect{};
        uint8_t playerReactionTimestamp[4]{};

        uint8_t ElementsActive{};
        uint8_t ElementsCorrect{};

        LedsElement ledsElement;
        SoundElement soundElement;
        ShapesElement shapeElement;
    };

    void update(GameState& state, Display& display, Input& input, LedController& ledController, SoundController& SoundController);

    constexpr GameDefinition definition
    {
        "Reaction"_PSTR,
        update
    };
}
