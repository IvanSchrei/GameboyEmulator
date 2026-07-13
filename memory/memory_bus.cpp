#include <cstdint>
#include <stdexcept>
#include <iostream>

#include "memory_bus.h"

#include "cartridge.h"
#include "hram.h"
#include "wram.h"
#include "ioRegisters.h"

//route calls to certain addresses forward to the respective class using the memory map


// make this injectable later or you're gonna hate yourself
MemoryBus::MemoryBus() : cartridge_("./Cartridges/tetris.gb"){}

MemoryBus::~MemoryBus(){}

//Memory Map:

//0000	3FFF	16 KiB ROM bank 00	From cartridge, usually a fixed bank
//4000	7FFF	16 KiB ROM Bank 01–NN	From cartridge, switchable bank via mapper (if any)
//8000	9FFF	8 KiB Video RAM (VRAM)	In CGB mode, switchable bank 0/1
//A000	BFFF	8 KiB External RAM	From cartridge, switchable bank if any
//C000	CFFF	4 KiB Work RAM (WRAM)	
//D000	DFFF	4 KiB Work RAM (WRAM)	In CGB mode, switchable bank 1–7
//E000	FDFF	Echo RAM (mirror of C000–DDFF)	Nintendo says use of this area is prohibited.
//FE00	FE9F	Object attribute memory (OAM)	
//FEA0	FEFF	Not Usable	Nintendo says use of this area is prohibited.
//FF00	FF7F	I/O Registers	
//FF80	FFFE	High RAM (HRAM)	
//FFFF	FFFF	Interrupt Enable register (IE)

uint8_t MemoryBus::read(uint16_t address){
    if(address <= 0x7FFF){
        //ROM from Cartridge
        return cartridge_.read(address);
    }
    else if(address <= 0x9FFF){
        // VRAM vram.cpp / .h
        return vram_.read(address);
    }
    else if(address <= 0xBFFF){
        //external RAM from Cartridge
        return cartridge_.read(address);
    }
    else if(address <= 0xDFFF){
        //work RAM
        return wram_.read(address);
    }
    else if(address <= 0xFDFF){
        // ⚠️ echo RAM works BUT:
        // real hardware quirks exist (timing weirdness)
        return wram_.read(address - 0x2000);
    }
    else if(address <= 0xFE9F){
        return oam_.read(address);
    }
    else if(address <= 0xFEFF){
        // ⚠️ "not usable" but still returns open bus behavior IRL
        return 0xFF;
    }
    else if(address <= 0xFF7F){
        return ioRegisters_.read(address);
    }
    else if(address <= 0xFFFE){
        //HRAM
        return hram_.read(address);
    }
    else if(address == 0xFFFF){
        //interrupt enable register
        return interruptEnable;
    }
    else {
        return 0xFF;
    }
}

void MemoryBus::write(uint16_t address, uint8_t value){
    if(address <= 0x7FFF){
        cartridge_.write(address, value);
    }
    else if(address <= 0x9FFF){
        vram_.write(address, value);
    }
    else if(address <= 0xBFFF){
        cartridge_.write(address, value);
    }
    else if(address <= 0xDFFF){
        wram_.write(address, value);
    }
    else if(address <= 0xFDFF){
        wram_.write(address - 0xE000, value);
    }
    else if(address <= 0xFE9F){
        oam_.write(address, value);
    }
    else if(address <= 0xFEFF){
        // ignore
    }
    else if(address <= 0xFF7F){
        ioRegisters_.write(address, value);
    }
    else if(address <= 0xFFFE){
        //HRAM
        hram_.write(address, value);
    }
    else if(address == 0xFFFF){
        //Interruot Enable Register
        interruptEnable = value;
    }
}
