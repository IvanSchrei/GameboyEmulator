#include <cstdint>

#include "oam.h"

uint8_t OAM::read(uint16_t address){
    address = address - 0xFE00;
    return oam[address];
}

void OAM::write(uint16_t address, uint8_t value){
    address = address - 0xFE00;
    oam[address] = value;
}