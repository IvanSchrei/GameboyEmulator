#ifndef HRAM_H
#define HRAM_H

#include <cstdint>
#include <array>

class HRAM{
    public:
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t value);
    private:
        std::array<uint8_t, 0x7F> hram = {0x00};
};

#endif