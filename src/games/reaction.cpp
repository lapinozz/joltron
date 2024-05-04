#include "reaction.hpp"

#include <Arduino.h>

#include "../Games.hpp"

#include "str.hpp"
#include "input.hpp"
#include "Display.hpp"
#include "LedController.hpp"
#include "Sounds.hpp"
#include "debug.hpp"

namespace Reaction
{
    uint8_t generateTargetNumber(uint8_t lenght, NumberCondition condition, bool correct)
    {
        uint8_t targetCount = -1;

        if(condition.comparaison == NumberCondition::Comparaison::Equal)
        {
            if(correct)
            {
                targetCount = condition.number;
            }
            else
            {
                targetCount = random(0, lenght);
                if(targetCount >= condition.number)
                {
                    targetCount++;
                }
            }
        }
        else if(condition.comparaison == NumberCondition::Comparaison::Less)
        {
            if(correct)
            {
                targetCount = random(0, condition.number);
            }
            else
            {
                targetCount = random(condition.number, lenght + 1);
            }
        }
        else if(condition.comparaison == NumberCondition::Comparaison::AtLeast)
        {
            if(correct)
            {
                targetCount = random(condition.number, lenght + 1);
            }
            else
            {
                targetCount = random(0, condition.number);
            }
        }

        return targetCount;
    }

    void fillFromCondition(Array<uint8_t> data, uint8_t target, Array<uint8_t> fillers, NumberCondition condition, bool correct)
    {
        const auto targetCount = generateTargetNumber(data.size, condition, correct);

        uint8_t used = 0;

        for(uint8_t x = 0; x < targetCount; x++)
        {
            data.data[used++] = target;
        }

        for(uint8_t fillerIndex = 0; fillerIndex < fillers.size - 1 && used < data.size; fillerIndex++)
        {
            const uint8_t count = random(0, data.size - used);
            for(uint8_t x = 0; x < count && used < data.size; x++)
            {
                data.data[used++] = fillers.data[fillerIndex];
            }
        }

        while(used < data.size)
        {
            data.data[used++] = fillers.data[fillers.size - 1];
        }
    }

    void drawInstructions(GameState& state, Display& display)
    {
        static constexpr const char* instructionStr = "Press the button when all the conditions   are met, be the      quickest to win!"_PSTR;
        
        display.selectPlayers(state.playerAlive);

        display.startDraw(0, 1*8, 128, 8*5);
        display.printP(instructionStr);
    }

    void setSoundElement(const SoundElement& element, bool correct, SoundController& SoundController)
    {
        const bool play = element.wantPlaying == correct;
        if(play)
        {
            SoundController.resume();
        }
        else
        {
            SoundController.pause();
        }
    }

    void setShapeElement(const ShapesElement& element, bool correct, Display& display)
    {
        uint8_t data[ShapesElement::maxShapeCount] = {};

        uint8_t fillers[static_cast<uint8_t>(ShapesElement::Shape::Count) - 1] = {};
        uint8_t used = 0;
        for(uint8_t x = 0; x < sizeof(fillers) + 1; x++)
        {
            if(static_cast<uint8_t>(element.targetShape) != x)
            {
                fillers[used++] = x;
            }
        }
        shuffle(fillers, sizeof(fillers));

        fillFromCondition({data, element.shapeCount}, static_cast<uint8_t>(element.targetShape), {fillers, sizeof(fillers)}, element.condition, correct);

        shuffle(data, element.shapeCount);

        const auto width = 24;
        const auto spacing = (Display::Width - element.shapeCount * width) / (element.shapeCount + 1);

        display.selectMenu();
        for(uint8_t x = 0; x < element.shapeCount; x++)
        {
            display.draw(*readPgm(&Glyphs::shapes[data[x]]), spacing + (width + spacing) * x, 20);
        }
    }

