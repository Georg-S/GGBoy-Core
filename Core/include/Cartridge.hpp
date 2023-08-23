#pragma once
#include <filesystem>
#include <vector>
#include <memory>

namespace ggb 
{
	enum MBCTYPE
	{
		NO_MBC = 0,
		MBC1 = 1,
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