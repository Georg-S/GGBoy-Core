#include "Cartridge/MemoryBankControllerThree.hpp"


void ggb::MemoryBankControllerThree::write(uint16_t address, uint8_t value)
{
	if (isRAMOrTimerEnableAddress(address)) 
	{
		m_ramAndTimerEnabled = shouldEnableRAM(value);
		return;
	}

	if (isROMBankingAddress(address)) 
	{
		setROMBank(value);
		return;
	}


}

uint8_t ggb::MemoryBankControllerThree::read(uint16_t address) const
{
	if (isFirstROMBankAddress(address))
		return m_cartridgeData[address];
	if (isROMBankAddress(address))
		return m_cartridgeData[convertRawAddressToBankAddress(address, m_romBank)];

	return 0;
}

void ggb::MemoryBankControllerThree::executeDMATransfer(uint16_t startAddress, uint8_t* oam, size_t sizeInBytes) const
{
}

void ggb::MemoryBankControllerThree::initialize(std::vector<uint8_t>&& cartridgeData)
{
}

void ggb::MemoryBankControllerThree::serialization(Serialization* serialization)
{
}

void ggb::MemoryBankControllerThree::setROMBank(uint8_t value)
{
	auto bank = (getROMBankCount() - 1) & value;
	m_romBank = std::max(bank, 1);
}
