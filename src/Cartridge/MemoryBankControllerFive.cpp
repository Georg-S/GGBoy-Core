#include "Cartridge/MemoryBankControllerFive.hpp"

#include <cassert>

#include "Utility.hpp"

static const std::string filePath = "RAM.bin";

void ggb::MemoryBankControllerFive::write(uint16_t address, uint8_t value)
{
	if (isRAMEnableAddress(address))
	{
		m_ramEnabled = shouldEnableRAM(value);
		return;
	}

	if (isLowerROMBankingAddress(address))
	{
		m_lowerROMBank = value;
		setROMBank();
		return;
	}

	if (isUpperROMBankingAddress(address)) 
	{
		m_upperROMBank = value & 0b1;
		setROMBank();
		return;
	}

	if (isCartridgeRAM(address))
	{
		if (!m_hasRam || !m_ramEnabled)
			return;

		m_ram[getRAMAddress(address)] = value;
		return;
	}

	if (isRAMBankingAddress(address))
	{
		setRAMBank(value);
		return;
	}
}

uint8_t ggb::MemoryBankControllerFive::read(uint16_t address) const
{
	if (isFirstROMBankAddress(address))
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

void ggb::MemoryBankControllerFive::initialize(std::vector<uint8_t>&& cartridgeData)
{
	MemoryBankController::initialize(std::move(cartridgeData));

	if (m_hasRam)
		m_ram = std::vector<uint8_t>(getRAMSize(), 0);
	// TODO save and load RAM based on some rule
}

void ggb::MemoryBankControllerFive::serialization(Serialization* serialization)
{
	MemoryBankController::serialization(serialization);
	serialization->read_write(m_ramEnabled);
	serialization->read_write(m_lowerROMBank);
	serialization->read_write(m_upperROMBank);
	serialization->read_write(m_romBankNumber);
	serialization->read_write(m_ramBankNumber);
}

void ggb::MemoryBankControllerFive::setROMBank()
{
	const int count = getROMBankCount() - 1;
	int bank = (m_upperROMBank << 8) | m_lowerROMBank;
	bank &= count;

	m_romBankNumber = bank;
}

void ggb::MemoryBankControllerFive::setRAMBank(int bank)
{
	if (bank <= 0xF)
		m_ramBankNumber = bank;
}

int ggb::MemoryBankControllerFive::getROMAddress(uint16_t address) const
{
	return convertRawAddressToBankAddress(address, m_romBankNumber);
}

int ggb::MemoryBankControllerFive::getRAMAddress(uint16_t address) const
{
	return convertRawAddressToRAMBankAddress(address, m_ramBankNumber);
}
