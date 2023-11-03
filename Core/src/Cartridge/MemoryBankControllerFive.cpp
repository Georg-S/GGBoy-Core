#include "Cartridge/MemoryBankControllerFive.hpp"

#include <cassert>

#include "Utility.hpp"

static constexpr bool isFirstBankAddress(uint16_t address)
{
	return address >= 0x0000 && address <= 0x3FFF;
}

static constexpr bool isRAMEnableAddress(uint16_t address)
{
	return address >= 0x0000 && address <= 0x1FFF;
}

static constexpr bool isLowerROMBankingAddress(uint16_t address)
{
	return address >= 0x2000 && address <= 0x2FFF;
}

static constexpr bool isUpperROMBankingAddress(uint16_t address)
{
	return address >= 0x3000 && address <= 0x3FFF;
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

void ggb::MemoryBankControllerFive::write(uint16_t address, uint8_t value)
{
	if (isRAMEnableAddress(address))
	{
		m_ramEnabled = ((value & 0xF) == 0xA);
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

void ggb::MemoryBankControllerFive::executeDMATransfer(uint16_t startAddress, uint8_t* oam, size_t sizeInBytes) const
{
	// TODO test: this has not been really tested yet, most games seem to not use it, therefore not sure if the code below is correct
	int address = startAddress;
	// TODO handle DMA across the border of bank0 and the following memory bank
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
	const int ramBankCount = getRAMBankCount();
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
