#include "BUS.hpp"
#include <cassert>
#include <iostream>

#include "Constants.hpp"
#include "Utility.hpp"
#include "Timer.hpp"
#include "PixelProcessingUnit.hpp"

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

constexpr static bool isObjectAttributeMemory(uint16_t address) 
{
    return (address >= 0xFE00 && address <= 0xFE9F);
}

void ggb::BUS::reset()
{
    m_memory = std::vector<uint8_t>(uint16_t(0xFFFF) + 1, 0);
}

void ggb::BUS::setCartridge(Cartridge* cartridge)
{
    m_cartridge = cartridge;
}

void ggb::BUS::setTimer(Timer* timer)
{
    m_timer = timer;
}

void ggb::BUS::setPixelProcessingUnit(PixelProcessingUnit* ppu)
{
    m_ppu = ppu;
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
        return 0xFF; // Reading from unused/invalid memory
    if (isCartridgeRAM(address))
        int de = 3; // TODO should probably write into the cartridge (because of RAM banking)
    if (address == 0xFF46) // DMA Transfer address, write only
        return 0xFF;

    return m_memory[address];
}

int8_t ggb::BUS::readSigned(uint16_t address) const
{
    return static_cast<int8_t>(read(address));
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
    {
        resetTimerDivider();
        return;
    }

    if (address == START_DIRECT_MEMORY_ACCESS_ADDRESS)
        directMemoryAccess(value);

    if (isUnusedMemory(address)) 
    {
        return; // Writing to unused/invalid memory does nothing
        assert(!"Unused memory used");
    }

    m_memory[address] = value;
}

void ggb::BUS::write(uint16_t address, uint16_t value)
{
    assert(!"DON'T USE THIS AS OF NOW");
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
    return &m_memory[address];
}

void ggb::BUS::requestInterrupt(int interrupt)
{
    ggb::setBit(m_memory[INTERRUPT_REQUEST_ADDRESS], interrupt);
}

void ggb::BUS::resetTimerDivider()
{
    m_timer->resetDividerRegister();
}

void ggb::BUS::directMemoryAccess(uint8_t value)
{
    // TODO maybe not do this in every function call, move into contructor
    // TODO if this project gets switched over to C++20 it would probably be better to use ranges instead of pointer fiddling
    auto oamStart = getPointerIntoMemory(OAM_ADDRESS);
    uint16_t sourceStartAddress = value << 8;
    assert(sourceStartAddress <= 0xDF00);

    if (isCartridgeROM(sourceStartAddress) || isCartridgeRAM(sourceStartAddress)) 
    {
        m_cartridge->executeOAMDMATransfer(sourceStartAddress, oamStart);
        return;
    }

    for (size_t i = 0; i < OAM_SIZE; i++)
    {
        // TODO does this get optimized by the compiler? if not should probably use memcpy
        oamStart[i] = m_memory[sourceStartAddress + i];
    }
}
