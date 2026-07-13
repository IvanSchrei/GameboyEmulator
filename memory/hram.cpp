#include <cstdint>

#include "hram.h"

//we get a uint16_t address but hram is only 127 bytes so we need to cut it down to only access these

uint8_t HRAM::read(uint16_t address){
    address = address - 0xFF80;
    return hram[address];
}

void HRAM::write(uint16_t address, uint8_t value){
    address = address - 0xFF80;
    hram[address] = value;
}