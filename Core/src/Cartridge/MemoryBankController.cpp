#include "Cartridge/MemoryBankController.hpp"

#include <cassert>
#include <fstream>

#include "Serialization.hpp"
#include "Utility.hpp"

// The last three values of this array are from unoffical sources and might be wrong
static constexpr int valueToROMBankCountMapping[] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 72, 80, 96};
// Second value is never used -> INVALID
static constexpr int valueToRAMBankCountMapping[] = {0, 0xFFFFF, 1, 4, 16, 8};

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

void ggb::MemoryBankController::loadRAM(const std::filesystem::path& path)
{
	if (!m_hasRam)
		return;

	Deserialize deserialize = Deserialize(path);
	deserialize.read_write(m_ram);
}

void ggb::MemoryBankController::saveRAM(const std::filesystem::path& path)
{
	if (!m_hasRam)
		return;

	Serialize serialize = Serialize(path);
	serialize.read_write(m_ram);
}

void ggb::MemoryBankController::initialize(std::vector<uint8_t>&& cartridgeData)
{
	m_cartridgeData = std::move(cartridgeData);
	m_ROMBankCount = getROMBankCount();
	m_RAMBankCount = getRAMBankCount();
	m_hasRam = m_ROMBankCount > 0;
}

void ggb::MemoryBankController::serialization(Serialization* serialization)
{
	serialization->read_write(m_cartridgeData);
	serialization->read_write(m_ram);
	serialization->read_write(m_hasRam);
	serialization->read_write(m_ROMBankCount);
	serialization->read_write(m_RAMBankCount);
}

void ggb::MemoryBankController::executeDMATransfer(const uint8_t* cartridgeData, uint8_t* oam, size_t sizeInBytes) const
{
	for (size_t i = 0; i < sizeInBytes; i++)
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
