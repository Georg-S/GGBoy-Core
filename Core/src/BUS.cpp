#include "BUS.hpp"
#include <cassert>

void ggb::BUS::setCartridge(Cartridge* cartridge)
{
    m_cartridge = cartridge;
}

uint8_t& ggb::BUS::read(uint16_t address)
{
    if (address >= 0 && address <= 0x7FFF)
        return m_cartridge->read(address);
    if (address >= 0xA000 && address <= 0xBFFF)
        return m_cartridge->read(address);



    assert(!"Not implemented");
    
    return m_memory[address];
}

void ggb::BUS::write(uint16_t address, uint8_t value)
{
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