    void setLedElement(const LedsElement& element, bool correct, LedController& ledController)
    {
        constexpr auto maxColorCount = static_cast<uint8_t>(LedsElement::Colors::Count);
        const auto colorCount = maxColorCount;

        constexpr uint8_t ledCount = 10; 

        uint8_t data[ledCount] = {};

        uint8_t fillers[maxColorCount - 1] = {};
        uint8_t used = 0;
        for(uint8_t x = 0; x < sizeof(fillers) + 1; x++)
        {
            if(static_cast<uint8_t>(element.targetColor) != x)
            {
                fillers[used++] = x;
            }
        }
        shuffle(fillers, sizeof(fillers));

        fillFromCondition({data, ledCount}, static_cast<uint8_t>(element.targetColor), {fillers, colorCount - 1}, element.condition, correct);

        shuffle(data, ledCount);

        static constexpr PROGMEM uint32_t colors[] =
        {
            {0xFF0000},
            {0x00FF00},
            {0x0000FF},
            {0xFFFF00},
            {0xA020F0},
        };

        for(uint8_t x = 0; x < ledCount; x++)
        {
            ledController.set(x, {readPgm(colors[data[x]])});
        }
    }

    void setElement(GameState& state, ElementType type, ElementFulfillment fulfillment, Display& display, Input& input, LedController& ledController, SoundController& soundController)
    {
        auto& data = state.data.reaction;

        if((data.elementsActive & (1 << static_cast<uint8_t>(type))) == 0)
        {
            return;
        }

        if(fulfillment == ElementFulfillment::Any)
        {
            fulfillment = Random::binary() ? ElementFulfillment::True : ElementFulfillment::False;
        }
        
        const bool correct = fulfillment == ElementFulfillment::True;

        auto& timing = data.timings[type];
        timing.lastChange = state.phaseDuration;
        timing.nextChange = timing.lastChange + random(timing.minWait, timing.maxWait); 

        if(type == ElementType::Shapes)
        {
            setShapeElement(data.shapeElement, correct, display);
        }
        else if(type == ElementType::Sound)
        {
            setSoundElement(data.soundElement, correct, soundController);
        }
        else if(type == ElementType::Leds)
        {
            setLedElement(data.ledsElement, correct, ledController);
        }

        if(correct)
        {
            data.elementsCorrect |= (1 << static_cast<uint8_t>(type));
        }
        else
        {
            data.elementsCorrect &= ~(1 << static_cast<uint8_t>(type));
        }
    }

    void drawConditions(GameState& state, Display& display)
    {
        display.selectPlayers(state.playerPresence);

        auto& data = state.data.reaction;

        if(state.difficulty == GameState::Difficulty::Easy)
        {
            display.startDraw(0, 16, Display::Width, Font::charHeight * 2);
            display.printP("Press button when the song stops"_PSTR);
            return;
        }

        static constexpr PROGMEM const char* comparaisons[] = 
        {
            "Exactly"_PSTR,
            "Less than"_PSTR,
            "At least"_PSTR,
        };

        static constexpr PROGMEM const char* colorNames[] = 
        {
            "Red"_PSTR,
            "Green"_PSTR,
            "Blue"_PSTR,
            "Yellow"_PSTR,
            "Purple"_PSTR,
        };
        
        static constexpr PROGMEM const char* shapeNames[] = 
        {
            "Square"_PSTR,
            "Triangle"_PSTR,
            "Circle"_PSTR,
        };

        uint8_t verticalPosition = 16;

        display.startDraw();
        display.printP("Conditions:"_PSTR);

        if(data.elementsActive & (1 << static_cast<uint8_t>(ElementType::Sound)))
        {
            display.startDraw(0, verticalPosition, 128, 8);
            if(data.soundElement.wantPlaying)
            {
                display.printP("Song is playing"_PSTR);
            }
            else
            {
                display.printP("Song is NOT playing"_PSTR);
            }

            verticalPosition += 16;
        }

        if(data.elementsActive & (1 << static_cast<uint8_t>(ElementType::Leds)))
        {
            display.startDraw(0, verticalPosition, 128, 8);

            display.printP(readPgm(&comparaisons[static_cast<uint8_t>(data.ledsElement.condition.comparaison)]));
            display.print(' ');
            display.print_UL(data.ledsElement.condition.number);
            display.print(' ');
            display.printP(readPgm(&colorNames[static_cast<uint8_t>(data.ledsElement.targetColor)]));
            display.SI2C.write(0, 0, 0, 0);
            display.printP("led"_PSTR);

            verticalPosition += 16;
        }

        if(data.elementsActive & (1 << static_cast<uint8_t>(ElementType::Leds)))
        {
            display.startDraw(0, verticalPosition, 128, 8);

            display.printP(readPgm(&comparaisons[static_cast<uint8_t>(data.shapeElement.condition.comparaison)]));
            display.print(' ');
            display.print_UL(data.shapeElement.condition.number);
            display.print(' ');
            display.printP(readPgm(&shapeNames[static_cast<uint8_t>(data.shapeElement.targetShape)]));

            verticalPosition += 16;
        }
    }

