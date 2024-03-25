#pragma once

#include <stdint.h>

template<uint8_t Width = 0, uint8_t Height = 0>
class DisplayBuffer
{
public:
    
    // Width * ((Height + 7) / 8);
    static constexpr uint16_t byteCount = Width * Height / 8;
    uint8_t data[byteCount] = {};

    static constexpr void make()
    {
        
    }

    void setPixel(uint8_t x, uint8_t y, bool set)
    {
        if(set)
        {
            data[y / 8 * Width + x] |= (1 << (y % 8));
        }
        else
        {
            data[y / 8 * Width + x] &= ~(1 << (y % 8));
        }
    }
};