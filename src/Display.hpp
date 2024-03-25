#pragma once

#include <stdint.h>

#include "SimpleI2C.h"
#include "font.hpp"
#include "DisplayBuffer.hpp"
#include "Glyphs.hpp"

class Display
{
public:
    SimpleI2C& SI2C;

    static constexpr uint8_t Width = 128;
    static constexpr uint8_t Height = 64;

    static constexpr uint16_t BufferSize = Width * Height / 8;

    static constexpr uint8_t Address = 0x3C;

    enum Commands : uint8_t
    {
        MEMORYMODE = 0x20,
        COLUMNADDR = 0x21,
        PAGEADDR = 0x22,
        SETCONTRAST = 0x81,
        CHARGEPUMP = 0x8D,
        SEGREMAP = 0xA0,
        DISPLAYALLON_RESUME = 0xA4,
        DISPLAYALLON = 0xA5,
        NORMALDISPLAY = 0xA6,
        INVERTDISPLAY = 0xA7,
        SETMULTIPLEX = 0xA8,
        DISPLAYOFF = 0xAE,
        DISPLAYON = 0xAF,
        COMSCANINC = 0xC0,
        COMSCANDEC = 0xC8,
        SETDISPLAYOFFSET = 0xD3,
        SETDISPLAYCLOCKDIV = 0xD5,
        SETPRECHARGE = 0xD9,
        SETCOMPINS = 0xDA,
        SETVCOMDETECT = 0xDB,
        SETLOWCOLUMN = 0x00,
        SETHIGHCOLUMN = 0x10,
        SETSTARTLINE = 0x40,
        EXTERNALVCC = 0x01,
        SWITCHCAPVCC = 0x02,
        RIGHT_HORIZONTAL_SCROLL = 0x26,
        LEFT_HORIZONTAL_SCROLL = 0x27,
        VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL = 0x29,
        VERTICAL_AND_LEFT_HORIZONTAL_SCROLL = 0x2A,
        DEACTIVATE_SCROLL = 0x2E,
        ACTIVATE_SCROLL = 0x2F,
        SET_VERTICAL_SCROLL_AREA = 0xA3,

        COMMANDS_MODE = 0x00,
        DATA_MODE = 0x40,
    };
    
    enum Screen : uint8_t
    {
        Player1 = (1 << 3),
        Player2 = (1 << 5),
        Player3 = (1 << 4),
        Player4 = (1 << 6),

        Menu    = (1 << 7),
        
        Players = Player1 | Player2 | Player3 | Player4,
        All     = Players | Menu,
    };
    
    Display(SimpleI2C& SI2C) : SI2C(SI2C)
    {

    }

    void init()
    {
        SI2C.startW(Address);

        SI2C.write(Commands::COMMANDS_MODE);

        SI2C.write(Commands::COMMANDS_MODE);
        SI2C.write(Commands::DISPLAYOFF);
        SI2C.write(Commands::MEMORYMODE, 0x00);
        SI2C.write(Commands::PAGEADDR, 0x00, 0x07); // start, end
        SI2C.write(Commands::COLUMNADDR, 0x00, Width - 1); // start, end

        SI2C.write(Commands::COMSCANDEC); // vertical normal
        //SI2C.write(Commands::COMSCANINC); // vertical flip

        SI2C.write(Commands::SEGREMAP | 0x01); // horizontal normal
        //SI2C.write(Commands::SEGREMAP); // horizontal flip

        SI2C.write(Commands::SETSTARTLINE | 0); // horizontal flip

        SI2C.write(Commands::SETDISPLAYCLOCKDIV, 0x80);

        SI2C.write(Commands::SETMULTIPLEX, Height - 1);

        SI2C.write(Commands::SETMULTIPLEX, Height - 1);

        SI2C.write(Commands::SETDISPLAYOFFSET, 0x00);

        SI2C.write(Commands::CHARGEPUMP, 0x14); // 0x10 for external VCC
        SI2C.write(Commands::SETPRECHARGE, 0xF1); // 0x22 for external VCC

        SI2C.write(Commands::SETCOMPINS, 0x12);

        SI2C.write(Commands::SETCONTRAST, 0xFF);

        SI2C.write(Commands::SETVCOMDETECT, 0x40);

        SI2C.write(Commands::DISPLAYALLON_RESUME);
        SI2C.write(Commands::NORMALDISPLAY);
        SI2C.write(Commands::DEACTIVATE_SCROLL);
        SI2C.write(Commands::DISPLAYON);

        //SI2C.stop();
    }

    void selectScreen(Screen screen)
    {
        SI2C.start();
        SI2C.sendAddresWrite(0b01110000);
        SI2C.write(screen);
        SI2C.stop();
    }

    void selectMenu()
    {
        selectScreen(Screen::Menu);
    }

