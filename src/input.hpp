#pragma once 

#include <avr/io.h>
#include <Arduino.h>

//const int outPins[] = {11,9,7,5};
//const int inPins[] = {12,10,8,6};

//const int frontButtons[] = {15, 16, 17};

/*
void initHandles()
{
  for(int x = 0; x < 4; x++)
  {
    pinMode(outPins[x], OUTPUT);
  }

  for(int x = 0; x < 4; x++)
  {
    pinMode(inPins[x], INPUT);
  }
}

void zapHandle(int index, int length = 20)
{
  digitalWrite(outPins[index], HIGH);
  delay(20);
  digitalWrite(outPins[index], LOW);
}

bool isHandlePressed(int index)
{
  return digitalRead(inPins[index]);
}
*/

struct Input
{
    static constexpr uint8_t menuButtonsMask = 0b1110;

    static constexpr uint8_t handleButtonMask_B = 0b10101;
    static constexpr uint8_t handleButtonMask_D = (1 << 6);

    static constexpr uint8_t handleOutputMask_B = (1 << 3) | (1 << 1);
    static constexpr uint8_t handleOutputMask_D = (1 << 7) | (1 << 5);

    static constexpr uint8_t handleCount = 4;
    static constexpr uint8_t manuButtonCount = 3;
    
    static constexpr uint8_t inputCount = handleCount + manuButtonCount;

    static constexpr uint32_t debounceDelay = 5;
    static constexpr uint32_t longPressDelay = 1000;

    uint8_t currentInputs{};
    uint8_t currentInputsPress{};
    uint8_t currentInputsLongPress{};

    uint8_t lastInputsPress{};
    uint8_t lastInputsLongPress{};

    uint32_t debounceTimers[inputCount] = {};
    uint32_t longPressTimers[inputCount] = {};

    enum Button : uint8_t
    {
        Handle0     = 1 << 0,
        Handle1     = 1 << 1,
        Handle2     = 1 << 2,
        Handle3     = 1 << 3,

        MenuUp      = 1 << 4,
        MenuSelect  = 1 << 5,
        MenuDown    = 1 << 6,

        Handles = Handle0 | Handle1 | Handle2 | Handle3,
        MenuButtons = MenuUp | MenuSelect | MenuDown,
    };

    void init()
    {
        DDRC &= ~menuButtonsMask;
        PORTC |= menuButtonsMask;

        DDRB &= ~handleButtonMask_B;
        PORTB &= ~handleButtonMask_B;

        DDRD &= ~handleButtonMask_D;
        PORTD &= ~handleButtonMask_D;

        PORTB &= ~handleOutputMask_B;
        DDRB |= handleOutputMask_B;

        PORTD &= ~handleOutputMask_D;
        DDRD |= handleOutputMask_D;

        currentInputs = read();
    }

    void zap(uint8_t handleIndex, uint8_t duration = 20)
    {
        static constexpr uint8_t masks[] =
        {
            1 << 7,
            1 << 1,
            1 << 3,
            1 << 5
        };

        auto& port = handleIndex == 1 || handleIndex == 2 ? PORTB : PORTD;

        port |= masks[handleIndex];
        delay(duration);
        port &= ~masks[handleIndex];
    }

    uint8_t read() const
    {
        const uint8_t portB = PINB;
        const uint8_t portC = PINC;
        const uint8_t portD = PIND;

        const uint8_t menuButtons = ((~portC) & menuButtonsMask);

        const uint8_t handleData = (portD & (1 << 6)) >> 3 | (portB & (1 << 4)) >> 2 | (portB & (1 << 2)) >> 1 | (portB & 1);

        const uint8_t inputs = menuButtons << 3 | handleData;

        return inputs;
    }

    void updatePresses()
    {
        const auto now = millis();

        lastInputsPress = currentInputsPress;
        lastInputsLongPress = currentInputsLongPress;

        for(uint8_t x = 0; x < inputCount; x++)
        {
            if(debounceTimers[x] && now - debounceTimers[x] > debounceDelay)
            {
                debounceTimers[x] = 0;

                const auto value = currentInputs & (1 << x);
                if(value)
                {
                    longPressTimers[x] = now;
                    currentInputsPress |= value;
                }
                else
                {
                    longPressTimers[x] = 0;
                    currentInputsPress &= ~(1 << x);
                    currentInputsLongPress &= ~(1 << x);
                }
            }
        }

        for(uint8_t x = 0; x < inputCount; x++)
        {
            if(longPressTimers[x] && now - longPressTimers[x] > longPressDelay)
            {
                longPressTimers[x] = 0;

                const auto value = currentInputs & (1 << x);
                if(value)
                {
                    currentInputsLongPress |= currentInputsPress & (1 << x);
                }
            }
        }
    }

    void update()
    {
        updatePresses();

        const uint8_t newInputs = read();
        if(newInputs == currentInputs)
        {
            return;
        }

        const auto now = millis();

        for(uint8_t x = 0; x < inputCount; x++)
        {
            const auto lastValue = currentInputs & (1 << x);
            const auto newValue = newInputs & (1 << x);

            if(lastValue == newValue)
            {
                continue;
            }

            debounceTimers[x] = now;
        }

        currentInputs = newInputs;
    }

    Button buttonFromIndex(uint8_t index) const
    {
        return static_cast<Button>(1 << index);
    }

    bool isPressedRaw(Button button) const
    {
        return currentInputs & button;
    }

    bool isPressed(Button button) const
    {
        return currentInputsPress & button;
    }

    bool isLongPressed(Button button) const
    {
        return currentInputsLongPress & button;
    }

    bool isNewPressed(Button button) const
    {
        return ~lastInputsPress & currentInputsPress & button;
    }

    bool isNewLongPressed(Button button) const
    {
        return ~lastInputsLongPress & currentInputsLongPress & button;
    }

    bool isNewRelease(Button button) const
    {
        return lastInputsPress & ~currentInputsPress & button;
    }
};