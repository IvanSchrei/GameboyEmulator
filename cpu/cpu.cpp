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
        case 0x20:{
            //0x20
            //JR NZ, s8
            //If Z flag is 0, jump s8(next number in memory) steps relative in PC, else execute next instruction (like always)
            bool z = Flags::test(F(), Flags::Z);
            int8_t offset = static_cast<int8_t>(memory_.read(PC_));
            PC_ += 1;
            if(!z){
                PC_ += offset;
            }
            break;
        }
        case 0x21:{
            //0x21
            //LD HL, d16
            //Load next 2 bytes of memory into HL. First byte is lower(0-7) and second is higher(8-15)
            uint8_t low = memory_.read(PC_);
            PC_ += 1;
            uint8_t high = memory_.read(PC_);
            PC_ +=1;
            uint16_t value = (high << 8) | low;
            setHL(value);
            break;
        }
        case 0x22:{
            //0x22
            //LD (HL+), A
            //Store content of A into memory location in HL and increment HL
            uint8_t a = Registers_[REG_A];
            uint16_t hl = getHL();
            memory_.write(hl, a);
            setHL(hl + 1);
            break;
        }
        case 0x23:{
            //0x23
            //INC HL
            //Increment content of HL by 1
            setHL(getHL() + 1);
            break;
        }
        case 0x24:{
            //0x24
            //INC H
            //Increment content of H by 1
            uint8_t h = Registers_[REG_H];
            uint8_t result = h + 1;
            uint8_t newFlags = F() & Flags::C;

            //Z
            Flags::assign(newFlags, Flags::Z, result == 0);

            //N
            Flags::clear(newFlags, Flags::N);

            //H
            Flags::assign(newFlags, Flags::H, Flags::halfCarryAdd8(h, 1));

            Registers_[REG_H] = result;
            F() = newFlags;
            break;
        }
        case 0x25:{
            //0x25
            //DEC H
            //Decrement content of H by 1
            uint8_t h = Registers_[REG_H];
            uint8_t result = h - 1;
            uint8_t newFlags = F() & Flags::C;

            //Z
            Flags::assign(newFlags, Flags::Z, result == 0);

            //N
            Flags::set(newFlags, Flags::N);

            //H
            Flags::assign(newFlags, Flags::H, Flags::halfBorrowSub8(h, 1));

            Registers_[REG_H] = result;
            F() = newFlags;
            break;
        }
        case 0x26:{
            //0x26
            //LD H, d8
            //Load 8 bit operand from memory into H
            uint8_t value = memory_.read(PC_);
            PC_ += 1;
            Registers_[REG_H] = value;
            break;
        }
        case 0x27:{
            //0x27
            //DAA
            //Adjust A to BCD (Binary Coded Decimal) after BCD addition and subtraction operations
            
            bool oldN = Flags::test(F(), Flags::N);
            bool oldH = Flags::test(F(), Flags::H);
            bool oldC = Flags::test(F(), Flags::C);
            uint8_t a = Registers_[REG_A];

            uint8_t newFlags = F() & (Flags::N | Flags::C);

            if(oldN){
                //previous = SUB
                if(oldH){
                    a -= 0x06;
                }
                if(oldC){
                    a-= 0x60;
                }
            }
            else{
                //previous = ADD

                uint8_t adjustmentValue = 0;

                if(oldH ||(a & 0x0F) > 9){
                    adjustmentValue |= 0x06;
                }
                if(oldC || a > 0x99){
                    adjustmentValue |= 0x60;
                    //C
                    Flags::set(newFlags, Flags::C);
                }
                a += adjustmentValue;
            }
            //Z
            Flags::assign(newFlags, Flags::Z, a == 0);

            //H
            Flags::clear(newFlags, Flags::H);
            
            Registers_[REG_A] = a;
            F() = newFlags;
            break;
        }
        case 0x28:{
            //0x28
            //JR Z, s8
            //if Z is 1 jump s8 steps from current PC
            int8_t offset = static_cast<int8_t>(memory_.read(PC_));
            PC_ += 1;
            bool z = Flags::test(F(), Flags::Z);
            if(z){
                PC_ += offset;
            }
            break;
        }
        case 0x29:{
            //0x29
            //ADD HL, HL
            //Add contents of HL to HL and store in HL (3 HL???? Half Life 3 on Gameboy confirmed ????)
            uint8_t newFlags = F() & Flags::Z;
            uint8_t oldHL = getHL();
            //N
            Flags::clear(newFlags, Flags::N);

            //H
            Flags::assign(newFlags, Flags::H, Flags::halfCarryAdd16(oldHL, oldHL));

            //C
            Flags::assign(newFlags, Flags::C, Flags::carryAdd16(oldHL, oldHL));

            setHL(oldHL + oldHL);
            F() = newFlags;
            break;
        }
        case 0x2A:{
            //0x2A
            //LD A, (HL+)
            //Load contents of memory at address saved in HL into A and increment HL
            uint16_t hl = getHL();
            uint8_t value = memory_.read(hl);
            Registers_[REG_A] = value;
            setHL(hl + 1);
            break;
        }
        case 0x2B:{
            //0x2B
            //DEC HL
            //Decrement content of HL by 1
            setHL(getHL() - 1);
            break;
        }
        case 0x2C:{
            //0x2C
            //INC L
            //Increment content of L by 1
            uint8_t l = Registers_[REG_L];
            uint8_t result = l + 1;
            uint8_t newFlags = F() & Flags::C;

            //Z
            Flags::assign(newFlags, Flags::Z, result == 0);

            //N
            Flags::clear(newFlags, Flags::N);

            //H
            Flags::assign(newFlags, Flags::H, Flags::halfCarryAdd8(l, 1));

            Registers_[REG_L] = result;
            F() = newFlags;
            break;
        }
        case 0x2D:{
            //0x2d
            //DEC L
            //Decrement content of L by 1
            uint8_t l = Registers_[REG_L];
            uint8_t result = l - 1;
            uint8_t newFlags = F() & Flags::C;

            //Z
            Flags::assign(newFlags, Flags::Z, result == 0);

            //N
            Flags::set(newFlags, Flags::N);

            //H
            Flags::assign(newFlags, Flags::H, Flags::halfBorrowSub8(l, 1));

            Registers_[REG_L] = result;
            F() = newFlags;
            break;
        }
        case 0x2E:{
            //0x2E
            //LD L d8
            //Load 8 bit operand from memory into L
            uint8_t value = memory_.read(PC_);
            PC_ += 1;
            Registers_[REG_L] = value;
            break;
        }
        case 0x2F:{
            //0x2F
            //CPL
            //Flip all bits of A
            uint8_t newFlags = F() & (Flags::Z | Flags::C);

            //N
            Flags::set(newFlags, Flags::N);

            //H
            Flags::set(newFlags, Flags::H);

            //flip
            Registers_[REG_A] = ~Registers_[REG_A];
            F() = newFlags;
            break;
        }
        case 0x30:{
            //0x30
            //JR NC, s8
            //If carry flag is 0 jump s8 steps from PC, else execute next instruction
            bool c = Flags::test(F(), Flags::C);
            int8_t offset = static_cast<int8_t>(memory_.read(PC_));
            PC_ += 1;
            if(!c){
                PC_ += offset;
            }
            break;
        }
        case 0x31:{
            //0x31
            //LD SP, d16
            //Load next 2 bytes of memory into SP(Stack Pointer), first byte is lower (0-7), second is higher (8-15)
            uint8_t low = memory_.read(PC_);
            PC_ += 1;
            uint8_t high = memory_.read(PC_);
            PC_ += 1;
            uint16_t value = (high << 8) | low;
            SP_ = value;
            break;
        }
        case 0x32:{
            //0x32
            //LD (HL-), A
            //Store contents of A into memory location in HL, then decrement HL by 1
            uint8_t a = Registers_[REG_A];
            uint16_t hl = getHL();
            memory_.write(hl, a);
            setHL(hl - 1);
            break;
        }
        case 0x33:{
            //0x33
            //INC SP
            //Increment SP (Stack Pointer) by 1
            SP_ = SP_ + 1;
            break;
        }
        case 0x34:{
            //0x34
            //INC (HL)
            //Increment content of address in HL by 1
            uint16_t hl = getHL();
            uint8_t oldValue = memory_.read(hl);
            uint8_t newValue = oldValue + 1;
            uint8_t newFlags = F() & Flags::C;
            
            //Z
            Flags::assign(newFlags, Flags::Z, newValue == 0);

            //N
            Flags::clear(newFlags, Flags::N);

            //H
            Flags::assign(newFlags, Flags::H, Flags::halfCarryAdd8(oldValue, 1));

            memory_.write(hl, newValue);
            F() = newFlags;
            break;
        }
        case 0x35:{
            //0x35
            //DEC (HL)
            //Decrement content of address in HL by 1
            uint16_t hl = getHL();
            uint8_t oldValue = memory_.read(hl);
            uint8_t newValue = oldValue - 1;
            uint8_t newFlags = F() & Flags::C;

            //Z
            Flags::assign(newFlags, Flags::Z, newValue == 0);

            //N
            Flags::set(newFlags, Flags::N);

            //H
            Flags::assign(newFlags, Flags::H, Flags::halfBorrowSub8(oldValue, 1));

            memory_.write(hl, newValue);
            F() = newFlags;
            break;
        }
        case 0x36:{
            //0x36
            //LD (HL), d8
            //Store next 8 bits from memory at address in HL
            uint16_t hl = getHL();
            uint8_t value = memory_.read(PC_);
            PC_ += 1;
            memory_.write(hl, value);
            break;
        }
        case 0x37:{
            //0x37
            //SCF
            //Set carry Flag
            uint8_t newFlags = F() & Flags::Z;

            //N
            Flags::clear(newFlags, Flags::N);

            //H
            Flags::clear(newFlags, Flags::H);

            //C
            Flags::set(newFlags, Flags::C);

            F() = newFlags;
            break;
        }
        case 0x38:{
            //0x38
            //JR C, s8
            //If carry flag is 1 jump s8 steps from PC, else execture next instruction
            bool c = Flags::test(F(), Flags::C);
            int8_t offset = static_cast<int8_t>(memory_.read(PC_));
            PC_ += 1;
            
            if(c){
                PC_ += offset;
            }

            break;
        }
        case 0x39:{
            //0x39
            //ADD HL, SP
            //Add content of SP tp content of HL and write it into HL
            uint16_t hl = getHL();
            uint16_t result = hl + SP_;
            uint8_t newFlags = F() & Flags::Z;

            //N
            Flags::clear(newFlags, Flags::N);

            //H
            Flags::assign(newFlags, Flags::H, Flags::halfCarryAdd16(hl, SP_));

            //C
            Flags::assign(newFlags, Flags::C, Flags::carryAdd16(hl, SP_));

            setHL(result);
            F() = newFlags;
            break;
        }
        case 0x3A:{
            //0x3A
            //LD A, (HL-)
            //Load content of address in HL into A, decrement HL
            uint16_t hl = getHL();
            Registers_[REG_A] = memory_.read(hl);
            setHL(hl - 1);
            break;
        }
        case 0x3B:{
            //0x3B
            //DEC SP
            //Decrement SP by 1
            SP_ -= 1;
            break;
        }
        case 0x3C:{
            //0x3C
            //INC A
            //Increment content in A by 1
            uint8_t a = Registers_[REG_A];
            uint8_t result = a + 1;
            uint8_t newFlags = F() & Flags::C;

            //Z
            Flags::assign(newFlags, Flags::Z, result == 0);

            //N
            Flags::clear(newFlags, Flags::N);

            //H
            Flags::assign(newFlags, Flags::H, Flags::halfCarryAdd8(a, 1));

            Registers_[REG_A] = result;            
            F() = newFlags;
            break;
        }
        case 0x3D:{
            //0x3D
            //DEC A
            //Decrement content of A by 1
            uint8_t a = Registers_[REG_A];
            uint8_t result = a - 1;
            uint8_t newFlags = F() & Flags::C;

            //Z
            Flags::assign(newFlags, Flags::Z, result == 0);

            //N
            Flags::set(newFlags, Flags::N);

            //H
            Flags::assign(newFlags, Flags::H, Flags::halfBorrowSub8(a, 1));

            Registers_[REG_A] = result;
            F() = newFlags;
            break;
        }
        case 0x3E:{
            //0x3E
            //LD A, d8
            //Loads next 8 bit from memory into A
            Registers_[REG_A] = memory_.read(PC_);
            PC_ += 1;
            break;
        }
        case 0x3F:{
            //0x3F
            //CCF
            //Flip the carry flag
            uint8_t newFlags = F() & Flags::Z;
            bool oldCarry = Flags::test(F(), Flags::C); 

            //N
            Flags::clear(newFlags, Flags::N);

            //H
            Flags::clear(newFlags, Flags::H);

            //C
            Flags::assign(newFlags, Flags::C, !oldCarry);

            F() = newFlags;
            break;
        }
        case 0x40:{
            //0x40
            //LD B, B
            //Load content of B into B, so B = B which is useless, at least i think so
            Registers_[REG_B] = Registers_[REG_B];
            break;
        }
        case 0x41:{
            //0x41
            //LD B, C
            //Load content of C into B
            Registers_[REG_B] = Registers_[REG_C];
            break;
        }
        case 0x42:{
            //0x42
            //LD B, D
            //Load content of D into B
            Registers_[REG_B] = Registers_[REG_D];
            break;
        }
        case 0x43:{
            //0x43
            //LD B, E
            //Load content of E into B
            Registers_[REG_B] = Registers_[REG_E];
            break;
        }
        case 0x44:{
            //0x44
            //LD B, H
            //Load contents of H into B
            Registers_[REG_B] = Registers_[REG_H];
            break;
        }
        case 0x45:{
            //0x45
            //LD B, L
            //Load contetns of L into B
            Registers_[REG_B] = Registers_[REG_L];
            break;
        }
        case 0x46:{
            //0x46
            //LD B, (HL)
            //Load content of address from HL into B
            Registers_[REG_B] = memory_.read(getHL());
            break;
        }
        case 0x47:{
            //0x47
            //LD B, A
            //Load content of A into B
            Registers_[REG_B] = Registers_[REG_A];
            break;
        }
        case 0x48:{
            //0x48
            //LD C, B
            //Load content of B into C
            Registers_[REG_C] = Registers_[REG_B];
            break;
        }
        case 0x49:{
            //0x49
            //LD C, C
            //Load content of C into C, which once again does nothing (will happen again in future)
            Registers_[REG_C] = Registers_[REG_C];
            break;
        }
        case 0x4A:{
            //0x4A
            //LD C, D
            //Load content of D into C
            Registers_[REG_C] = Registers_[REG_D];
            break;
        }
        case 0x4B:{
            //0x4B
            //LD C, E
            //Load content of E into C
            Registers_[REG_C] = Registers_[REG_E];
            break;
        }
        case 0x4C:{
            //0x4C
            //LD C, H
            //Load content of H into C
            Registers_[REG_C] = Registers_[REG_H];
            break;
        }
        case 0x4D:{
            //0x4D
            //LD C, L
            //Load content of L into C
            Registers_[REG_C] = Registers_[REG_L];
            break;
        }
        case 0x4E:{
            //0x4E
            //LD C, (HL)
            //Load content at address in HL into C
            Registers_[REG_C] = memory_.read(getHL());
            break;
        }
        case 0x4F:{
            //0x4F
            //LD C, A
            //Load content of A into C
            Registers_[REG_C] = Registers_[REG_A];
            break;
        }
        case 0x50:{
            //0x50
            //LD D, B
            //Load content of B into D
            Registers_[REG_D] = Registers_[REG_B];
            break;
        }
        case 0x51:{
            //0x51
            //LD D, C
            //Load content of C into D
            Registers_[REG_D] = Registers_[REG_C];
            break;
        }
        case 0x52:{
            //0x52
            //LD D, D
            //Load content of D into D (useless again just like me)
            Registers_[REG_D] = Registers_[REG_D];
            break;
        }
        case 0x53:{
            //0x53
            //LD D, E
            //Load content of E into D
            Registers_[REG_D] = Registers_[REG_E];
            break;
        }
        case 0x54:{
            //0x54
            //LD D, H
            //Load content of H into D
            Registers_[REG_D] = Registers_[REG_H];
            break;
        }
        case 0x55:{
            //0x55
            //LD D, L
            //Load content of L into D
            Registers_[REG_D] = Registers_[REG_L];
            break;
        }
        case 0x56:{
            //0x56
            //LD D, (HL)
            //Load content at address stored in HL into D
            Registers_[REG_D] = memory_.read(getHL());
            break;
        }
        case 0x57:{
            //0x57
            //LD D, A
            //Load content of A into D
            Registers_[REG_D] = Registers_[REG_A];
            break;
        }
        case 0x58:{
            //0x58
            //LD E, B
            //Load content of B into E
            Registers_[REG_E] = Registers_[REG_B];
            break;
        }
        case 0x59:{
            //0x59
            //LD E, C
            //Load content of C into E
            Registers_[REG_E] = Registers_[REG_C];
            break;
        }
        case 0x5A:{
            //0x5A
            //LD E, D
            //Load content of D into E
            Registers_[REG_E] = Registers_[REG_D];
            break;
        }
        case 0x5B:{
            //0x5B
            //LD E, E
            //Load content of E into E, useless
            Registers_[REG_E] = Registers_[REG_E];
            break;
        }
        case 0x5C:{
            //0x5C
            //LD E, H
            //Load content of H into E
            Registers_[REG_E] = Registers_[REG_H];
            break;
        }
        case 0x5D:{
            //0x5D
            //LD E, L
            //Load content of L into E
            Registers_[REG_E] = Registers_[REG_L];
            break;
        }
        case 0x5E:{
            //0x5E
            //LD E, (HL)
            //Load content at address in HL into E
            Registers_[REG_E] = memory_.read(getHL());
            break;
        }
        case 0x5F:{
            //0x5F
            //LD E, A
            //Load content of A into E
            Registers_[REG_E] = Registers_[REG_A];
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