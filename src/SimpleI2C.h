#pragma once

#include <avr/io.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

class SimpleI2C
{
public:
    enum Status : uint8_t
    {
        START = 0x08,
        REPEATED_START = 0x08,
        MT_SLA_ACK = 0x18,
        MT_DATA_ACK = 0x28,

        MASK = 0xF8,
    };

    void setPullup(bool active) const
    {
        if(active)
        {
            sbi(PORTC, 1);
            sbi(PORTC, 5);
        }
        else
        {
            cbi(PORTC, 1);
            cbi(PORTC, 5);
        }
    }

    void init(bool fast = false) const
    {  
        setPullup(1);

        // initialize twi prescaler and bit rate
        cbi(TWSR, TWPS0);
        cbi(TWSR, TWPS1);

        if(fast)
        {
            TWBR = ((F_CPU / 400000) - 16) / 2;
        }
        else
        {
            TWBR = ((F_CPU / 100000) - 16) / 2;
        }

        // enable twi module and acks
        TWCR = _BV(TWEN) | _BV(TWEA);
    }

    void onError() const
    {

    }

    void start() const
    {
        // Send START condition
        TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

        // Wait for TWINT flag set. This indicates that the START condition has been transmitted
        while (!(TWCR & (1<<TWINT)));

        // Check value of TWI status register. Mask prescaler bits. If status different from START go to ERROR
        if ((TWSR & Status::MASK) != Status::START && (TWSR & Status::MASK) != Status::REPEATED_START)
        {
            onError();
        }
    }

    void startW(uint8_t address) const
    {
        start();
        sendAddresWrite(address);
    }

    void startR(uint8_t address) const
    {
        start();
        sendAddresRead(address);
    }

    void write(uint8_t data) const
    {
        TWDR = data;
        TWCR = (1<<TWINT) | (1<<TWEN);
        
        while (!(TWCR & (1<<TWINT)));
    }

    void write(uint8_t data1, uint8_t data2) const
    {
        write(data1);
        write(data2);
    }

    void write(uint8_t data1, uint8_t data2, uint8_t data3) const
    {
        write(data1);
        write(data2);
        write(data3);
    }

    void write(uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4) const
    {
        write(data1);
        write(data2);
        write(data3);
        write(data4);
    }

    void write(const uint8_t* data, uint16_t count) const
    {
        while(count--)
        {
            TWDR = *(data++);
            TWCR = (1<<TWINT) | (1<<TWEN);
            
            while (!(TWCR & (1<<TWINT)));
        }
    }

    void write_P(const uint8_t* data, uint16_t count) const
    {
        while(count--)
        {
            TWDR = pgm_read_byte(data++);
            TWCR = (1<<TWINT) | (1<<TWEN);
            
            while (!(TWCR & (1<<TWINT)));
        }
    }

    void sendAddresWrite(uint8_t address) const
    {
        write(address << 1);
    }

    void sendAddresRead(uint8_t address) const
    {
        write(address << 1 | 1);
    }

    void stop() const
    {
        TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
        while ((TWCR & (1 << TWSTO)));
    }

    void send(uint8_t address, uint8_t registerAddress, uint8_t* buffer, uint16_t bufferSize) const
    {
        start();

        sendAddresWrite(address);

        write(registerAddress);

        write(buffer, bufferSize);

        stop();
    }
};