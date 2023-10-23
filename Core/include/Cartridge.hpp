#pragma once
#include <filesystem>
#include <vector>
#include <memory>
#include "MemoryBankController.hpp"

namespace ggb 
{
	class Cartridge 
	{
	public:
		Cartridge() = default;
		bool load(const std::filesystem::path& romPath);
		void write(uint16_t address, uint8_t value);
		void executeOAMDMATransfer(uint16_t startAddress, uint8_t* oam);
		uint8_t read(uint16_t address) const;

	private:
		std::unique_ptr<MemoryBankController> createMemoryBankController(MBCTYPE mbcType, std::vector<uint8_t>&& cartridgeData) const;

		std::unique_ptr<MemoryBankController> m_memoryBankController;
		MBCTYPE m_mbcType = MC_INVALID;
	};

	std::unique_ptr<Cartridge> loadCartridge(const std::filesystem::path& path);
}