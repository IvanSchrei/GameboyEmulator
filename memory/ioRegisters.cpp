#include <iostream>
#include <cstdint>

#include "ioRegisters.h"

uint8_t IORegisters::read(uint16_t address){
    uint16_t actualAddress = address - 0xFF00;
    //here we check if address is special and if so do something cool
    //else we just read from memory like some dumbass

    //actually for read most things are just normal memory reads
    switch(address){
        case 0xFF00:{
            //Joypad, but just read
            return io[actualAddress];
            break;
        }
        case 0xFF04:{
            //DIV (divider) increments over time
            return io[actualAddress];
            break;
        }
        case 0xFF0F:{
            //Interrupts, but just read for now
            return io[actualAddress];
            break;
        }
        default: {
            return io[actualAddress];
            break;
        }
    };
}

void IORegisters::write(uint16_t address, uint8_t value){
    uint8_t actualAddress = address - 0xFF00;

    //here we check if address is special and if so do something cool
    //else we just write into memory like some dumbass

    switch(address){
        case 0xFF00:{
            //Joypad, but just store for now
            io[actualAddress] = value;
            break;
        }
        case 0xFF04:{
            //DIV (divider)
            //if we write any Value set to 0
            io[actualAddress] = 0;
            break;
        }
        case 0xFF05:{
            //Timer
            io[actualAddress] = value;
            break;
        }
        case 0xFF06:{
            //Timer
            io[actualAddress] = value;
            break;
        }
        case 0xFF07:{
            //Timer
            io[actualAddress] = value;
            break;
        }
        case 0xFF0F:{
            //Interrupts
            io[actualAddress] = value;
            break;
        }
        default:{
            io[actualAddress] = value;
            break;
        }
    }
}