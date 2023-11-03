#include "Cartridge/MemoryBankControllerNone.hpp"

void ggb::MemoryBankControllerNone::write(uint16_t address, uint8_t value)
{
	// We don't have any RAM etc. so do nothing here on purpose
}

uint8_t ggb::MemoryBankControllerNone::read(uint16_t address) const
{
	return m_cartridgeData[address];
}

void ggb::MemoryBankControllerNone::executeDMATransfer(uint16_t startAddress, uint8_t* oam, size_t sizeInBytes) const
{
	auto convertedAddress = startAddress - 0x4000;
	MemoryBankController::executeDMATransfer(&m_cartridgeData[convertedAddress], oam, sizeInBytes);
}
