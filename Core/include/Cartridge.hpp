#pragma once
#include <filesystem>
#include <vector>
#include <memory>

namespace ggb 
{
	class Cartridge 
	{
	public:
		Cartridge() = default;
		bool load(const std::filesystem::path& romPath);
		uint8_t read(uint16_t address);

	private:
		std::vector<std::byte> m_cartridgeData;
	};

	std::unique_ptr<Cartridge> loadCartridge(const std::filesystem::path& path);
}