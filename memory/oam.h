#ifndef OAM_H
#define OAM_H

#include <cstdint>
#include <array>

class OAM{
    public:
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t value);
    private:
        std::array<uint8_t, 0xA0> oam = {0x0};
};

#endif