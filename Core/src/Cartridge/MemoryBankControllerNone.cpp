#include "Cartridge/MemoryBankControllerNone.hpp"

ggb::MemoryBankControllerNone::MemoryBankControllerNone(std::vector<uint8_t>&& cartridgeData)
	: MemoryBankController(std::move(cartridgeData))
{
}

void ggb::MemoryBankControllerNone::write(uint16_t address, uint8_t value)
{
	// We don't have any RAM etc. so do nothing here on purpose
}

uint8_t ggb::MemoryBankControllerNone::read(uint16_t address) const
{
	return m_cartridgeData[address];
}

void ggb::MemoryBankControllerNone::executeOAMDMATransfer(uint16_t startAddress, uint8_t* oam) const
{
	auto convertedAddress = startAddress - 0x4000;
	MemoryBankController::executeOAMDMATransfer(&m_cartridgeData[convertedAddress], oam);
}
