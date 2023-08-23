#include "Cartridge.hpp"

#include "Logging.hpp"

#include <fstream>

using namespace ggb;

static constexpr uint16_t mbcTypeAddress = 0x0147;

static constexpr bool isRAMEnableAddress(uint16_t address)
{
	return address >= 0x0000 && address <= 0x1FFF;
}

static constexpr bool isROMBankingAddress(uint16_t address)
{
	return address >= 0x2000 && address <= 0x3FFF;
}

static constexpr bool isRAMBankingAddress(uint16_t address)
{
	return address >= 0x4000 && address <= 0x5FFF;
}

static constexpr bool isBankingModeAddress(uint16_t address)
{
	return address >= 0x6000 && address <= 0x7FFF;
}

constexpr static bool isCartridgeRAM(uint16_t address)
{
	return (address >= 0xA000 && address <= 0xBFFF);
}

bool ggb::Cartridge::load(const std::filesystem::path& romPath)
{
	if (!std::filesystem::exists(romPath))
	{
		logError("File not found: " + romPath.string());
	}

	std::ifstream stream(romPath, std::ios::in | std::ios::binary);
	auto bufData = std::vector<char>(std::istreambuf_iterator<char>(stream), {});
	m_cartridgeData.clear();
	m_cartridgeData.reserve(bufData.size());
	for (auto& data : bufData)
		m_cartridgeData.emplace_back(static_cast<uint8_t>(std::move(data)));

	m_mbcType = getMBCType();

	return true;
}

void ggb::Cartridge::write(uint16_t address, uint8_t value)
{
	if (m_mbcType == MBC1)
	{
		if (isRAMEnableAddress(address)) 
			m_ramEnabled = ((value & 0xF) == 0xA);

		if (isROMBankingAddress(address)) 
		{
			auto num = value & 0x1F;
			int d = 3;
		}
		if (isRAMBankingAddress(address))
			int e = 5;
		if (isBankingModeAddress(address))
			int f = 5;

		if (isCartridgeRAM(address))
			int g = 3;
	}
}

uint8_t ggb::Cartridge::read(uint16_t address) const
{
	if (isCartridgeRAM(address))
		int c = 3;
	return m_cartridgeData[address];
}

MBCTYPE ggb::Cartridge::getMBCType() const
{
	auto val = m_cartridgeData[mbcTypeAddress];
	logNumBinary(val);

	return MBCTYPE(val);
}

std::unique_ptr<Cartridge> ggb::loadCartridge(const std::filesystem::path& path)
{
	auto res = std::make_unique<Cartridge>();

	if (res->load(path))
		return res;

	return nullptr;
}
