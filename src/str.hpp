#pragma once

#include <stdint.h>
#include "avr/pgmspace.h"

constexpr int strlen_constexpr(const char* src)
{
    int len = 0;
    while(src[len] != 0)
    {
        len++;
    }

    return len;
}

template<size_t N>
struct StringLiteral
{
    constexpr StringLiteral(const char (&str)[N])
    {
        for(unsigned int x = 0; x < N; x++)
        {
            value[x] = str[x];
        }
        
        value[N] = 0;
    }
    
    char value[N + 1] = {};
};

template<StringLiteral Str>
constexpr static const char* get()
{
    PROGMEM static constexpr StringLiteral copy = Str;
    return copy.value;
}

template<StringLiteral Str>
constexpr auto operator""_PSTR()
{
    PROGMEM static constexpr StringLiteral copy = Str;
    return copy.value;
}