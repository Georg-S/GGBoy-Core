#include "BUS.hpp"
#include <cassert>
#include <iostream>

#include "Utility.hpp"

constexpr static bool isVRAMAddress(uint16_t address) 
{
    return (address >= 0x800 && address <= 0x9FFF);
}

constexpr static bool isCartridgeROM(uint16_t address)
{
    return (address >= 0x000 && address <= 0x7FFF);
        //|| (address >= 0xA000 && address <= 0xBFFF);
}

constexpr static bool isEchoMemory(uint16_t address) 
{
    return (address >= 0xE000 && address <= 0xFDFF);
}

void ggb::BUS::setCartridge(Cartridge* cartridge)
{
    m_cartridge = cartridge;
}

uint8_t ggb::BUS::read(uint16_t address) const
{
    if (isCartridgeROM(address))
        return m_cartridge->read(address);
    if (isEchoMemory(address))
        address -= 0x2000;

    //assert(!"Not implemented");

    return m_memory[address];
}

void ggb::BUS::write(uint16_t address, uint8_t value)
{
    if (isVRAMAddress(address))
        int b = 3;
    if (isEchoMemory(address))
        address -= 0x2000;

    // TODO check if ok to always write into this RAM
    m_memory[address] = value;
}

void ggb::BUS::write(uint16_t address, uint16_t value)
{
    assert(!"DON'T USE THIS AS OF NOW");
    //const uint8_t high = value >> 8;
    //const uint8_t low = value && 0xFFFF;

    //assert(address + 1 < m_memory.size());
    //m_memory[address] = high;
    //m_memory[address+1] = low;
}

void ggb::BUS::setBitValue(uint16_t address, int bit, bool bitValue)
{
    auto value = read(address);
    setBitToValue(value, bit, value);
    write(address, value); 
}

bool ggb::BUS::checkBit(uint16_t address, int bit) const
{
    return isBitSet(read(address), bit);
}

void ggb::BUS::printVRAM()
{
    //for (size_t i = 0x8000; i < 0x9FFF; i++) 
    //{
    //    std::cout << (int)m_memory[i] << std::endl;
    //}

    bool print = false;
    for (size_t i = 0x9800; i < 0x9BFF; i++)
    {
        if (m_memory[i] != 0)
            print = true;


    }

    for (size_t i = 0x9800; i < 0x9BFF; i++)
    {
        if (m_memory[i] != 0)
            int b = 3;
        if (print)
            std::cout << (int)m_memory[i] << std::endl;
    }
    if (print)
        int d = 3;
}
