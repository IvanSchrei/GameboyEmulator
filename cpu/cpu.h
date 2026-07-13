#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <array>

#include "../memory/memory_bus.h"

class CPU{
    public:
        //add other components like Display and Input here
        CPU(MemoryBus& memory);
        ~CPU();
        uint8_t& F();
        void initialize();
        void cycle();
    private:
        MemoryBus& memory_;

        //Registers. these can be accessed as one 16-bit register or 2 8-bit register.
        //[0] = A
        //[1] = Flags, lower 4 bits are always 0 upper are the flags Z N H C
        //documentation flag symbols:
        // 0 = set to 0
        // 1 = set to 1
        // - = leave untouched
        // Z N H C = recalculate
        //Z = Zero
        //N = Subtracted last cycle
        //H = Halfcarry
        //C = Carry
        //[2] = B
        //[3] = C
        //[4] = D
        //[5] = E
        //[6] = H
        //[7] = L
        //writing to 16 bit register, like BC:
        //BC = B(B << 8) | C
        //just use Helpers for that
        std::array<uint8_t, 8> Registers_;

        //stack pointer
        uint16_t SP_;

        //Programm Counter
        uint16_t PC_;

        uint16_t getAF();
        void setAF(uint16_t value);

        uint16_t getBC();
        void setBC(uint16_t value);

        uint16_t getDE();
        void setDE(uint16_t value);

        uint16_t getHL();
        void setHL(uint16_t value);
};

#endif