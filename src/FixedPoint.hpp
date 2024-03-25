#pragma once

#include <stdint.h>

//template<uint8_t M = 8, uint8_t N = 8>
class FixedPoint
{
    union
    {
        int16_t raw;

        struct
        {
            uint8_t lower;
            int8_t upper;
        };
    };

    static constexpr uint8_t M = 8;
    static constexpr uint8_t N = 8;
    static constexpr uint8_t K = 1 << (N - 1);

public:
    constexpr FixedPoint(int8_t integer, uint8_t fraction = 0) : lower(fraction), upper(integer)
    {
    }

    constexpr FixedPoint() : raw(0)
    {
    }

    constexpr int16_t getRaw() const
    {
        return raw;
    }

    constexpr int8_t getInteger() const
    {
        return upper;
    }

    constexpr uint8_t getFraction() const
    {
        return lower;
    }

    constexpr void setInteger(int8_t integer)
    {
        upper = integer;
    }

    constexpr void setFraction(uint8_t fraction)
    {
        lower = fraction;
    }

    static constexpr FixedPoint fromRaw(int16_t raw)
    {
        FixedPoint fp;
        fp.raw = raw;
        return fp;
    }

    static constexpr int16_t sat16(int32_t x)
    {
        if (x > 32767) return 32767;
        else if (x < -32768) return -32768;
        else return (int16_t)x;
    }

    constexpr FixedPoint operator-()
    {
        return fromRaw(-raw);
    }

    constexpr FixedPoint& operator+=(FixedPoint b)
    {
        *this = *this + b;
        return *this;
    }

    constexpr FixedPoint& operator-=(FixedPoint b)
    {
        *this = *this - b;
        return *this;
    }

    friend inline FixedPoint operator+(FixedPoint a, FixedPoint b);
    friend inline FixedPoint operator-(FixedPoint a, FixedPoint b);
    friend inline FixedPoint operator*(FixedPoint a, FixedPoint b);
    friend inline FixedPoint operator/(FixedPoint a, FixedPoint b);

    friend inline bool operator==(FixedPoint a, FixedPoint b);
    friend inline bool operator!=(FixedPoint a, FixedPoint b);
    friend inline bool operator<(FixedPoint a, FixedPoint b);
    friend inline bool operator<=(FixedPoint a, FixedPoint b);
    friend inline bool operator>(FixedPoint a, FixedPoint b);
    friend inline bool operator>=(FixedPoint a, FixedPoint b);
};

inline FixedPoint operator+(FixedPoint a, FixedPoint b)
{
    return FixedPoint::fromRaw(a.raw + b.raw);
}

inline FixedPoint operator-(FixedPoint a, FixedPoint b)
{
    return FixedPoint::fromRaw(a.raw - b.raw);
}

inline FixedPoint operator*(FixedPoint a, FixedPoint b)
{
    int32_t temp = (int32_t)a.getRaw() * (int32_t)b.getRaw();
    temp += FixedPoint::K;
    temp >>= FixedPoint::N;
    return FixedPoint::fromRaw(FixedPoint::sat16(temp));
}

inline FixedPoint operator/(FixedPoint a, FixedPoint b)
{
    int32_t temp = (int32_t)a.raw << FixedPoint::N;
    
    if ((temp >= 0 && b.raw >= 0) || (temp < 0 && b.raw < 0))
    {   
        temp += b.raw / 2;
    }
    else
    {
        temp -= b.raw / 2;
    }

    return FixedPoint::fromRaw(temp / b.raw);
}

inline bool operator==(FixedPoint a, FixedPoint b)
{
    return a.raw == b.raw;
}

inline bool operator!=(FixedPoint a, FixedPoint b)
{
    return a.raw != b.raw;
}

inline bool operator<(FixedPoint a, FixedPoint b)
{
    return a.raw < b.raw;
}

inline bool operator<=(FixedPoint a, FixedPoint b)
{
    return a.raw <= b.raw;
}

inline bool operator>(FixedPoint a, FixedPoint b)
{
    return a.raw > b.raw;
}

inline bool operator>=(FixedPoint a, FixedPoint b)
{
    return a.raw >= b.raw;
}