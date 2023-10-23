#pragma once
#include <cstdint>
#include <vector>

#include "Constants.hpp"

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

	class MemoryBankController
	{
	public:
		MemoryBankController(std::vector<uint8_t>&& cartridgeData)
			: m_cartridgeData(std::move(cartridgeData))
		{
		};
		virtual ~MemoryBankController() = default;
		virtual void write(uint16_t address, uint8_t value) = 0;
		virtual uint8_t read(uint16_t address) const = 0;
		virtual void executeOAMDMATransfer(uint16_t startAddress, uint8_t* oam) const = 0;

		static MBCTYPE getMBCType(const std::vector<uint8_t>& cartRidgeData)
		{
			auto val = cartRidgeData[MBC_TYPE_ADDRESS];
			return MBCTYPE(val);
		}

		MBCTYPE getMBCType() const
		{
			auto val = m_cartridgeData[MBC_TYPE_ADDRESS];
			return MBCTYPE(val);
		}

		static uint16_t convertRawAddressToBankAddress(uint16_t address, int romBankNumber) 
		{
			auto startAddress = romBankNumber * ROM_BANK_SIZE;
			auto newAddress = address - 0x4000;
			return startAddress + newAddress;
		}

	protected:
		std::vector<uint8_t> m_cartridgeData;
	};
}