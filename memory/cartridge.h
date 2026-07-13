#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <cstdint>
#include <vector>
#include <string>

class Cartridge{
    public:

        Cartridge();
        Cartridge(const std::string& path);
        ~Cartridge();

        bool loadCartridge(const std::string& path);
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t value);
    private:
        //ROM data
        std::vector<uint8_t> rom;
        //optional MBC
        uint8_t currentRAMBank;
        bool ramEnabled;
        uint8_t bankingMode;
        std::vector<uint8_t> externalRAM;
        uint16_t ROMsize;
        uint32_t RAMsize;
        uint8_t MBCtype;
        uint8_t lowerROMbits;
        uint8_t upperROMbits;
        uint16_t romBankNumber;
};

#endif