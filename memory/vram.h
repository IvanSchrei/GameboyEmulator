#ifndef VRAM_H
#define VRAM_H

#include <cstdint>
#include <array>

class VRAM{
    public:
        //read & write functions
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t value);
    private:
        std::array<uint8_t, 0x2000> vram = {0x0};
};

#endif