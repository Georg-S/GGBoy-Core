#include "BUS.hpp"
#include <cassert>
#include <iostream>

#include "Audio/AudioProcessingUnit.hpp"
#include "Constants.hpp"
#include "Utility.hpp"
#include "Timer.hpp"
#include "PixelProcessingUnit.hpp"

void ggb::BUS::reset()
{
	m_memory = std::vector<uint8_t>(uint16_t{ 0xFFFF } + 1, 0);
	for (auto& elem : m_vram)
		std::fill(std::begin(elem), std::end(elem), 0);
	for (auto& elem : m_wram)
		std::fill(std::begin(elem), std::end(elem), 0);

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
	m_memory[START_DIRECT_MEMORY_ACCESS_ADDRESS] = 0x00; // 0xFF for DMG
	m_memory[BACKGROUND_PALETTE_ADDRESS] = 0xFC;
	m_memory[OBJECT_PALETTE_0_ADDRESS] = 0x00; // Value is actually random
	m_memory[OBJECT_PALETTE_1_ADDRESS] = 0x00; // Value is actually random
	m_memory[LCD_WINDOW_Y_ADDRESS] = 0x00;
	m_memory[LCD_WINDOW_X_ADDRESS] = 0x00;
	m_memory[GBC_SPEED_SWITCH_ADDRESS] = 0x7E;
	m_memory[GBC_VRAM_BANKING_ADDRESS] = 0xFF; // TODO for DMG mode VRAM bank must always be zero
	m_memory[GBC_VRAM_DMA_SOURCE_HIGH_ADDRESS] = 0xFF;
	m_memory[GBC_VRAM_DMA_SOURCE_LOW_ADDRESS] = 0xFF;
	m_memory[GBC_VRAM_DMA_DESTINATION_LOW_ADDRESS] = 0xFF;
	m_memory[GBC_VRAM_DMA_DESTINATION_HIGH_ADDRESS] = 0xFF;
	m_memory[GBC_VRAM_DMA_LENGTH_START_ADDRESS] = 0xFF;
	m_memory[GBC_INFRARED_ADDRESS] = 0xFF;
	m_memory[GBC_BACKGROUND_PALETTE_SPECIFICATION_ADDRESS] = 0xFF;
	m_memory[GBC_BACKGROUND_PALETTE_DATA_ADDRESS] = 0xFF;
	m_memory[GBC_OBJECT_COLOR_PALETTE_SPECIFICATION_ADDRESS] = 0xFF;
	m_memory[GBC_OBJECT_COLOR_PALETTE_DATA_ADDRESS] = 0xFF;
	m_memory[GBC_WRAM_BANKING_ADDRESS] = 0xF8; // 0xFF for DMG
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
	if (isEchoRAMAddress(address))
		address -= (ECHO_RAM_START_ADDRESS - WRAM_START_ADDRESS);
	if (isCartridgeROMAddress(address) || isCartridgeRAMAddress(address))
		return m_cartridge->read(address);
	if (isUnusedMemoryAddress(address))
		return 0xFF; // Reading from unused/invalid memory
	if (address == START_DIRECT_MEMORY_ACCESS_ADDRESS)
		return 0xFF; // DMA Transfer address is write only
	if (isAudioAddress(address))
	{
		auto value = m_audio->read(address);
		if (value)
			return value.value();
	}

	if (address == GBC_VRAM_BANKING_ADDRESS)
	{
		if (getActiveVRAMBank() == 1)
			return 0xFF;
		return 0xFE;
	}
	if (isVRAMAddress(address))
		return m_vram[getActiveVRAMBank()][getVRAMIndexFromAddress(address)];
	if (isWRAMAddress(address))
		return m_wram[getWRAMBank(address)][getWRAMAddress(address)];
	if (address == GBC_BACKGROUND_PALETTE_DATA_ADDRESS || address == GBC_OBJECT_COLOR_PALETTE_DATA_ADDRESS)
		return m_ppu->GBCReadColorRAM(address);

	if (address == GBC_VRAM_DMA_DESTINATION_HIGH_ADDRESS)
		return 0xFF;
	if (address == ggb::GBC_VRAM_DMA_DESTINATION_LOW_ADDRESS)
		return 0xFF;
	if (address == ggb::GBC_VRAM_DMA_SOURCE_HIGH_ADDRESS)
		return 0xFF;
	if (address == ggb::GBC_VRAM_DMA_SOURCE_LOW_ADDRESS)
		return 0xFF;
	return m_memory[address];
}

int8_t ggb::BUS::readSigned(uint16_t address) const
{
	// TODO: Currently this is "implementation defined behavior" with C++ 20 this can easily be made well defined
	return static_cast<int8_t>(read(address));
}

