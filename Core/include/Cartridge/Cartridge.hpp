#pragma once
#include <filesystem>
#include <vector>
#include <memory>
#include "MemoryBankController.hpp"
#include "Serialization.hpp"

namespace ggb 
{
	class Cartridge 
	{
	public:
		Cartridge() = default;
		bool load(const std::filesystem::path& romPath);
		void write(uint16_t address, uint8_t value);
		void executeDMATransfer(uint16_t startAddress, uint8_t* out, size_t sizeInBytes);
		uint8_t read(uint16_t address) const;
		void serialize(Serialization* serialize);
		void deserialize(Serialization* deserialize);
		void saveRAM(const std::filesystem::path& outputPath);
		void loadRAM(const std::filesystem::path& inputPath);

	private:
		void serialization(Serialization* serialize);
		std::unique_ptr<MemoryBankController> createMemoryBankController(MBCTYPE mbcType) const;

		std::unique_ptr<MemoryBankController> m_memoryBankController;
		MBCTYPE m_mbcType = MC_INVALID;
	};

	std::unique_ptr<Cartridge> loadCartridge(const std::filesystem::path& path);
}