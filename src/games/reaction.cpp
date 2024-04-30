#include "reaction.hpp"

#include <Arduino.h>

#include "../Games.hpp"

#include "str.hpp"
#include "input.hpp"
#include "Display.hpp"
#include "LedController.hpp"
#include "Sounds.hpp"

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

        if(fulfillment == ElementFulfillment::Any)
        {
            fulfillment = Random::binary() ? ElementFulfillment::True : ElementFulfillment::False;
        }
        
        const bool correct = fulfillment == ElementFulfillment::True;

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
            data.ElementsCorrect |= (1 << static_cast<uint8_t>(type));
        }
        else
        {
            data.ElementsCorrect &= ~(1 << static_cast<uint8_t>(type));
        }
    }

    void drawConditions(GameState& state, Display& display)
    {
        auto& data = state.data.reaction;

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

        display.selectPlayers(state.playerPresence);

        display.startDraw();
        display.printP("Conditions:"_PSTR);

        if(data.ElementsActive & (1 << static_cast<uint8_t>(ElementType::Sound)))
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

        if(data.ElementsActive & (1 << static_cast<uint8_t>(ElementType::Leds)))
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

        if(data.ElementsActive & (1 << static_cast<uint8_t>(ElementType::Leds)))
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

        const Song song = static_cast<Song>(random(static_cast<uint8_t>(Song::Reaction_Start), static_cast<uint8_t>(Song::Reaction_End) + 1));
        soundController.play(song);

        data.shapeElement = {
            4,
            ShapesElement::Shape::Triangle,
            {2, NumberCondition::Comparaison::Less}
        };
        data.ledsElement = {
            LedsElement::Colors::Purple,
            {5, NumberCondition::Comparaison::Less}
        };

        data.ElementsActive = 0xff;
    }

    void update(GameState& state, Display& display, Input& input, LedController& ledController, SoundController& soundController)
    {
        auto& data = state.data.reaction;

        if(state.phase != state.lastPhase)
        {
            if(state.phase == GameState::Phase::Init)
            {
                data = {};
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
            else if(state.phase == GameState::Phase::Running)
            {
                initElements(state, display, input, ledController, soundController);
                drawConditions(state, display);
            }
        }

        if(state.phase != GameState::Phase::Running)
        {
            return;
        }

        display.selectPlayers(state.playerPresence);

        setElement(state, ElementType::Shapes, ElementFulfillment::True, display, input, ledController, soundController);

        setElement(state, ElementType::Leds, ElementFulfillment::False, display, input, ledController, soundController);


        delay(300);
        
        for(int8_t x = 0; x < state.maxPlayerCount; x++)
        {
            if(!state.isPlayerAlive(x) || (data.playerReacts & (1 << x)))
            {
               continue;
            }

            if(input.isPressedRaw(input.buttonFromIndex(x)))
            {

            }
        }
    }
}