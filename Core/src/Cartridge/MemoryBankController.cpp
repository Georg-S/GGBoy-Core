#include "Cartridge/MemoryBankController.hpp"

ggb::MemoryBankController::MemoryBankController(std::vector<uint8_t>&& cartridgeData)
	: m_cartridgeData(std::move(cartridgeData))
{
}

ggb::MBCTYPE ggb::MemoryBankController::getMBCType() const
{
	return ggb::getMBCType(m_cartridgeData);
}

void ggb::MemoryBankController::executeOAMDMATransfer(const uint8_t* cartridgeData, uint8_t* oam) const
{
	for (size_t i = 0; i < OAM_SIZE; i++)
	{
		// TODO maybe use memcpy instead
		oam[i] = cartridgeData[i];
	}
}


int ggb::convertRawAddressToBankAddress(uint16_t address, int romBankNumber)
{
	auto startAddress = romBankNumber * ROM_BANK_SIZE;
	auto newAddress = address - 0x4000;
	return startAddress + newAddress;
}

ggb::MBCTYPE ggb::getMBCType(const std::vector<uint8_t>& cartRidgeData)
{
	auto val = cartRidgeData[MBC_TYPE_ADDRESS];
	return MBCTYPE(val);
}
