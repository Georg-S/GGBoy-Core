#include "Cartridge/MemoryBankController.hpp"

#include <cassert>

// The last three values of this array are from unoffical sources and might be wrong
static constexpr int valueToROMBankCountMapping[] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 72, 80, 96};
// Second value is never used -> INVALID
static constexpr int valueToRAMBankCountMapping[] = {0, 0xFFFFF, 1, 4, 16, 8};

ggb::MemoryBankController::MemoryBankController(std::vector<uint8_t>&& cartridgeData)
	: m_cartridgeData(std::move(cartridgeData))
{
	m_ROMBankCount = getROMBankCount();
	m_RAMBankCount = getRAMBankCount();
}

ggb::MBCTYPE ggb::MemoryBankController::getMBCType() const
{
	return ggb::getMBCType(m_cartridgeData);
}

int ggb::MemoryBankController::getRomSize() const
{
	return getROMBankCount() * ROM_BANK_SIZE;
}

int ggb::MemoryBankController::getROMBankCount() const
{
	const auto val = m_cartridgeData[0x148];
	return valueToROMBankCountMapping[val];
}

int ggb::MemoryBankController::getRAMSize() const
{
	return getRAMBankCount() * RAM_BANK_SIZE;
}

int ggb::MemoryBankController::getRAMBankCount() const
{
	const auto val = m_cartridgeData[0x149];
	assert(val != 1);
	return valueToRAMBankCountMapping[val];
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

int ggb::convertRawAddressToRAMBankAddress(uint16_t address, int ramBankNumber)
{
	auto startAddress = ramBankNumber * RAM_BANK_SIZE;
	auto newAddress = address - 0xA000;
	return startAddress + newAddress;
}

ggb::MBCTYPE ggb::getMBCType(const std::vector<uint8_t>& cartRidgeData)
{
	auto val = cartRidgeData[MBC_TYPE_ADDRESS];
	return MBCTYPE(val);
}