    void initElements(GameState& state, Display& display, Input& input, LedController& ledController, SoundController& soundController)
    {
        auto& data = state.data.reaction;

        if(debug::fastPath)
        {
            data.minDuration = 2000;
            data.maxDuration = 5000;
        }
        else
        {
            if(state.difficulty == GameState::Difficulty::Easy)
            {
                data.minDuration = static_cast<uint32_t>(1000) * 5;
                data.maxDuration = static_cast<uint32_t>(1000) * 40;
            }
        }

        data.duration = random(data.minDuration, data.maxDuration);

        if(state.difficulty == GameState::Difficulty::Easy)
        {
            data.elementsActive |= (1 << ElementType::Sound);
            data.soundElement.wantPlaying = false;

            data.timings[ElementType::Sound] =
            {
                data.duration, //min
                data.duration + 10, //max

                0,0
            };

            return;
        }

        data.elementsActive |= (1 << ElementType::Sound);
        if(data.elementsActive & (1 << ElementType::Sound))
        {
            data.soundElement.wantPlaying = Random::binary();

            data.timings[ElementType::Sound] =
            {
                3000, //min
                8000, //max

                0,0
            };
        }

        data.elementsActive |= (1 << ElementType::Leds);
        if(data.elementsActive & (1 << ElementType::Leds))
        {
            data.ledsElement.targetColor = static_cast<LedsElement::Colors>(random(0, LedsElement::Colors::Count));

            data.ledsElement.condition.comparaison = static_cast<NumberCondition::Comparaison>(random(0, NumberCondition::Comparaison::Count));
            if(data.ledsElement.condition.comparaison == NumberCondition::Comparaison::Equal)
            {
                data.ledsElement.condition.number = random(0, 10);
            }
            else if(data.ledsElement.condition.comparaison == NumberCondition::Comparaison::AtLeast)
            {
                data.ledsElement.condition.number = random(3, 10);
            }
            else if(data.ledsElement.condition.comparaison == NumberCondition::Comparaison::Less)
            {
                data.ledsElement.condition.number = random(3, 9);
            }

            data.timings[ElementType::Leds] =
            {
                3000, //min
                8000, //max

                0,0
            };
        }

        data.elementsActive |= (1 << ElementType::Shapes);
        if(data.elementsActive & (1 << ElementType::Shapes))
        {
            data.shapeElement.shapeCount = data.shapeElement.maxShapeCount;
            data.shapeElement.targetShape = static_cast<ShapesElement::Shape>(random(0, ShapesElement::Shape::Count));

            data.shapeElement.condition.comparaison = static_cast<NumberCondition::Comparaison>(random(0, NumberCondition::Comparaison::Count));
            if(data.shapeElement.condition.comparaison == NumberCondition::Comparaison::Equal)
            {
                data.shapeElement.condition.number = random(0, 5);
            }
            else if(data.shapeElement.condition.comparaison == NumberCondition::Comparaison::AtLeast)
            {
                data.shapeElement.condition.number = random(2, 5);
            }
            else if(data.shapeElement.condition.comparaison == NumberCondition::Comparaison::Less)
            {
                data.shapeElement.condition.number = random(2, 5);
            }

            data.timings[ElementType::Shapes] =
            {
                3000, //min
                8000, //max

                0,0
            };
        }
    }

    void initDifficulty(GameState& state)
    {
        auto& data = state.data.reaction;

        if(state.difficulty == GameState::Difficulty::None)
        {
            if(state.playCount[state.gameIndex] > 1)
            {
                state.difficulty = Random::binary() ? GameState::Difficulty::Easy : GameState::Difficulty::Hard;
            }
            else
            {
                state.difficulty = GameState::Difficulty::Easy;
            }
        }
        else if(state.difficulty == GameState::Difficulty::Normal)
        {
            state.difficulty = GameState::Difficulty::Hard;
        }
    }

