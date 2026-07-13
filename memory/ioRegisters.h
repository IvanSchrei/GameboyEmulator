#ifndef IOREGISTERS_H
#define IOREGISTERS_H

#include <cstdint>
#include <array>

class IORegisters{
    public:
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t value);
    private:
        std::array<uint8_t, 0x80> io = {0x0};
        uint8_t upperJoypadBits;
        uint8_t lowerJoypadBits;
};

#endif