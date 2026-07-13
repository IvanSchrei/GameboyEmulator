#ifndef MEMORYBUS_H
#define MEMORYBUS_H

#include <cstdint>
#include "cartridge.h"
#include "hram.h"
#include "wram.h"
#include "ioRegisters.h"
#include "vram.h"
#include "oam.h"

class MemoryBus{
    public:
        MemoryBus();
        ~MemoryBus();

        uint8_t read(uint16_t adress);
        void write(uint16_t adress, uint8_t value);
    private:
        WRAM wram_;
        Cartridge cartridge_;
        HRAM hram_;
        uint8_t interruptEnable = 0;
        IORegisters ioRegisters_;
        VRAM vram_;
        OAM oam_;
};

#endif