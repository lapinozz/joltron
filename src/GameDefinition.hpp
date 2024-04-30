#pragma once

struct GameState;
struct Display;
struct Input;
struct LedController;
struct SoundController;

struct GameDefinition
{
    using Func = void (*)(GameState&, Display&, Input&, LedController&, SoundController&);

    const char* title = nullptr;
    Func update = nullptr;
};