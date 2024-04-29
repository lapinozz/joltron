#pragma once

#include "utils.hpp"
#include "input.hpp"
#include "Display.hpp"
#include "Settings.hpp"
#include "LedController.hpp"

#include "Games.hpp"

enum Menus : uint8_t
{
    Main,
    SelectGame,
    DifficultySelect,
    Settings,
    Paused,

    Count
};

enum MenuAction : uint8_t
{
    None,
    Play,
    Resume,
    ExitToMenu,
    Traitor,
};

enum EntryType : uint8_t
{
    Menu,
    Action,
    GameSelect,
    Difficulty,
    Setting
};

class MenuEntry
{
public:
    using Param = uint16_t;

private:
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
    constexpr MenuEntry(const char* text, EntryType type, Param param) : type(type), text(text), data{.param=param}
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

    Param getParam() const
    { 
        return readPgm(&data.param);
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

PROGMEM static constexpr MenuEntry MenuEntries_DifficultySelect[]
{
    {"Easy"_PSTR, EntryType::Difficulty, static_cast<MenuEntry::Param>(GameState::Difficulty::Easy)},
    {"Normal"_PSTR, EntryType::Difficulty, static_cast<MenuEntry::Param>(GameState::Difficulty::Normal)},
    {"Hard"_PSTR, EntryType::Difficulty, static_cast<MenuEntry::Param>(GameState::Difficulty::Hard)},
};

PROGMEM static constexpr MenuEntry MenuEntries_Paused[]
{
    {"Resume"_PSTR, MenuAction::Resume},
    {"Exit To Menu"_PSTR, MenuAction::ExitToMenu},
    {"TRAITOR"_PSTR, MenuAction::Traitor},
};

template<uint8_t size>
using MenuEntries = MenuEntry[size];

constexpr const auto& buildGameSelectMenu()
{    
    return []<auto... Xs>(seq<Xs...>) -> const auto&
    {
        PROGMEM static constexpr MenuEntry entries[GameCount + 1] =
        {
            {"Back"_PSTR, Menus::Main},
            MenuEntry{gameDefinitions[Xs].title, EntryType::GameSelect, Xs}...
        };

        return entries;
    }
    (gen_seq<GameCount>{});
}

constexpr const auto& MenuEntries_SelectGame = buildGameSelectMenu();

constexpr const auto& buildSettingsMenu()
{    
    return []<auto... Xs>(seq<Xs...>) -> const auto&
    {
        PROGMEM static constexpr MenuEntry entries[SettingCount + 1] =
        {
            {"Back"_PSTR, Menus::Main},
            MenuEntry{settingDefinitions[Xs].name, EntryType::Setting, Xs}...
        };

        return entries;
    }
    (gen_seq<SettingCount>{});
}

constexpr const auto& MenuEntries_Settings = buildSettingsMenu();

#define MENU_LIST_ENTRY(menu) {MenuEntries_ ## menu, sizeof(MenuEntries_  ## menu) / sizeof(MenuEntries_ ## menu[0])}

PROGMEM static constexpr MenuDefinition menuDefinitions[static_cast<uint8_t>(Menus::Count)]
{
    MENU_LIST_ENTRY(Main),
    MENU_LIST_ENTRY(SelectGame),
    MENU_LIST_ENTRY(DifficultySelect),
    MENU_LIST_ENTRY(Settings),
    MENU_LIST_ENTRY(Paused),
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
        display.clearRect();

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

    const MenuEntry* onSelect()
    {
        const auto& entry = definition.entries[currentIndex];
        if(entry.getType() == EntryType::Menu)
        {
            setMenu(entry.getTarget());
        }
        else
        {
            return &entry;
        }

        return nullptr;
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
    
    const MenuEntry* update(Display& display, Input& input, LedController& ledController)
    {
        updateLeds(input, ledController);

        if(needsRedraw)
        {
            redraw(display);
        }

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
            return onSelect();
        }

        return nullptr;
    }
};