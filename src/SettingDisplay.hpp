#pragma once

#include "utils.hpp"
#include "input.hpp"
#include "Display.hpp"
#include "Settings.hpp"
#include "LedController.hpp"

#include "settings.hpp"

class SettingDisplay
{
public:
    bool needsRedraw = false;
    SettingDefinition definition = {};

    static constexpr auto halfPressColor = LedController::fromRGB(0, 255 / 2, 0);

    void redraw(Display& display)
    {
        needsRedraw = false;

        display.selectMenu();
        display.clearRect();

        display.printP(definition.name);

        const auto type = definition.type;
        if(type == SettingDefinition::Type::Number)
        {
            display.startDraw(10, Font::charHeight, 100, Font::charHeight);
            display.SI2C.write(0b11111111);

            const auto& number = definition.number;
            auto& value= *number.value;

            const uint8_t width = 80;
            const auto filledWidth = (value - number.min) * width / number.max;
            for(uint8_t x = 0; x < width; x++)
            {
                const bool isFilled = x < filledWidth;
                display.SI2C.write(isFilled ? 0b00111100 : 0);
            }
            display.SI2C.write(0b11111111);
        }
    }

    void onUp()
    {
        needsRedraw = true;

        const auto type = definition.type;
        if(type == SettingDefinition::Type::Number)
        {
            const auto& number = definition.number;
            auto& value= *number.value;
            value += number.increment; 
            if(value > number.max)
            {
                value = number.max;
            }
        }
    }

    void onDown()
    {
        needsRedraw = true;

        const auto type = definition.type;
        if(type == SettingDefinition::Type::Number)
        {
            const auto& number = definition.number;
            auto& value= *number.value;
            value -= number.increment; 
            if(value < number.min)
            {
                value = number.min;
            }
        }
    }

    void onValueChange()
    {
        
    }

    void updateLeds(Input& input, LedController& ledController)
    {
        const auto getColorForButton = [input](Input::Button button)
        {
            return halfPressColor * input.isPressed(button) + halfPressColor * input.isLongPressed(button);
        };

        const auto upColor = getColorForButton(Input::Button::MenuUp);
        ledController.set(6, upColor);
        ledController.set(7, upColor);

        const auto downColor = getColorForButton(Input::Button::MenuDown);
        ledController.set(1, downColor);
        ledController.set(2, downColor);

        const auto selectColor = getColorForButton(Input::Button::MenuSelect);
        ledController.set(4, selectColor);
        ledController.set(9, selectColor);
    }

public:
    void setSetting(SettingId id)
    {
        needsRedraw = true;
        definition = readPgm(settingDefinitions[static_cast<uint8_t>(id)]);
    }
    
    bool update(Display& display, Input& input, LedController& ledController)
    {
        updateLeds(input, ledController);

        if(needsRedraw)
        {
            redraw(display);
        }

        if(input.isNewPressed(Input::Button::MenuUp))
        {
            onUp();
        }
        else if(input.isNewPressed(Input::Button::MenuDown))
        {
            onDown();
        }
        else if(input.isNewPressed(Input::Button::MenuSelect))
        {
            return true;
        }

        return false;
    }
};