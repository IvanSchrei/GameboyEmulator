#include <cstdint>

#include "vram.h"

uint8_t VRAM::read(uint16_t address){
    address = address - 0x8000;
    return vram[address];
}

void VRAM::write(uint16_t address, uint8_t value){
    address = address - 0x8000;
    vram[address] = value;
}