    void selectPlayers(uint8_t players = 0b1111)
    {
        uint8_t screens{};
        
        if(players & (1 << 0))
        {
            screens |= Screen::Player1;
        }
        else if(players & (1 << 1))
        {
            screens |= Screen::Player2;
        }
        else if(players & (1 << 2))
        {
            screens |= Screen::Player3;
        }
        else if(players & (1 << 3))
        {
            screens |= Screen::Player4;
        }

        selectScreen(static_cast<Screen>(screens));
    }

    void selectScreenfromIndex(uint8_t playerIndex)
    {
        switch(playerIndex)
        {
            case 0:
                selectScreen(Screen::Player1);
                break;
            case 1:
                selectScreen(Screen::Player2);
                break;
            case 2:
                selectScreen(Screen::Player3);
                break;
            case 3:
                selectScreen(Screen::Player4);
                break;
        }
    }

    void horizontalScroll(bool left, uint8_t start, uint8_t end, uint8_t speed)
    {
        SI2C.startW(Address);
        SI2C.write(Commands::COMMANDS_MODE);

        SI2C.write(0x26 + left);
        SI2C.write(0x00);
        SI2C.write(start);
        SI2C.write(speed);
        SI2C.write(end);
        SI2C.write(0x00);
        SI2C.write(0xFF);

        SI2C.write(Commands::ACTIVATE_SCROLL);
    }


    void stopScroll()
    {
        SI2C.startW(Address);
        SI2C.write(Commands::COMMANDS_MODE);
        SI2C.write(Commands::DEACTIVATE_SCROLL);
    }
    void clearRect(uint8_t x = 0, uint8_t y = 0, uint8_t width = Width, uint8_t height = Height)
    {
        startDraw(x, y, width, height);

        const auto byteCount = static_cast<uint16_t>(width) * height / 8;

        for(uint16_t x = 0; x < byteCount; x++)
        {
            SI2C.write(0x00);
        }
    }

    void startDraw(uint8_t x = 0, uint8_t y = 0, uint8_t width = Width, uint8_t height = Height)
    {
        SI2C.start();
        SI2C.sendAddresWrite(Address);

        SI2C.write(Commands::COMMANDS_MODE);

        SI2C.write(Display::Commands::PAGEADDR);
        SI2C.write(y / 8);
        SI2C.write(y / 8 + (height - 1) / 8);

        SI2C.write(Display::Commands::COLUMNADDR);
        SI2C.write(x);
        SI2C.write(x + width - 1);  
        
        SI2C.start();
        SI2C.sendAddresWrite(Address);
        SI2C.write(Commands::DATA_MODE);
    }
    
    template <uint8_t Width, uint8_t Height>
    void draw(const DisplayBuffer<Width, Height>& buffer, uint8_t x = 0, uint8_t y = 0)
    {
        startDraw(x, y, Width, ((Height + 7) / 8) * 8);
        SI2C.write(&buffer.data[0], buffer.byteCount);
    }

    void draw(const Glyph& glyph, uint8_t x = 0, uint8_t y = 0)
    {
        const auto g = readPgm(glyph);
        startDraw(x, y, g.width, ((g.height + 7) / 8) * 8);
        
        SI2C.write_P(g.data, g.length);
    }

    void print(char c)
    {
        SI2C.write_P(Font::getChar(c), Font::charWidth);
    }

    void print(const char* str)
    {
        while(*str != 0)
        {
            print(*(str++));
            SI2C.write(0x00);
        }
    }

    void printP(const char* str)
    {
        while(true)
        {
            const char c = pgm_read_byte(str++);
            if(!c)
            {
                break;
            }

            print(c);
            SI2C.write(0x00);
        }
    }

    void print(unsigned long number, uint8_t base = 10)
    {
        char buf[8 * sizeof(number) + 1];
        char *str = &buf[sizeof(buf) - 1];

        *str = '\0';

        do
        {
            char c = number % base;
            number /= base;

            *--str = c < 10 ? c + '0' : c + 'A' - 10;
        } while(number);

        return print(str);
    }

    void print(long number, uint8_t base = 10)
    {
        if (number < 0)
        {
            print('-');
            number = -number;
        }

        print(static_cast<unsigned long>(number), base);
    }

    void print(float number, uint8_t precision = 2)
    {
        if (number < 0.0)
        {
            print('-');
            number = -number;
        }

        double rounding = 0.5;
        for (uint8_t x = 0; x < precision; x++)
            rounding /= 10.0;
        
        number += rounding;

        unsigned long intPart = static_cast<unsigned long>(number);
        double remainder = number - static_cast<double>(intPart);
        print(intPart);

        if (precision > 0)
        {
            print('.'); 
        }

        // Extract digits from the remainder one at a time
        while (precision-- > 0)
        {
            remainder *= 10.0;
            uint8_t toPrint = static_cast<uint8_t>(remainder);
            print(static_cast<char>('0' + toPrint));
            remainder -= toPrint; 
        }
    }
};