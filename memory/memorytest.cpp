#include <iostream>
#include <cstdint>
#include "memory_bus.h"

void assertEqual(uint8_t expected, uint8_t actual, const std::string& testName){
if(expected != actual){
std::cout << "[FAIL] " << testName
<< " | expected: " << (int)expected
<< " got: " << (int)actual << std::endl;
} else {
std::cout << "[PASS] " << testName << std::endl;
}
}

int main(){
MemoryBus bus;

std::cout << "==== MEMORY TEST START ====" << std::endl;

// -------------------------
// WRAM TEST (0xC000–0xDFFF)
// -------------------------
bus.write(0xC000, 0x42);
assertEqual(0x42, bus.read(0xC000), "WRAM basic");

// echo RAM mirror test (0xE000 mirrors 0xC000)
assertEqual(0x42, bus.read(0xE000), "Echo RAM mirror");

// -------------------------
// HRAM TEST (0xFF80–0xFFFE)
// -------------------------
bus.write(0xFF80, 0x99);
assertEqual(0x99, bus.read(0xFF80), "HRAM basic");

// -------------------------
// VRAM TEST (0x8000–0x9FFF)
// -------------------------
bus.write(0x8000, 0x55);
assertEqual(0x55, bus.read(0x8000), "VRAM basic");

// -------------------------
// OAM TEST (0xFE00–0xFE9F)
// -------------------------
bus.write(0xFE00, 0x77);
assertEqual(0x77, bus.read(0xFE00), "OAM basic");

// -------------------------
// IO REGISTERS TEST (0xFF00–0xFF7F)
// -------------------------
bus.write(0xFF00, 0xAB);
assertEqual(0xAB, bus.read(0xFF00), "IO basic (FF00)");

// DIV reset behavior
bus.write(0xFF04, 0xFF);
assertEqual(0x00, bus.read(0xFF04), "DIV reset");

// Interrupt Flag register
bus.write(0xFF0F, 0x1F);
assertEqual(0x1F, bus.read(0xFF0F), "IF register");

// -------------------------
// INTERRUPT ENABLE (0xFFFF)
// -------------------------
bus.write(0xFFFF, 0xAA);
assertEqual(0xAA, bus.read(0xFFFF), "IE register");

// -------------------------
// CARTRIDGE ROM READ TEST
// -------------------------
uint8_t romValue = bus.read(0x0000);
std::cout << "[INFO] ROM[0x0000] = " << (int)romValue << std::endl;

// -------------------------
// CARTRIDGE RAM TEST (if exists)
// -------------------------
bus.write(0xA000, 0x66);
uint8_t cartRam = bus.read(0xA000);
std::cout << "[INFO] Cart RAM test value = " << (int)cartRam << std::endl;

std::cout << "==== MEMORY TEST END ====" << std::endl;

return 0;

}
