#include "BUS.hpp"
#include <cassert>

void ggb::BUS::setCartridge(Cartridge* cartridge)
{
    m_cartridge = cartridge;
}

uint8_t ggb::BUS::read(uint16_t address)
{
    if (address >= 0 && address <= 0x7FFF)
        return m_cartridge->read(address);
    if (address >= 0xA000 && address <= 0xBFFF)
        return m_cartridge->read(address);



    assert(!"Not implemented");
    
    return 0;
}
