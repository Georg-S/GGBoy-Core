#include "BUS.hpp"
#include <cassert>

constexpr static bool isVRAMAddress(uint16_t address) 
{
    return (address >= 0x800 && address <= 0x9FFF);
}

constexpr static bool isCartridgeROM(uint16_t address)
{
    return (address >= 0x000 && address <= 0x7FFF) 
        || (address >= 0xA000 && address <= 0xBFFF);
}

constexpr static bool isCopyMemory(uint16_t address) 
{
    return (address >= 0xE000 && address <= 0xFDFF);
}

void ggb::BUS::setCartridge(Cartridge* cartridge)
{
    m_cartridge = cartridge;
}

uint8_t& ggb::BUS::read(uint16_t address)
{
    if (isCartridgeROM(address))
        return m_cartridge->read(address);
    if (isCopyMemory(address))
        assert(!"PROBLEM");

    //assert(!"Not implemented");
    
    return m_memory[address];
}

void ggb::BUS::write(uint16_t address, uint8_t value)
{
    if (isVRAMAddress(address))
        int b = 3;

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
