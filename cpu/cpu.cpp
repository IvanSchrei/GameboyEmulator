#include <cstdint>

#include "cpu.h"
#include "../memory/memory_bus.h"
#include "helpers.h"

enum {
    REG_A, REG_F,
    REG_B, REG_C,
    REG_D, REG_E,
    REG_H, REG_L
};

CPU::CPU(MemoryBus& memory) : memory_(memory) {}

CPU::~CPU() {}

uint8_t& CPU::F()
{
    return Registers_[REG_F];
}

void CPU::initialize(){
    //initialize Programm Counter
    //starts at 0x100 = 256
    PC_ = 0x100;

    //initialize Registers by setting them to 0x0
    for(int i = 0; i<8; ++i){
        Registers_[i] = 0x0;
    }

    //initialize Stackpointer, starts at 0xFFFE
    SP_ = 0xFFFE;
}

//helpers for addressing 16-bit Registers
uint16_t CPU::getAF(){
    uint16_t ret = (Registers_[REG_A] << 8) | Registers_[REG_F];
    return ret;
}

void CPU::setAF(uint16_t value){
    Registers_[REG_A] = (value & 0xFF00) >> 8; //A
    Registers_[REG_F] = (value & 0x00F0); //F, only high nibble
}

uint16_t CPU::getBC(){
    uint16_t ret = (Registers_[REG_B] << 8) | Registers_[REG_C];
    return ret;
}

void CPU::setBC(uint16_t value){
    Registers_[REG_B] = (value & 0xFF00) >> 8; //B
    Registers_[REG_C] = (value & 0x00FF); //C
}

uint16_t CPU::getDE(){
    uint16_t ret = (Registers_[REG_D] << 8) | Registers_[REG_E];
    return ret;
}

void CPU::setDE(uint16_t value){
    Registers_[REG_D] = (value & 0xFF00) >> 8; //D
    Registers_[REG_E] = (value & 0x00FF); //E
}

uint16_t CPU::getHL(){
    uint16_t ret = (Registers_[REG_H] << 8) | Registers_[REG_L];
    return ret;
}

void CPU::setHL(uint16_t value){
    Registers_[REG_H] = (value & 0xFF00) >> 8; //H
    Registers_[REG_L] = (value & 0x00FF); //L
}

