#include "BUS.hpp"
#include <cassert>
#include <iostream>

#include "Audio/AudioProcessingUnit.hpp"
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

constexpr static bool isAudioMemory(uint16_t address) 
{
    return (address >= 0xFF10 && address <= 0xFF3F);
}

void ggb::BUS::reset()
{
    m_memory = std::vector<uint8_t>(uint16_t(0xFFFF) + 1, 0);

    m_memory[INPUT_REGISTER_ADDRESS] = 0xCF;
    m_memory[SERIAL_TRANSFER_ADDRESS] = 0x00;
    m_memory[SERIAL_TRANSFER_CONTROL_ADDRESS] = 0x7E;
    m_memory[TIMER_DIVIDER_REGISTER_ADDRESS] = 0x18;
    m_memory[TIMER_COUNTER_ADDRESS] = 0x00;
    m_memory[TIMER_MODULO_ADDRESS] = 0x00;
    m_memory[TIMER_CONTROL_ADDRESS] = 0xF8;
    m_memory[INTERRUPT_REQUEST_ADDRESS] = 0xE1;
    m_memory[AUDIO_CHANNEL_1_FREQUENCY_SWEEP_ADDRESS] = 0x80;
    m_memory[AUDIO_CHANNEL_1_LENGTH_DUTY_ADDRESS] = 0xBF;
    m_memory[AUDIO_CHANNEL_1_VOLUME_ENVELOPE_ADDRESS] = 0xF3;
    m_memory[AUDIO_CHANNEL_1_PERIOD_LOW_ADDRESS] = 0xFF;
    m_memory[AUDIO_CHANNEL_1_PERIOD_HIGH_CONTROL_ADDRESS] = 0xBF;
    m_memory[AUDIO_CHANNEL_2_LENGTH_DUTY_ADDRESS] = 0x3F;
    m_memory[AUDIO_CHANNEL_2_VOLUME_ENVELOPE_ADDRESS] = 0x00;
    m_memory[AUDIO_CHANNEL_2_PERIOD_LOW_ADDRESS] = 0xFF;
    m_memory[AUDIO_CHANNEL_2_PERIOD_HIGH_CONTROL_ADDRESS] = 0xBF;
    m_memory[AUDIO_CHANNEL_3_DAC_ENABLE_ADDRESS] = 0x7F;
    m_memory[AUDIO_CHANNEL_3_LENGTH_TIMER_ADDRESS] = 0xFF;
    m_memory[AUDIO_CHANNEL_3_OUTPUT_LEVEL_ADDRESS] = 0x9F;
    m_memory[AUDIO_CHANNEL_3_PERIOD_LOW_ADDRESS] = 0xFF;
    m_memory[AUDIO_CHANNEL_3_PERIOD_HIGH_CONTROL_ADDRESS] = 0xBF;
    m_memory[AUDIO_CHANNEL_4_LENGTH_TIMER_ADDRESS] = 0xFF;
    m_memory[AUDIO_CHANNEL_4_VOLUME_ENVELOPE_ADDRESS] = 0x00;
    m_memory[AUDIO_CHANNEL_4_FREQUENCY_RANDOMNESS_ADDRESS] = 0x00;
    m_memory[AUDIO_CHANNEL_4_CONTROL_ADDRESS] = 0xBF;
    m_memory[AUDIO_MASTER_VOLUME_VIN_PANNING_ADDRESS] = 0x77;
    m_memory[AUDIO_SOUND_PANNING_ADDRESS] = 0xF3;
    m_memory[AUDIO_MASTER_CONTROL_ADDRESS] = 0xF1;
    m_memory[LCD_CONTROL_REGISTER_ADDRESS] = 0x91;
    m_memory[LCD_STATUS_REGISTER_ADDRESS] = 0x81;
    m_memory[LCD_VIEWPORT_Y_ADDRESS] = 0x00;
    m_memory[LCD_VIEWPORT_X_ADDRESS] = 0x00;
    m_memory[LCD_Y_COORDINATE_ADDRESS] = 0x91;
    m_memory[LCD_Y_COMPARE_ADDRESS] = 0x00;
    m_memory[START_DIRECT_MEMORY_ACCESS_ADDRESS] = 0xFF; // 0x00 for GBC
    m_memory[BACKGROUND_PALETTE_ADDRESS] = 0xFC;
    m_memory[OBJECT_PALETTE_0_ADDRESS] = 0x00; // Value is actually random
    m_memory[OBJECT_PALETTE_1_ADDRESS] = 0x00; // Value is actually random
    m_memory[LCD_WINDOW_Y_ADDRESS] = 0x00; 
    m_memory[LCD_WINDOW_X_ADDRESS] = 0x00; 
    m_memory[GBC_SPEED_SWITCH_ADDRESS] = 0xFF;
    m_memory[GBC_VRAM_BANKING_ADDRESS] = 0xFF;
    m_memory[GBC_VRAM_DMA_SOURCHE_HIGH_ADDRESS] = 0xFF;
    m_memory[GBC_VRAM_DMA_SOURCHE_LOW_ADDRESS] = 0xFF;
    m_memory[GBC_VRAM_DMA_DESTINATION_LOW_ADDRESS] = 0xFF;
    m_memory[GBC_VRAM_DMA_DESTINATION_HIGH_ADDRESS] = 0xFF;
    m_memory[GBC_VRAM_DMA_LENGTH_START_ADDRESS] = 0xFF;
    m_memory[GBC_INFRARED_ADDRESS] = 0xFF;
    m_memory[GBC_BACKGROUND_PALETTE_SPECIFICATION_ADDRESS] = 0xFF;
    m_memory[GBC_BACKGROUND_PALETTE_DATA_ADDRESS] = 0xFF;
    m_memory[GBC_OBJECT_COLOR_PALETTE_SPECIFICATION_ADDRESS] = 0xFF;
    m_memory[GBC_OBJECT_COLOR_PALETTE_DATA_ADDRESS] = 0xFF;
    m_memory[GBC_WRAM_BANKING_ADDRESS] = 0xFF;
    m_memory[ENABLED_INTERRUPT_ADDRESS] = 0x00;
}

