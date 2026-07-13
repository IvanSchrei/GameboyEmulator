#ifndef WRAM_H
#define WRAM_H

#include <cstdint>
#include <array>

class WRAM{
    public:
        uint8_t read(uint16_t address); //read a Byte from the specified address
        void write(uint16_t address, uint8_t value);    //write a Byte into the specified address
        void clear();
    private:
        std::array<uint8_t, 0x2000> wram = {0x0};
};

#endif