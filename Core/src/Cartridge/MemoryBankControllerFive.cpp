#include "Cartridge/MemoryBankControllerFive.hpp"

#include <cassert>

#include "Utility.hpp"

static constexpr bool isRAMEnableAddress(uint16_t address)
{
	return address >= 0x0000 && address <= 0x1FFF;
}

static constexpr bool isROMLSBAddress(uint16_t address)
{
	return address >= 0x2000 && address <= 0x2FFF;
}

static constexpr bool isROMMSBAddress(uint16_t address)
{
	return address >= 0x3000 && address <= 0x3FFF;
}

static constexpr bool isRAMBankingAddress(uint16_t address)
{
	return address >= 0x4000 && address <= 0x5FFF;
}

static constexpr bool isRAMBankAddress(uint16_t address)
{
	return address >= 0x4000 && address <= 0x7FFF;
}

static constexpr bool isBankingModeAddress(uint16_t address)
{
	return address >= 0x6000 && address <= 0x7FFF;
}

constexpr static bool isCartridgeRAM(uint16_t address)
{
	return (address >= 0xA000 && address <= 0xBFFF);
}

ggb::MemoryBankControllerFive::MemoryBankControllerFive(std::vector<uint8_t>&& cartridgeData)
	: MemoryBankController(std::move(cartridgeData))
{
}

void ggb::MemoryBankControllerFive::write(uint16_t address, uint8_t value)
{
	if (isRAMEnableAddress(address)) 
	{
		m_ramEnabled = ((value & 0xF) == 0xA);
		return;
	}

	if (isROMLSBAddress(address))
	{
		constexpr uint8_t eightBitMask = 0;
		m_romBankNumber = m_romBankNumber & eightBitMask;
		m_romBankNumber |= eightBitMask;
		return;
	}
	if (isROMMSBAddress(address))
	{
		uint8_t num = value & 0b1;
		setBitToValue(m_romBankNumber, 8, num == 1);
		return;
	}
	if (isRAMBankingAddress(address))
		int e = 5; // TODO
	if (isBankingModeAddress(address))
		int f = 5; // TODO
	if (isCartridgeRAM(address)) 
	{
		if (m_ramEnabled)
			int g = 3; // TODO
	}
}

uint8_t ggb::MemoryBankControllerFive::read(uint16_t address) const
{
	if (isRAMBankAddress(address))
	{
		const auto newAddress = convertRawAddressToBankAddress(address, m_romBankNumber);
		return m_cartridgeData[newAddress];
	}

	if (isCartridgeRAM(address))
		assert(!"not implemented yet");
	return m_cartridgeData[address];
}

void ggb::MemoryBankControllerFive::executeOAMDMATransfer(uint16_t startAddress, uint8_t* oam) const
{
	auto convertedAddress = convertRawAddressToBankAddress(startAddress, m_romBankNumber);
	MemoryBankController::executeOAMDMATransfer(&m_cartridgeData[convertedAddress], oam);
}

void ggb::MemoryBankControllerFive::serialization(Serialization* serialization)
{
	MemoryBankController::serialization(serialization);
	serialization->read_write(m_ramEnabled);
	serialization->read_write(m_romBankNumber);
}
