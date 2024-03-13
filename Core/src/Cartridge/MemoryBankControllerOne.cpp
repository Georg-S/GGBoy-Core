#include "Cartridge/MemoryBankControllerOne.hpp"

#include <cassert>

#include "Utility.hpp"

static const std::string filePath = "RAM.bin";

void ggb::MemoryBankControllerOne::write(uint16_t address, uint8_t value)
{
	if (isRAMEnableAddress(address)) 
	{
		m_ramEnabled = ((value & 0xF) == 0xA);
		return;
	}

	if (isLowerROMBankingAddress(address))
	{
		m_lowerROMBank = value & 0x1F;
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

	if (isRAMorUpperROMBankingAddress(address))
	{
		const int bank = value & 0b11;
		if (m_romBankingMode) 
		{
			m_upperROMBank = bank;
			setROMBank();
			return;
		}

		setRAMBank(bank);
		return;
	}

	if (isBankingModeAddress(address)) 
	{
		m_romBankingMode = !isBitSet(value, 0);
		return;
	}
}

uint8_t ggb::MemoryBankControllerOne::read(uint16_t address) const
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

void ggb::MemoryBankControllerOne::executeDMATransfer(uint16_t startAddress, uint8_t* oam, size_t sizeInBytes) const
{
	// TODO test: this has not been really tested yet, most games seem to not use it, therefore not sure if the code below is correct
	int address = startAddress;
	if (isCartridgeRAM(address))
	{
		address = convertRawAddressToRAMBankAddress(address, m_ramBankNumber);
		MemoryBankController::executeDMATransfer(&m_ram[address], oam, sizeInBytes);
		return;
	}

	if (isROMBankAddress(address))
		address = convertRawAddressToBankAddress(address, m_romBankNumber);
	MemoryBankController::executeDMATransfer(&m_cartridgeData[address], oam, sizeInBytes);
}

void ggb::MemoryBankControllerOne::initialize(std::vector<uint8_t>&& cartridgeData)
{
	MemoryBankController::initialize(std::move(cartridgeData));

	if (m_hasRam)
		m_ram = std::vector<uint8_t>(std::max(static_cast<int>(RAM_BANK_SIZE), getRAMSize()), 0);

	// TODO load ram on some rule (if a file exists with the game name or so)
}

void ggb::MemoryBankControllerOne::serialization(Serialization* serialization)
{
	MemoryBankController::serialization(serialization);
	serialization->read_write(m_ramEnabled);
	serialization->read_write(m_romBankingMode);
	serialization->read_write(m_lowerROMBank);
	serialization->read_write(m_upperROMBank);
	serialization->read_write(m_romBankNumber);
	serialization->read_write(m_ramBankNumber);
}

void ggb::MemoryBankControllerOne::setRAMBank(int bank)
{
	if (bank >= getRAMBankCount())
		return;
	m_ramBankNumber = bank;
}

void ggb::MemoryBankControllerOne::setROMBank()
{
	int bank = (m_upperROMBank << 5) | m_lowerROMBank;
	bank &= (getROMBankCount() - 1);
	m_romBankNumber = bank;
}

int ggb::MemoryBankControllerOne::getROMAddress(uint16_t address) const
{
	return convertRawAddressToBankAddress(address, m_romBankNumber);
}

int ggb::MemoryBankControllerOne::getRAMAddress(uint16_t address) const
{
	return convertRawAddressToRAMBankAddress(address, m_ramBankNumber);
}
