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
    inline auto init()
    {
        unsigned long seed = 0;
        for(uint8_t x = 0; x < sizeof(seed) * 8 / 2; x++)
        {
            delay(2);
            seed |= analogRead(A6) & 0x03;
            seed <<= 2;
        }

        randomSeed(seed);
    
        return seed;
    }

    inline bool binary()
    {
        return random(0, 2);
    }
}

template <typename T, typename Size = uint8_t>
struct Array
{
    T* data;
    Size size;
};

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

template<typename T>
inline void swap(T& v1, T& v2)
{
    T temp = v1;
    v1 = v2;
    v2 = temp;
}

inline void reverseArray(uint8_t* values, uint8_t count)
{
    for (uint8_t x = 0; x < count / 2; x++)
    {
        swap(values[x], values[count - x - 1]);
    }
}

inline void shuffle(uint8_t* values, uint8_t count)
{    
    for (uint8_t x = count - 1; x > 0; x--) 
    { 
        const auto y = random(x + 1);
        swap(values[x], values[y]); 
    } 
}

template<class T> using Invoke = typename T::type;

template<unsigned...> struct seq{ using type = seq; };

template<class S1, class S2> struct concat;

template<unsigned... I1, unsigned... I2>
struct concat<seq<I1...>, seq<I2...>>
  : seq<I1..., (sizeof...(I1)+I2)...>{};

template<class S1, class S2>
using Concat = Invoke<concat<S1, S2>>;

template<unsigned N> struct gen_seq;
template<unsigned N> using GenSeq = Invoke<gen_seq<N>>;

template<unsigned N>
struct gen_seq : Concat<GenSeq<N/2>, GenSeq<N - N/2>>{};

template<> struct gen_seq<0> : seq<>{};
template<> struct gen_seq<1> : seq<0>{};