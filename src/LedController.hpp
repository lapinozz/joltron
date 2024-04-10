#pragma once

#include <FastLED.h>

#define LED_TYPE WS2812B

struct LedController
{
    using Color = uint32_t;

    static constexpr uint8_t LedPin = 2;
    static constexpr uint8_t LedCount = 10;

    CRGB leds[LedCount] = {};

    static constexpr uint8_t showDelay = 12;
    uint32_t lastShowTime = 0;

    constexpr static uint32_t fromRGB(uint8_t R, uint8_t G, uint8_t B)
    {
        return (static_cast<uint32_t>(R) << 0) | (static_cast<uint32_t>(G) << 8) | (static_cast<uint32_t>(B) << 16);
    }

    void init()
    {
        FastLED.addLeds<LED_TYPE, LedPin, GRB>(leds, LedCount).setCorrection(UncorrectedColor).setTemperature(UncorrectedTemperature);
        FastLED.setBrightness(20);

        display();
    }

    void setAsByte(uint8_t val)
    {
        leds[0].r = 255;
        leds[0].g = 255;
        leds[0].b = 255;

        for(int x = 0; x < 8; x++)
        {
            leds[1+x].r = (val & (1 << x)) ? 255 : 0;
        }

        display();
    }

    void set(uint8_t index, CRGB value)
    {
        leds[index] = value;
    }

    void clear()
    {
        for(uint8_t x = 0; x < LedCount; x++)
        {
            leds[x] = {};
        }
    }

    void display()
    {
        const auto now = millis();
        if(now - lastShowTime < showDelay)
        {
            return;
        }

        lastShowTime = now;
        FastLED.show();
    }
};