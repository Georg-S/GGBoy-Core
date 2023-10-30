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

static const std::string filePath = "RAM.bin";

ggb::MemoryBankControllerOne::MemoryBankControllerOne(std::vector<uint8_t>&& cartridgeData)
	: MemoryBankController(std::move(cartridgeData))
{
	if (m_hasRam)
		m_ram = std::vector<uint8_t>(getRAMSize(), 0);

	if (std::filesystem::exists(filePath)) 
	{
		// TODO use a better path
		loadRAM(filePath);
		assert(m_ram.size() == getRAMSize());
	}
}

void ggb::MemoryBankControllerOne::write(uint16_t address, uint8_t value)
{
	if (isRAMEnableAddress(address)) 
	{
		const bool previous = m_ramEnabled;
		m_ramEnabled = ((value & 0xF) == 0xA);
		if (!m_ramEnabled && (m_ramEnabled != previous))
			saveRAM(filePath);
		return;
	}

	if (isROMBankingAddress(address))
	{
		auto num = value & 0x1F;
		m_romBankNumber = std::max(num, 1);
		// TODO mask bits to max count of rom bank
		return;
	}

	if (isCartridgeRAM(address)) 
	{
		if (!m_hasRam || !m_ramEnabled)
			return;

		m_ram[getRAMAddress(address)] = value;
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
		return m_cartridgeData[getROMAddress(address)];

	if (isCartridgeRAM(address)) 
	{
		if (!m_hasRam || !m_ramEnabled)
			return 0xFF;

		return m_ram[getRAMAddress(address)];
	}

	assert(!"Invalid");
	return m_cartridgeData[address];
}

void ggb::MemoryBankControllerOne::executeOAMDMATransfer(uint16_t startAddress, uint8_t* oam) const
{
	auto convertedAddress = convertRawAddressToBankAddress(startAddress, m_romBankNumber);
	MemoryBankController::executeOAMDMATransfer(&m_cartridgeData[convertedAddress], oam);
}

void ggb::MemoryBankControllerOne::serialization(Serialization* serialization)
{
	MemoryBankController::serialization(serialization);
	serialization->read_write(m_ramEnabled);
	serialization->read_write(m_romBankNumber);
	serialization->read_write(m_ramBankNumber);
}

int ggb::MemoryBankControllerOne::getROMAddress(uint16_t address) const
{
	return convertRawAddressToBankAddress(address, m_romBankNumber);
}

int ggb::MemoryBankControllerOne::getRAMAddress(uint16_t address) const
{
	return convertRawAddressToRAMBankAddress(address, m_ramBankNumber);
}
