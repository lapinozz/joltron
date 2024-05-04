#pragma once

#include <stdint.h>

struct Settings
{
    int16_t volume = 20;
};

inline Settings settings{}; 

enum class SettingId : uint8_t
{
    Volume,

    Count
};

constexpr uint8_t SettingCount = static_cast<uint8_t>(SettingId::Count);

struct SettingDefinition
{
public:

    enum class Type : uint8_t
    {
        Number
    };

    struct Number
    {
        int16_t* value;
        int16_t min;
        int16_t max;
        int16_t increment;
    };

    const char* name;

    Type type;
    union
    {
      Number number;
    };    
};


PROGMEM constexpr SettingDefinition settingDefinitions[] =
{
    {"Volume"_PSTR, SettingDefinition::Type::Number, {&settings.volume, 0, 30, 5}}
};