void ggb::BUS::write(uint16_t address, uint8_t value)
{
	if (isEchoRAMAddress(address))
		address -= 0x2000;

	if (isCartridgeROMAddress(address))
	{
		m_cartridge->write(address, value);
		return;
	}

	if (isCartridgeRAMAddress(address))
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

	if (isAudioAddress(address))
	{
		if (m_audio->write(address, value))
			return;
	}
	else if (isUnusedMemoryAddress(address))
	{
		return; // Writing to unused/invalid memory does nothing
	}

	if (address == GBC_VRAM_DMA_LENGTH_START_ADDRESS)
	{
		m_memory[GBC_VRAM_DMA_LENGTH_START_ADDRESS] = value;
		gbcVRAMDirectMemoryAccess();
		return;
	}

	if (address == GBC_SPEED_SWITCH_ADDRESS) 
	{
		if (isBitSet(value, 0))
			toggleGBCDoubleSpeed();
		return;
	}

	if (address == GBC_BACKGROUND_PALETTE_DATA_ADDRESS || address == GBC_OBJECT_COLOR_PALETTE_DATA_ADDRESS) 
	{
		m_ppu->GBCWriteToColorRAM(address, value);
		return;
	}

	if (isVRAMAddress(address))
	{
		m_vram[getActiveVRAMBank()][getVRAMIndexFromAddress(address)] = value;
		return;
	}

	if (isWRAMAddress(address))
	{
		m_wram[getWRAMBank(address)][getWRAMAddress(address)] = value;
		return;
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

uint8_t* ggb::BUS::getVRAMStartPointer(size_t bank)
{
	if (bank > 1)
		return nullptr;
	return &(m_vram[bank][0]);
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
	serialization->read_write(m_wram);
	serialization->read_write(m_vram);
}

bool ggb::BUS::isGBCDoubleSpeedOn() const
{
	return isBitSet(m_memory[GBC_SPEED_SWITCH_ADDRESS], 7);
}

void ggb::BUS::toggleGBCDoubleSpeed()
{
	setBitToValue(m_memory[GBC_SPEED_SWITCH_ADDRESS], 7, !isGBCDoubleSpeedOn());
	m_memory[ENABLED_INTERRUPT_ADDRESS] = 0x00;
	m_memory[INPUT_REGISTER_ADDRESS] = 0x30;
}

void ggb::BUS::directMemoryAccess(uint8_t value)
{
	uint16_t sourceStartAddress = value << 8;
	assert(sourceStartAddress <= 0xDF00);
	directMemoryAccess(sourceStartAddress, OAM_ADDRESS, OAM_MEMORY_SIZE);
}

void ggb::BUS::directMemoryAccess(uint16_t sourceAddress, uint16_t destinationAddress, uint16_t sizeInBytes)
{
	// This is not really the most performant way to perform the DMA but it is the most easy to understand and most robust
	for (uint16_t i = 0; i < sizeInBytes; i++) 
	{
		uint16_t from = sourceAddress + i;
		uint16_t to = destinationAddress + i;
		auto value = read(from);
		write(to, value);
	}
}

// The timings of gbcDMA are currently not correct (maybe correct timings are mandatory for some games?)
void ggb::BUS::gbcVRAMDirectMemoryAccess()
{
	constexpr uint8_t clearLowerFourBitsMask = ~0b1111;
	constexpr uint8_t clearUpperThreeBits = ~0b11100000;
	const auto sourceHigh = m_memory[GBC_VRAM_DMA_SOURCE_HIGH_ADDRESS];
	const auto sourceLow = m_memory[GBC_VRAM_DMA_SOURCE_LOW_ADDRESS] & clearLowerFourBitsMask;
	const auto destinationHigh = m_memory[GBC_VRAM_DMA_DESTINATION_HIGH_ADDRESS] & clearUpperThreeBits;
	const auto destinationLow = m_memory[GBC_VRAM_DMA_DESTINATION_LOW_ADDRESS] & clearLowerFourBitsMask;
	const uint16_t source = combineUpperAndLower(sourceHigh, sourceLow); // Four lower bits are ignored
	const uint16_t destination = (VRAM_START_ADDRESS + combineUpperAndLower(destinationHigh, destinationLow)); // Four lower bits are ignored
	const uint16_t length = static_cast<uint16_t>((m_memory[GBC_VRAM_DMA_LENGTH_START_ADDRESS] & 0b1111111) + 1) * 0x10;

	bool isHblankDMA = isBitSet(m_memory[GBC_VRAM_DMA_LENGTH_START_ADDRESS], 7);
	if (isHblankDMA)
		return; // TODO correctly implement hblank DMA

	assert((getVRAMIndexFromAddress(destination) + length - 1) < std::size(m_vram[0]));

	directMemoryAccess(source, destination, length);
	m_memory[GBC_VRAM_DMA_LENGTH_START_ADDRESS] = 0xFF;
}

int ggb::BUS::getActiveVRAMBank() const
{
	if (isBitSet(m_memory[GBC_VRAM_BANKING_ADDRESS], 0))
		return 1;
	return 0;
}

int ggb::BUS::getWRAMBank(uint16_t address) const
{
	assert(isWRAMAddress(address));
	if (address < WRAM_SWITCHABLE_BANK_START_ADDRESS)
		return 0;
	return std::max(m_memory[GBC_WRAM_BANKING_ADDRESS] & 0b111, 1);
}

uint16_t ggb::BUS::getWRAMAddress(uint16_t address) const
{
	assert(isWRAMAddress(address));
	if (address < WRAM_SWITCHABLE_BANK_START_ADDRESS)
		return address - WRAM_START_ADDRESS;
	return address - WRAM_SWITCHABLE_BANK_START_ADDRESS;
}

int ggb::getVRAMIndexFromAddress(uint16_t address)
{
	assert(isVRAMAddress(address));
	auto result = address - VRAM_START_ADDRESS;
	assert(result < VRAM_BANK_MEMORY_SIZE);
	return result;
}
