#include "Cartridge/MemoryBankControllerNone.hpp"

void ggb::MemoryBankControllerNone::write(uint16_t address, uint8_t value)
{
	// We don't have any RAM etc. so do nothing here on purpose
}

uint8_t ggb::MemoryBankControllerNone::read(uint16_t address) const
{
	return m_cartridgeData[address];
}
