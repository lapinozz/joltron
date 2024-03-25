#pragma once

#include "utils.hpp"
#include "input.hpp"
#include "Display.hpp"
#include "LedController.hpp"

enum Menus : uint8_t
{
    Main,
    SelectGame,
    Settings,
};

enum MenuAction : uint8_t
{
    None,
    Play,

};

enum EntryType : uint8_t
{
    Menu,
    Action,
};

class MenuEntry
{
    using Param = uint16_t;

    EntryType type;
    
    const char* text;

    union Data
    {
        Menus target;
        MenuAction action;
        Param param;
        void* paramPtr;
    };

    Data data;

public:
    constexpr MenuEntry(const char* text, Menus target) : type(EntryType::Menu), text(text), data{.target=target}
    {}
    constexpr MenuEntry(const char* text, MenuAction action) : type(EntryType::Action), text(text), data{.action=action}
    {}

/*
    constexpr MenuEntry(const char* text, Param param) : type(EntryType::Menu), data{.param=param}
    {}

    constexpr MenuEntry(const char* text, void* paramPtr) : type(EntryType::Menu), data{.paramPtr=paramPtr}
    {}
*/

    EntryType getType() const
    { 
        return readPgm(&type);
    }

    Menus getTarget() const
    { 
        return readPgm(&data.target);
    }

    MenuAction getAction() const
    { 
        return readPgm(&data.action);
    }

    const char* getText() const
    { 
        return readPgm(&text);
    }
};
struct MenuDefinition
{
    const MenuEntry* entries;
    uint8_t entryCount;
};

PROGMEM static constexpr MenuEntry MenuEntries_Main[] = {
    {"Play"_PSTR, MenuAction::Play},
    {"Select Game"_PSTR, Menus::SelectGame},
    {"Settings"_PSTR, Menus::Settings},
};

PROGMEM static constexpr MenuEntry MenuEntries_Sub[]
{
    {"Main2"_PSTR, Menus::Main},
    {"Sub2"_PSTR, Menus::Main},
};

#define MENU_LIST_ENTRY(menu) {MenuEntries_ ## menu, sizeof(MenuEntries_  ## menu) / sizeof(MenuEntries_ ## menu[0])}

PROGMEM static constexpr MenuDefinition menuDefinitions[]
{
    MENU_LIST_ENTRY(Main),
    //MENU_LIST_ENTRY(Sub),
};

MenuDefinition getMenu(Menus menu)
{
    return readPgm<MenuDefinition>(menuDefinitions[menu]);
}

class MenuDisplay
{
public:
    bool needsRedraw = false;
    MenuDefinition definition = {};

    uint8_t currentIndex = 0;

    static constexpr uint8_t indicatorZoneWidth = Font::charWidth + 3;
    static constexpr uint8_t textZoneWidth = Display::Width - indicatorZoneWidth;

    static constexpr auto halfPressColor = LedController::fromRGB(0, 255 / 2, 0);

    void redraw(Display& display)
    {
        needsRedraw = false;

        display.selectMenu();
        display.clearRect(indicatorZoneWidth, 0, textZoneWidth);

        for(int x = 0; x < definition.entryCount; x++)
        {
            display.startDraw(0, x * Font::charHeight);

            display.print(x == currentIndex ? '>' : ' ');

            for(int y = Font::charWidth; y < indicatorZoneWidth; y++)
            {
                display.SI2C.write(0x00);
            }

            display.printP(definition.entries[x].getText());
        }
    }

    void moveUp()
    {
        if(currentIndex <= 0)
        {
            return;
        }

        currentIndex--;
        needsRedraw = true;
    }

    void moveDown()
    {
        if(currentIndex + 1 >= definition.entryCount)
        {
            return;
        }
        
        currentIndex++;
        needsRedraw = true;
    }

    MenuAction onSelect()
    {
        const auto& entry = definition.entries[currentIndex];
        if(entry.getType() == EntryType::Menu)
        {
            setMenu(entry.getTarget());
        }
        else if(entry.getType() == EntryType::Action)
        {
            return entry.getAction();
        }

        return {};
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
    void setMenu(Menus menu)
    {
        currentIndex = 0;
        needsRedraw = true;
        definition = getMenu(menu);
    }
    
    MenuAction update(Display& display, Input& input, LedController& ledController)
    {
        MenuAction action = MenuAction::None;

        if(input.isNewPressed(Input::Button::MenuUp))
        {
            moveUp();
        }
        else if(input.isNewPressed(Input::Button::MenuDown))
        {
            moveDown();
        }
        else if(input.isNewPressed(Input::Button::MenuSelect))
        {
            action = onSelect();
        }

        updateLeds(input, ledController);

        if(needsRedraw)
        {
            redraw(display);
        }

        return action;
    }
};