    void updateTiming(GameState& state, Display& display, Input& input, LedController& ledController, SoundController& soundController)
    {
        auto& data = state.data.reaction;

        for(uint8_t x = 0; x < ElementType::Count; x++)
        {
            if((data.elementsActive & (1 << x)) == 0)
            {
                continue;
            }

            uint8_t activeCount = 0;
            uint8_t correctCount = 0;

            for(uint8_t y = 0; y < ElementType::Count; y++)
            {
                if((data.elementsActive & (1 << y)) != 0)
                {
                    activeCount++;
                }
                
                if((data.elementsCorrect & (1 << y)) != 0)
                {
                    correctCount++;
                }
            }

            const bool almostActive = correctCount >= activeCount - 1;

            const bool isGraced = state.phaseDuration <= data.gracePeriod;

            auto fullfillment = ElementFulfillment::Any;

            const bool canActivate = !almostActive || !isGraced;
            if(!canActivate)
            {
                fullfillment = ElementFulfillment::False;
            }

            const bool canDeactivate = state.phaseDuration < data.duration;
            if(!canDeactivate)
            {
                fullfillment = ElementFulfillment::True;
            }

            auto& timing = data.timings[x];
            const bool timeTrigger = state.phaseDuration >= timing.nextChange && (state.phaseDuration - state.deltaTime) < timing.nextChange;

            if(timeTrigger || data.firstTrigger)
            {
                setElement(state, static_cast<ElementType>(x), fullfillment, display, input, ledController, soundController);
            }
        }

        data.firstTrigger = false;
    }

    void updatePlayerInputs(GameState& state, Display& display, Input& input)
    {
        auto& data = state.data.reaction;
        
        for(int8_t x = 0; x < state.maxPlayerCount; x++)
        {
            if(!state.isPlayerPresent(x) || (data.playerReacts & (1 << x)))
            {
               continue;
            }

            if(!input.isNewPressed(input.buttonFromIndex(x)))
            {
                continue;
            }

            data.playerReacts |= (1 << x);

            const bool isCorrect = data.elementsCorrect == data.elementsActive;
            if(isCorrect)
            {
                data.playerReactCorrect |= (1 << x);
                data.playerReactionTimestamp[x] = state.phaseDuration;
            }
            else
            {
                data.hasIncorrectReaction = true;
            }

            if(data.firstReactionTimestamp == 0)
            {
                data.firstReactionTimestamp = state.phaseDuration;
            }
        }
    }

    void computeDeaths(GameState& state)
    {
        auto& data = state.data.reaction;

        if(data.hasIncorrectReaction)
        {
            for(int8_t x = 0; x < GameState::maxPlayerCount; x++)
            {
                if(!state.isPlayerPresent(x))
                {
                    continue;
                }

                if((data.playerReacts & (1 << x)) != 0 && (data.playerReactCorrect & (1 << x)) == 0)
                {
                    state.onPlayerDie(x);
                }
            }

            return;   
        }
        
        uint8_t playerOrder[GameState::maxPlayerCount] = {};
        uint32_t playerSpeed[GameState::maxPlayerCount] = {};
        uint8_t playerCount = 0;
        for(int8_t x = 0; x < GameState::maxPlayerCount; x++)
        {
            if(!state.isPlayerPresent(x))
            {
                continue;
            }

            if((data.playerReacts & (1 << x)) == 0)
            {
                continue;
            }

            if((data.playerReactCorrect & (1 << x)) == 0)
            {
                continue;
            }

            playerOrder[playerCount] = x;

            if(data.playerReactionTimestamp[x])
            {
                playerSpeed[playerCount] = data.playerReactionTimestamp[x] - data.firstCorrectTime;
            }
            else
            {
                playerSpeed[playerCount] = -1;
            }

            playerCount++;
        }

        for(int8_t x = 0; x < state.maxPlayerCount; x++)
        {
            if(!state.isPlayerPresent(x))
            {
                continue;
            }

            const bool hasReact = data.playerReacts & (1 << x);
            const bool hasCorrect = data.playerReactCorrect & (1 << x);

            if(!hasCorrect || !hasReact)
            {
                state.onPlayerDie(x);
            }
        }

        if(playerCount > 1)
        {
            sort(playerOrder, playerSpeed, playerCount);
        }
        else
        {
            return;
        }

        //const bool onlyLastDie = state.difficulty == GameState::Difficulty::Easy;
        const bool onlyLastDie = true;

        if(onlyLastDie)
        {
            state.onPlayerDie(playerOrder[playerCount - 1]);
        }
        else
        {
            for(int8_t x = 1; x < playerCount; x++)
            {
                state.onPlayerDie(playerOrder[x]);
            }
        }
    }

