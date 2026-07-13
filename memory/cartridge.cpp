#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>

#include "cartridge.h"

// This file is CLOSE but still lying about MBC1 behavior in multiple places
// Main issue: you're not COMBINING bank bits, you're just overwriting them

Cartridge::Cartridge(const std::string& path){
    loadCartridge(path);
}

Cartridge::Cartridge() : Cartridge("") {}

Cartridge::~Cartridge(){}

bool Cartridge::loadCartridge(const std::string& path){
    std::ifstream input_stream;
    input_stream.open(path, std::ios::binary);

    if(!input_stream){
        return false;
    }

    rom = std::vector<uint8_t>(
        std::istreambuf_iterator<char>(input_stream),
        std::istreambuf_iterator<char>()
    );

    if(rom.size() < 0x150){
        return false;
    }

    //validate header checksum
    uint8_t checksum = 0;
    for(uint16_t addr = 0x0134; addr <= 0x014C; addr++){
        checksum = checksum - rom[addr] - 1;
    }

    uint8_t ByteAt14D = rom[0x014D];
    uint8_t checksumLow = (checksum & 0xFF);
    if(ByteAt14D != checksumLow){
        return false;
    }

    // -------------------------
    // HEADER PARSING
    // -------------------------

    uint8_t romSizeCode = rom[0x0148];
    switch(romSizeCode){
        case 0x00:{
            //2 banks
            romBankNumber = 2;
            break;
        }
        case 0x01:{
            //4 banks
            romBankNumber = 4;
            break;
        }
        case 0x02:{
            //8 banks
            romBankNumber = 8;
            break;
        }
        case 0x03:{
            //16 banks
            romBankNumber = 16;
            break;
        }
        case 0x04:{
            //32 banks
            romBankNumber = 32;
            break;
        }
        case 0x05:{
            //64 banks
            romBankNumber = 64;
            break;
        }
        case 0x06:{
            //128 banks
            romBankNumber = 128;
            break;
        }
        case 0x07:{
            //256 banks
            romBankNumber = 256;
            break;
        }
        case 0x52:{
            //72 banks
            romBankNumber = 72;
            break;
        }
        case 0x53:{
            //80 banks
            romBankNumber = 80;
            break;
        }
        case 0x54:{
            //96 banks
            romBankNumber = 96;
            break;
        }
    }

    uint8_t ramSizeCode = rom[0x0149];
    switch(ramSizeCode){
        case 0x0:{
            RAMsize = 0;
            break;
        }
        case 0x1:{
            // ❗ BUG:
            // you leave RAMsize UNINITIALIZED here
            // 👉 explicitly set RAMsize = 0 or handle properly
            RAMsize = 0;
            break;
        }
        case 0x2:{
            RAMsize = 0x2000;
            break;
        }
        case 0x3:{
            RAMsize = 0x8000; 
            break;
        }
        case 0x4:{
            RAMsize = 0x20000;
            break;
        }
        case 0x5:{
            RAMsize = 0x10000;
            break;
        }
    }

    externalRAM.resize(RAMsize);

    uint8_t type = rom[0x0147];
    MBCtype = type;

    switch(type){
        case 0x00:
            std::cout << "MBC0 (no banking)\n";
            break;

        case 0x01:
        case 0x02:
        case 0x03:
            std::cout << "MBC1\n";
            break;

        default:
            std::cout << "Unsupported MBC for now\n";
            break;
    }

    // -------------------------
    // INITIAL STATE
    // -------------------------

    ramEnabled = false;

    lowerROMbits = 1;
    upperROMbits = 0;

    currentRAMBank = 0;

    bankingMode = 0;

    return true;
}

uint8_t Cartridge::read(uint16_t address){

    uint16_t bank;

    if(address <= 0x3FFF){

        if(bankingMode == 0){
            bank = 0;
        }
        else{
            bank = (upperROMbits << 5);
            bank %= romBankNumber;
        }

        uint32_t offset = bank * 0x4000 + address;
        if(offset < rom.size()){
            return rom[offset];
        }
        
        return 0xFF;
    }

    else if(address <= 0x7FFF){
        // build FULL bank number before calculating offset

        if(bankingMode == 0){
            //ROM banking mode
            bank = (upperROMbits << 5) | lowerROMbits;
        }
        else if(bankingMode == 1){
            bank = lowerROMbits;

        }

        uint16_t numberOfBanks = romBankNumber;
        bank %= numberOfBanks;

        //
        uint32_t offset = bank * 0x4000 + (address - 0x4000);

        if(offset < rom.size()){
            return rom[offset];
        }

        return 0xFF;
    }

    else if(address <= 0xBFFF){

        if(!ramEnabled || externalRAM.empty()){
            return 0xFF;
        }
        if(bankingMode == 0){
            bank = 0;
        }
        else{
            bank = currentRAMBank;
        }


        uint32_t offset = bank * 0x2000 + (address - 0xA000);

        if(offset < externalRAM.size()){
            return externalRAM[offset];
        }

        return 0xFF;
    }

    return 0xFF;
}

void Cartridge::write(uint16_t address, uint8_t value){

    if(address <= 0x1FFF){
        ramEnabled = ((value & 0x0F) == 0x0A);
    }
    else if(address <= 0x3FFF){
        // lower 5 bits of ROM bank

        lowerROMbits = value & 0x1F;

        if(lowerROMbits == 0){
            lowerROMbits = 1;
        }
    }

    else if(address <= 0x5FFF){
        // this register is dual-purpose

        if(bankingMode == 0){
            //ROM banking mode

            // take value & 0x03 (2 bits)
            // store as upper ROM bits
            upperROMbits = (value & 0x03);
        }
        else if(bankingMode == 1){
            // RAM banking mode
            currentRAMBank = value & 0x03;
        }
    }

    else if(address <= 0x7FFF){
        bankingMode = value & 0x01;
        // switching mode changes meaning of previous writes
        // 👉 sometimes you must recompute active bank
    }

    else if(address <= 0xBFFF){
        if(ramEnabled && !externalRAM.empty()){
            uint32_t offset;
            uint16_t bank;
            if(bankingMode == 0){
                //ram bank = 0
                bank = 0;
            }
            else{
                bank = currentRAMBank;
            }
            offset = bank * 0x2000 + (address - 0xA000);
            if(offset < externalRAM.size()){
                externalRAM[offset] = value;
            }
        }
    }
}