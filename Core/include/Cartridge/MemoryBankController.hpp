#pragma once
#include <cstdint>
#include <vector>
#include <filesystem>

#include "Constants.hpp"
#include "Serialization.hpp"

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

		MC_INVALID, // Should be the last entry
	};

	int convertRawAddressToBankAddress(uint16_t address, int romBankNumber);
	int convertRawAddressToRAMBankAddress(uint16_t address, int ramBankNumber);
	MBCTYPE getMBCType(const std::vector<uint8_t>& cartRidgeData);

	class MemoryBankController // Often abbreviated as MBC
	{
	public:
		virtual ~MemoryBankController() = default;
		virtual void write(uint16_t address, uint8_t value) = 0;
		virtual uint8_t read(uint16_t address) const = 0;
		virtual void executeDMATransfer(uint16_t startAddress, uint8_t* oam, size_t sizeInBytes) const = 0;
		MBCTYPE getMBCType() const;
		int getRomSize() const;
		int getROMBankCount() const;
		int getRAMSize() const;
		int getRAMBankCount() const;
		void loadRAM(const std::filesystem::path& path); // Does nothing if MBC has no RAM
		void saveRAM(const std::filesystem::path& path) const; // Does nothing if MBC has no RAM
		virtual void initialize(std::vector<uint8_t>&& cartridgeData);
		virtual void serialization(Serialization* serialization);

	protected:
		void executeDMATransfer(const uint8_t* cartridgeData, uint8_t* oam, size_t sizeInBytes) const;

		std::vector<uint8_t> m_cartridgeData;
		std::vector<uint8_t> m_ram;
		bool m_hasRam = false;
		int m_ROMBankCount = 0;
		int m_RAMBankCount = 0;
	};
}