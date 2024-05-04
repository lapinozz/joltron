#pragma once

#include "avr/pgmspace.h"

#include "GameDefinition.hpp"
#include "DisplayBuffer.hpp"

#include "str.hpp"
#include "FixedPoint.hpp"

struct GameState;

namespace Reaction
{
    enum ElementType
    {
        Sound,
        Leds,
        Shapes,

        Count
    };

    struct NumberCondition
    {
        enum Comparaison : uint8_t
        {
            Equal,
            Less,
            AtLeast,

            Count
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
        enum Shape : uint8_t
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
        enum Colors : uint8_t
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

    struct Timing
    {
        uint32_t minWait;
        uint32_t maxWait;

        uint32_t lastChange;
        uint32_t nextChange;
    };

    struct Data
    {
        uint8_t playerReacts{};
        uint8_t playerReactCorrect{};
        uint32_t playerReactionTimestamp[4]{};
        uint32_t firstReactionTimestamp{};

        bool hasIncorrectReaction = false;

        uint32_t timeToReactAfterFirst = 1000 * 2;

        uint32_t firstCorrectTime = 0;

        uint8_t elementsActive{};
        uint8_t elementsCorrect{};

        LedsElement ledsElement{};
        SoundElement soundElement{};
        ShapesElement shapeElement{};

        Timing timings[ElementType::Count]{};

        uint32_t gracePeriod = 3000;

        uint32_t duration = 0;
        uint32_t minDuration = static_cast<uint32_t>(1000) * 20;
        uint32_t maxDuration = static_cast<uint32_t>(1000) * 60 * 2;

        bool firstTrigger = true;
    };

    void update(GameState& state, Display& display, Input& input, LedController& ledController, SoundController& SoundController);

    constexpr GameDefinition definition
    {
        "Reaction"_PSTR,
        update
    };
}
