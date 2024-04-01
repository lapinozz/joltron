#pragma once

#include "avr/pgmspace.h"
#include "Arduino.h"

template<typename T>
T readPgm(const T* ptr)
{
    uint8_t buffer[sizeof(T)];
    memcpy_P(buffer, ptr, sizeof(T));
    return *reinterpret_cast<T*>(buffer);
}

template<typename T>
T readPgm(const T& ptr)
{
    return readPgm(&ptr);
}

namespace Random
{
    inline void init()
    {
        unsigned long seed = 0;
        for(uint8_t x = 0; x < sizeof(seed) * 8 / 2; x++)
        {
            delay(2);
            seed |= analogRead(A6) & 0x03;
            seed <<= 2;
        }

        randomSeed(seed);
    }

    inline bool binary()
    {
        return random(0, 2);
    }
}

inline void sort(uint8_t* keys, uint8_t* values, uint8_t count)
{
    for (uint8_t x = 1; x < count; x++)
    {
        const auto key = keys[x];
        auto y = x - 1;
 
        while (y >= 0 && values[keys[y]] > values[key])
        {
            keys[y + 1] = keys[y];
            y = y - 1;
        }

        keys[y + 1] = key;
    }
}