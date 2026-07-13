#include "helpers.h"

namespace Flags
{
    void set(uint8_t& f, uint8_t flag){
        f |= flag;
    }

    void clear(uint8_t& f, uint8_t flag){
        f &= ~flag;
    }

    void assign(uint8_t& f, uint8_t flag, bool value){
        if(value){
            set(f, flag);
        }
        else{
            clear(f, flag);
        }
    }

    bool test(uint8_t f, uint8_t flag){
        return (f & flag) != 0;
    }

    //Add uint8_t
    bool halfCarryAdd8(uint8_t a, uint8_t b){
        return ((a & 0xF) + (b & 0xF)) > 0xF;
    }
    bool carryAdd8(uint8_t a, uint8_t b){
        return uint16_t(a) + uint16_t(b) > 0xFF;
    }

    //Sub uint8_t
    bool halfBorrowSub8(uint8_t a, uint8_t b){
        return (a & 0xF) < (b & 0xF);
    }
    bool borrowSub8(uint8_t a, uint8_t b){
        return a < b;
    }

    //Add uint16_t
    bool halfCarryAdd16(uint16_t a, uint16_t b){
        return ((a & 0xFFF) + (b & 0xFFF)) > 0xFFF;
    }
    bool carryAdd16(uint16_t a, uint16_t b){
        return uint32_t(a) + uint32_t(b) > 0xFFFF;  //uint32
    }

}