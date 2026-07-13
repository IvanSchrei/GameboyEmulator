#ifndef HELPERS_H
#define HELPERS_H

#include <cstdint>
#include <array>

namespace Flags
{
    constexpr uint8_t Z = 0x80;
    constexpr uint8_t N = 0x40;
    constexpr uint8_t H = 0x20;
    constexpr uint8_t C = 0x10;

    void set(uint8_t& f, uint8_t flag);
    void clear(uint8_t& f, uint8_t flag);
    void assign(uint8_t& f, uint8_t flag, bool value);
    bool test(uint8_t& f, uint8_t flag);

    bool halfCarryAdd8(uint8_t a, uint8_t b);
    bool carryAdd8(uint8_t a, uint8_t b);

    bool halfBorrowSub8(uint8_t a, uint8_t b);
    bool borrowSub8(uint8_t a, uint8_t b);

    bool halfCarryAdd16(uint16_t a, uint16_t b);
    bool carryAdd16(uint16_t a, uint16_t b);
}

#endif