    void showResults(GameState& state, Display& display)
    {
        auto& data = state.data.reaction;

        uint8_t playerOrder[GameState::maxPlayerCount] = {};
        uint32_t playerSpeed[GameState::maxPlayerCount] = {};
        uint8_t playerCount = 0;
        for(int8_t x = 0; x < GameState::maxPlayerCount; x++)
        {
            if(!state.isPlayerPresent(x))
            {
                continue;
            }

            playerOrder[playerCount] = x;

            if(data.playerReactionTimestamp[x])
            {
                playerSpeed[playerCount] = data.playerReactionTimestamp[x] - data.firstCorrectTime;
            }
            else
            {
                playerSpeed[playerCount] = -1;
            }

            playerCount++;
        }

        sort(playerOrder, playerSpeed, playerCount);
        
        display.selectPlayers(state.playerPresence);

        display.startDraw(22, 0, Display::Width - 22, 8);
        display.printP("Player"_PSTR);

        display.startDraw(95 - Font::charAdvance * 4 / 2 + Font::charAdvance / 2, 0, Display::Width - 95, 8);
        display.printP("ms"_PSTR);

        uint8_t ranking = 0;
        for(int8_t x = 0; x < playerCount; x++)
        {
            const auto playerIndex = playerOrder[x];
            const auto name = state.names[playerIndex];
            const auto speed = playerSpeed[x];

            if(x == 0 || speed != playerSpeed[x - 1])
            {
                ranking++;
            }

            display.startDraw(0, 8 + 2 * 8 * x, Display::Width - 0, 8);
            display.print('#');
            display.printSpace();
            display.print('0' + ranking);
            
            display.startDraw(22 + 15, 8 + 2 * 8 * x, Font::charAdvance, 8);
            display.print(name);

            if(speed != -1UL)
            {
                display.startDraw(95, 8 + 2 * 8 * x, Display::Width - 95, 8);
                display.print_UL(speed);
            }
        }
    }

    void update(GameState& state, Display& display, Input& input, LedController& ledController, SoundController& soundController)
    {
        auto& data = state.data.reaction;

        if(state.phase != state.lastPhase)
        {
            if(state.phase == GameState::Phase::Init)
            {
                data = {};
                initDifficulty(state);
            }
            else if(state.phase == GameState::Phase::Demo)
            {
                state.advance();
                return;
            }
            else if(state.phase == GameState::Phase::Instructions)
            {
                soundController.play(Song::Reaction_Intro);
                drawInstructions(state, display);
                return;
            }
            else if(state.phase == GameState::Phase::Countdown)
            {
                const Song song = static_cast<Song>(random(static_cast<uint8_t>(Song::Reaction_Start), static_cast<uint8_t>(Song::Reaction_End) + 1));
                soundController.play(song);
            }
            else if(state.phase == GameState::Phase::Running)
            {
                initElements(state, display, input, ledController, soundController);
                drawConditions(state, display);
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

        updateTiming(state, display, input, ledController, soundController);
        updatePlayerInputs(state, display, input);

        if(data.firstCorrectTime == 0 && data.elementsActive == data.elementsCorrect)
        {
            data.firstCorrectTime = state.phaseDuration;
        }

        const bool allPlayerReacted = data.playerReacts == state.playerPresence;
        const bool timeExpired = data.firstReactionTimestamp != 0 && state.phaseDuration > data.firstReactionTimestamp + data.timeToReactAfterFirst;
        if(allPlayerReacted || timeExpired || data.hasIncorrectReaction)
        {
            computeDeaths(state);
            state.advance();
        }

        if(debug::fastPath)
        {
            display.selectMenu();
            display.startDraw();
            display.print((data.elementsCorrect & (1 << ElementType::Leds)) ? 'O' : 'X');
            display.print((data.elementsCorrect & (1 << ElementType::Sound)) ? 'O' : 'X');
            display.print((data.elementsCorrect & (1 << ElementType::Shapes)) ? 'O' : 'X');
            display.print((data.elementsActive == data.elementsCorrect) ? '!' : ' ');

            display.print_UL(state.phaseDuration);
            display.print(' ');
            display.print_UL(data.duration);
        }
    }
}