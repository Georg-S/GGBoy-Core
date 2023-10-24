#include "Cartridge/MemoryBankControllerOne.hpp"

#include <cassert>

static constexpr bool isFirstBankAddress(uint16_t address) 
{
	return address >= 0x0000 && address <= 0x3FFF;
}

static constexpr bool isRAMEnableAddress(uint16_t address)
{
	return address >= 0x0000 && address <= 0x1FFF;
}

static constexpr bool isROMBankingAddress(uint16_t address)
{
	return address >= 0x2000 && address <= 0x3FFF;
}

static constexpr bool isRAMBankingAddress(uint16_t address)
{
	return address >= 0x4000 && address <= 0x5FFF;
}

static constexpr bool isROMBankAddress(uint16_t address)
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

ggb::MemoryBankControllerOne::MemoryBankControllerOne(std::vector<uint8_t>&& cartridgeData, bool hasRam)
	: MemoryBankController(std::move(cartridgeData))
	, m_hasRam(hasRam)
{
	if (m_hasRam)
		m_ram = std::vector<uint8_t>(32768, 0);
}

void ggb::MemoryBankControllerOne::write(uint16_t address, uint8_t value)
{
	if (isRAMEnableAddress(address)) 
	{
		m_ramEnabled = ((value & 0xF) == 0xA);
		return;
	}

	if (isROMBankingAddress(address))
	{
		auto num = value & 0x1F;
		m_romBankNumber = std::max(num, 1);
		return;
	}
	if (isCartridgeRAM(address)) 
	{
		if (!m_hasRam || !m_ramEnabled)
			return;

		m_ram[address - 0xA000] = value;
		return;
	}
	assert(!"");
	if (isRAMBankingAddress(address))
		int e = 5; // TODO
	if (isBankingModeAddress(address)) 
	{
		assert(!"Not implemented");
	}

}

uint8_t ggb::MemoryBankControllerOne::read(uint16_t address) const
{
	if (isFirstBankAddress(address))
		return m_cartridgeData[address];

	if (isROMBankAddress(address))
	{
		const auto newAddress = convertRawAddressToBankAddress(address, m_romBankNumber);
		return m_cartridgeData[newAddress];
	}

	if (isCartridgeRAM(address)) 
	{
		if (!m_hasRam)
			return 0xFF;
		return m_ram[address - 0xA000];
	}

	assert(!"Invalid");
	return m_cartridgeData[address];
}

void ggb::MemoryBankControllerOne::executeOAMDMATransfer(uint16_t startAddress, uint8_t* oam) const
{
	auto convertedAddress = convertRawAddressToBankAddress(startAddress, m_romBankNumber);
	MemoryBankController::executeOAMDMATransfer(&m_cartridgeData[convertedAddress], oam);
}