void ggb::BUS::setCartridge(Cartridge* cartridge)
{
    m_cartridge = cartridge;
}

void ggb::BUS::setTimer(Timer* timer)
{
    m_timer = timer;
}

void ggb::BUS::setAudio(AudioProcessingUnit* audio)
{
    m_audio = audio;
}

void ggb::BUS::setPixelProcessingUnit(PixelProcessingUnit* ppu)
{
    m_ppu = ppu;
}

uint8_t ggb::BUS::read(uint16_t address) const
{
    if (isEchoMemory(address))
        address -= 0x2000;
    if (isCartridgeROM(address) || isCartridgeRAM(address))
        return m_cartridge->read(address);
    if (isUnusedMemory(address))
        return 0xFF; // Reading from unused/invalid memory
    if (address == 0xFF46) // DMA Transfer address, write only
        return 0xFF;
    if (isAudioMemory(address))
    {
        auto value = m_audio->read(address);
        if (value)
            return value.value();
    }

    return m_memory[address];
}

int8_t ggb::BUS::readSigned(uint16_t address) const
{
    // TODO: Currently this is "implementation defined behavior" with C++ 20 this can easily be made well defined
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
    {
        directMemoryAccess(value);
        return;
    }

    if (isAudioMemory(address)) 
    {
        if (m_audio->write(address, value))
            return;
    }
    else if (isUnusedMemory(address)) 
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

uint8_t* ggb::BUS::getPointerIntoMemory(uint16_t address)
{
    assert(address >= 0xF000);
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

void ggb::BUS::serialization(Serialization* serialization)
{
    serialization->read_write(m_memory);
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
