#pragma once
#include <filesystem>
#include <vector>
#include <memory>

namespace ggb 
{
	enum MBCTYPE
	{
		NO_MBC = 0x00,
		MBC1 = 0x01,
		MBC1_RAM = 0x02,
		MBC1_RAM_BATTERY = 0x03,
		MBC2 = 0x05,
		MBC2_BATTERY = 0x06,
		MBC5 = 0x19,
		MBC5_RAM = 0x1A,
		MC5_RAM_BATTERY = 0x1B,
		MC5_RUMBLE = 0x1C,
		MC5_RUMBLE_RAM = 0x1D,
		MC5_RUMBLE_RAM_BATTERY = 0x1E,
	};

	class Cartridge 
	{
	public:
		Cartridge() = default;
		bool load(const std::filesystem::path& romPath);
		void write(uint16_t address, uint8_t value);
		uint8_t read(uint16_t address) const;
		MBCTYPE getMBCType() const;

	private:
		std::vector<uint8_t> m_cartridgeData;
		MBCTYPE m_mbcType;
		bool m_ramEnabled = false;
		int romBankNumber = 1;
	};

	std::unique_ptr<Cartridge> loadCartridge(const std::filesystem::path& path);
}