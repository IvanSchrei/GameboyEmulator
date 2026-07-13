#include <iostream>
#include <cstdint>
#include <cstring>

#include "wram.h"

//need to adjust address here otherwise we just write outside the internal memory array

uint8_t WRAM::read(uint16_t address){
    address = address - 0xC000;
    return wram[address];
}

void WRAM::write(uint16_t address, uint8_t value){
    address = address - 0xC000;
    wram[address] = value;
}

void WRAM::clear(){
    std::memset(wram.data(), 0, wram.size());
}