void CPU::cycle(){
    //fetch
    //get opcode, which is 8 bits starting from PC_ and depending on it we will then execute an instruction
    //opcode can also be longer or have additional parameters stored on following addresses but that is handled later
    uint8_t opcode = memory_.read(PC_);
    PC_ += 1;
    //decode
    //get type of the instruction, probs stored in first nibble or two

    //execute
    //switch statement that decides what happens, get Params for instruction
    switch(opcode){
        case 0x00: {
            //0x00
            //NOP
            //just increase PC by one, which we already did.
            break;
        }
        case 0x01:{
            //0x01
            //LD BC d16
            //Load next two bytes into BC
            //3 bytes
            uint8_t lowByte = memory_.read(PC_);
            PC_ += 1;
            uint8_t highByte = memory_.read(PC_);
            uint16_t value = (highByte << 8) | lowByte;
            setBC(value);
            PC_ += 1;
            break;
        }
        case 0x02:{
            //0x02
            //LD BC, A
            //Store value of A into memory specified in BC
            //1 byte
            uint8_t value = Registers_[REG_A];
            uint16_t address = getBC();
            memory_.write(address, value);
            break;
        }
        case 0x03:{
            //0x03
            //INC BC
            //increment content of BC by 1
            //1 byte
            setBC(getBC() + 1);
            break;
        }
        case 0x04:{
            //0x04
            //INC B
            //increment content of B by 1
            //FLAGS: update Z and H, set N to 0
            //1 byte
            uint8_t b = Registers_[REG_B];
            uint8_t result = b + 1;
            uint8_t newFlags = F() & Flags::C;
            
            //set Z flag
            Flags::assign(newFlags, Flags::Z, result==0);

            //set H flag
            Flags::assign(newFlags, Flags::H, Flags::halfCarryAdd8(b, 1));

            Registers_[REG_B] = result;
            F() = newFlags;
            break;
        }
        case 0x05:{
            //0x05
            //DEC B
            //decrement content of B by 1
            //FLAGS: update Z and H, set N to 1
            //1 byte
            uint8_t b = Registers_[REG_B];
            uint8_t result = b - 1;
            uint8_t newFlags = F() & Flags::C;

            //set Z flag
            Flags::assign(newFlags, Flags::Z, result == 0);

            //set H flag
            Flags::assign(newFlags, Flags::H, Flags::halfBorrowSub8(b, 1));

            //set N flag
            Flags::set(newFlags, Flags::N);

            Registers_[REG_B] = result;
            F() = newFlags;
            break;
        }
        case 0x06:{
            //0x06
            //LD B d8
            //load d8(next Byte) into B
            //2 bytes
            uint8_t value = memory_.read(PC_);
            Registers_[REG_B] = value;
            PC_ += 1;
            break;
        }
        case 0x07:{
            //0x07
            //RLCA
            //Rotate contents of Register A to the left.
            uint8_t a = Registers_[REG_A];
            uint8_t leftmostBit = (a & 0x80) >> 7;
            uint8_t result = (a << 1) | leftmostBit;
            uint8_t newFlags = 0;
            Registers_[REG_A] = result;

            //C
            Flags::assign(newFlags, Flags::C, leftmostBit);

            F() = newFlags;
            break;
        }
        case 0x08:{
            //0x08
            //LD a16 SP
            //store lower byte of SP_ at the 16 bit address a16, store upper byte of SP_ at a16+1
            //3 Bytes [opcode, a16(1), a16(2)]

            uint8_t low = memory_.read(PC_);
            PC_ += 1;
            uint8_t high = memory_.read(PC_);
            uint16_t address = (high << 8) | low;

            uint8_t lowByte = SP_ & 0xFF;
            
            uint8_t highByte = (SP_ >> 8) & 0xFF;
            memory_.write(address, lowByte);
            memory_.write(address + 1, highByte);
            break;
        }
        case 0x09:{
            //0x09
            //ADD HL BC
            //add the contents of BC to contents of HL and store in HL
            //FLAGS: N = 0, calculate H and C flags
            uint16_t hl = getHL();
            uint16_t bc = getBC();
            uint16_t result = hl + bc;
            
            //Flags, only get Z
            uint8_t newFlags = F() & Flags::Z;
            
            //H Flag
            Flags::assign(newFlags, Flags::H, Flags::halfCarryAdd16(hl, bc));

            //C Flag
            Flags::assign(newFlags, Flags::C, Flags::carryAdd16(hl, bc));
            
            setHL(result);
            F() = newFlags;
            break;
        }
        case 0x0A:{
            //0x0A
            //LD A BC
            //load memory content at address BC into A
            uint16_t bc = getBC();
            Registers_[REG_A] = memory_.read(bc);
            break;
        }
        case 0x0B:{
            //0x0B
            //DEC BC
            //decrement content of BC by 1
            uint16_t bc = getBC();
            bc -= 1;
            setBC(bc);
            break;
        }
        case 0x0C:{
            //0x0C
            //INC C
            //increment content of C by 1
            //FLAGS: calculate Z and H, set N to 0
            uint8_t c = Registers_[REG_C];
            uint8_t result = c + 1;
            uint8_t newFlags = F() & Flags::C;

            //set Z flag
            Flags::assign(newFlags, Flags::Z, result==0);

            //set H flag
            Flags::assign(newFlags, Flags::H, Flags::halfCarryAdd8(c, 1));

            Registers_[REG_C] = result;
            F() = newFlags;
            break;
        }
        case 0x0D:{
            //0x0D
            //DEC C
            //decrement content of C by 1
            //FLAGS: calculate Z and H, set N to 1
            uint8_t d = Registers_[REG_C];
            uint8_t result = d - 1;
            uint8_t newFlags = F() & Flags::C;

            //set Z flag
            Flags::assign(newFlags, Flags::Z, result == 0);

            //set N flag
            Flags::set(newFlags, Flags::N);

            //set H flag
            Flags::assign(newFlags, Flags::H, Flags::halfBorrowSub8(d, 1));

            Registers_[REG_C] = result;
            F() = newFlags;
            break;
        }
        case 0x0E:{
            //0x0E
            //LD C d8
            //load next 8 bits into C
            //2 bytes
            uint8_t value = memory_.read(PC_);
            PC_ += 1;
            Registers_[REG_C] = value;
            break;
        }
        case 0x0F:{
            //0x0F
            //RRCA
            //Rotate the Contents of A to the right
            uint8_t a = Registers_[REG_A];
            uint8_t rightmostBit = (a & 0x01);
            uint8_t result = (rightmostBit << 7)|(a >> 1);
            uint8_t newFlags = 0;
            
            //C
            Flags::assign(newFlags, Flags::C, rightmostBit);

            Registers_[REG_A] = result;
            F() = newFlags;
            break;
        }
        case 0x10:{
            //0x1000
            //STOP
            //enter stop mode, stop system clock (not yet implemented) and LCD COntroller (idk), Registers stay unchanged
            //Can be canceled by RESET
            //reset IE flags, Input to P10-P13 is LOW
            //2 bytes
            PC_ += 1;
            break;
        }
        case 0x11:{
            //0x11
            //LD DE d16
            //Load next 2 bytes into DE, first = lower, second = higher
            uint8_t lowByte = memory_.read(PC_);
            PC_ += 1;
            uint8_t highByte = memory_.read(PC_);
            PC_ += 1;
            uint16_t value = (highByte << 8) | lowByte;
            setDE(value);
            break;
        }
        case 0x12:{
            //0x12
            //LD DE A
            //Store contents of A into memory location specified by DE
            uint8_t value = Registers_[REG_A];
            uint16_t address = getDE();
            memory_.write(address, value);
            break;
        }
        case 0x13:{
            //0x13
            //INC DE
            //Increment content of register pair DE by 1
            uint16_t value = getDE();
            setDE(value);
            break;
        }
        case 0x14:{
            //0x14
            //INC D
            //Increment content of register D by 1
            uint8_t d = Registers_[REG_D];
            uint8_t result = d + 1;
            uint8_t newFlags = F() & Flags::C;

            //H Flag
            Flags::assign(newFlags, Flags::H, Flags::halfCarryAdd8(d, 1));

            //Z Flag
            Flags::assign(newFlags, Flags::Z, result = 0);

            Registers_[REG_D] = result;
            F() = newFlags;
            break;
        }
        case 0x15:{
            //0x15
            //DEC D
            //Decrement content of register D by 1
            uint8_t d = Registers_[REG_D];
            uint8_t result = d - 1;
            uint8_t newFlags = Registers_[REG_F] & 0x10;

            //set Z flag
            Flags::assign(newFlags, Flags::Z, result == 0);

            //set N flag
            Flags::set(newFlags, Flags::N);

            //set H flag
            Flags::assign(newFlags, Flags::H, Flags::halfBorrowSub8(d, 1));

            Registers_[REG_D] = result;
            F() = newFlags;
            break;
        }
        case 0x16:{
            //0x16
            //LD d d8
            //Load next 8 bit number from memory into D
            uint8_t value = memory_.read(PC_);
            Registers_[REG_D] = value;
            PC_ += 1;
            break;
        }
        case 0x17:{
            //0x17
            //RLA
            //Rotate contents of Register A left through C Flag. Leftmost into Carry, Carry into Rightmost
            uint8_t newFlags = 0;
            uint8_t leftmost = (Registers_[REG_A] >> 7) & 1;
            bool oldCarry = Flags::test(F(), Flags::C);
            Registers_[REG_A] = (Registers_[REG_A] << 1) | oldCarry;
            Flags::assign(newFlags, Flags::C, leftmost);
            F() = newFlags;
            break;
        }
        case 0x18:{
            //0x18
            //JR s8
            //Jump Relative from current address in PC. s8 = signed 8 bit int
            int8_t offset = static_cast<int8_t>(memory_.read(PC_));
            PC_ += 1;
            PC_ += offset;
            break;
        }
        case 0x19:{
            //0x19
            //ADD HL, DE
            //Add contents of register DE to contents of register HL. Sotre result in HL
            uint16_t hl = getHL();
            uint16_t de = getDE();
            uint16_t result = hl + de;
            
            uint8_t newFlags = F() & Flags::Z;

            //N
            Flags::clear(newFlags, Flags::N);

            //H
            Flags::assign(newFlags, Flags::H, Flags::halfCarryAdd16(hl, de));

            //C
            Flags::assign(newFlags, Flags::C, Flags::carryAdd16(hl, de));

            setHL(result);
            F() = newFlags;
            break;
        }
        case 0x1A:{
            //0x1A
            //LD A, (DE)
            //Load 8-bit content of memory at address saved in DE into register A
            uint16_t de = getDE();
            uint8_t value = memory_.read(de);
            Registers_[REG_A] = value;

            break;
        }
        case 0x1B:{
            //0x1B
            //DEC DE
            //Decrement content of DE by 1
            uint16_t de = getDE();
            uint16_t result = de - 1;
            setDE(result);

            break;
        }
        case 0x1C:{
            //0x1C
            //INC E
            //Increment content of E by 1
            uint8_t e = Registers_[REG_E];
            uint8_t result = e + 1;
            uint8_t newFlags = F() & Flags::C;

            //Z
            Flags::assign(newFlags, Flags::Z, result == 0);

            //N
            Flags::clear(newFlags, Flags::N);

            //H
            Flags::assign(newFlags, Flags::H, Flags::halfCarryAdd8(e, 1));

            Registers_[REG_E] = result;
            F() = newFlags;
            break;
        }
        case 0x1D:{
            //0x1D
            //DEC E
            //Decrement contents of E by 1
            uint8_t e = Registers_[REG_E];
            uint8_t result = e - 1;
            uint8_t newFlags = F() & Flags::C;

            //Z
            Flags::assign(newFlags, Flags::Z, result == 0);

            //N 
            Flags::set(newFlags, Flags::N);

            //H
            Flags::assign(newFlags, Flags::H, Flags::halfBorrowSub8(e, 1));

            Registers_[REG_E] = result;
            F() = newFlags;
            break;
        }
        case 0x1E:{
            //0x1E
            //LD E, d8
            //Load 8 bit operand in memory into E
            uint8_t value = memory_.read(PC_);
            PC_ += 1;
            Registers_[REG_E] = value;
            break;
        }
        case 0x1F:{
            //0x1F
            //RRA
            //Rotate A to the right through Carry.
            uint8_t newFlags = 0;
            uint8_t rightmost = Registers_[REG_A] & 1;
            bool oldCarry = Flags::test(F(), Flags::C);
            Registers_[REG_A] = (Registers_[REG_A] >> 1) | (oldCarry << 7);
            Flags::assign(newFlags, Flags::C, rightmost);
            F() = newFlags;
            break;
        }


        case 0xCB:{
            //16 bit opcodes
            uint8_t opcode2 = memory_.read(PC_);
            uint16_t fullOpcode = (opcode << 8) | opcode2;
            //switch for 16 bit instructions
            break;
        }

        default: {
            //Unknown instruction
        }
    } 
}