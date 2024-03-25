#pragma once

struct GameState;
struct Display;
struct Input;
struct LedController;

struct GameDefinition
{
    using Func = void (*)(GameState&, Display&, Input&, LedController&);

    const char* title = nullptr;
    Func update = nullptr;
};