#include "BUS.hpp"
#include <cassert>
#include <iostream>

#include "Constants.hpp"
#include "Utility.hpp"
#include "Timer.hpp"

constexpr static bool isIOAddress(uint16_t address) 
{
    return (address >= 0xFF00 && address <= 0xFFFF);
}

constexpr static bool isVRAMAddress(uint16_t address) 
{
    return (address >= 0x8000 && address <= 0x9FFF);
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

constexpr static bool isUnusedMemory(uint16_t address)
{
    return (address >= 0xFEA0 && address <= 0xFEFF);
}

constexpr static bool isCartridgeRAM(uint16_t address)
{
    return (address >= 0xA000 && address <= 0xBFFF);
}

void ggb::BUS::setCartridge(Cartridge* cartridge)
{
    m_cartridge = cartridge;
}

void ggb::BUS::setTimer(Timer* timer)
{
    m_timer = timer;
}

uint8_t ggb::BUS::read(uint16_t address) const
{
    if (isEchoMemory(address))
        address -= 0x2000;
    if (isCartridgeROM(address))
        return m_cartridge->read(address);
    if (isCartridgeRAM(address))
        return m_cartridge->read(address);
    if (isUnusedMemory(address))
        int c = 3;
    if (isCartridgeRAM(address))
        int de = 3;

    //assert(!"Not implemented");

    return m_memory[address];
}

void ggb::BUS::write(uint16_t address, uint8_t value)
{
    if (isEchoMemory(address))
        address -= 0x2000;

    if (isCartridgeROM(address)) 
    {
        m_cartridge->write(address, value);
        return;
    }
    if (isCartridgeRAM(address))
    {
        m_cartridge->write(address, value);
        return;
    }

    if (address == TIMER_DIVIDER_REGISTER_ADDRESS)
        resetTimerDivider();

    if (isUnusedMemory(address)) 
    {
        return;
        assert(!"Unused memory used");
    }

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
    setBitToValue(value, bit, bitValue);
    write(address, value); 
}

void ggb::BUS::setBit(uint16_t address, int bit)
{
    auto value = read(address);
    ggb::setBit(value, bit);
    write(address, value);
}

void ggb::BUS::resetBit(uint16_t address, int bit)
{
    auto value = read(address);
    ggb::clearBit(value, bit);
    write(address, value);
}

bool ggb::BUS::checkBit(uint16_t address, int bit) const
{
    return isBitSet(read(address), bit);
}

uint8_t* ggb::BUS::getPointerIntoMemory(uint16_t address)
{
    assert(isIOAddress(address)); // As of know, this should be only used for memory mapped IO
    return &m_memory[address];
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

void ggb::BUS::requestInterrupt(int interrupt)
{
    this->setBit(INTERRUPT_REQUEST_ADDRESS, interrupt);
}

void ggb::BUS::resetTimerDivider()
{
    m_timer->resetDividerRegister();
}
