#pragma once

#include "MemoryBankController.hpp"

namespace ggb
{
	class MemoryBankControllerOne : public MemoryBankController
	{
	public:
		void write(uint16_t address, uint8_t value) override;
		uint8_t read(uint16_t address) const override;
		void initialize(std::vector<uint8_t>&& cartridgeData) override;
		virtual void serialization(Serialization* serialization) override;

	private:
		void setROMBank();
		void setRAMBank(int bank);
		int getROMAddress(uint16_t address) const;
		int getRAMAddress(uint16_t address) const;

		bool m_ramEnabled = false;
		bool m_romBankingMode = true;
		int m_lowerROMBank = 1;
		int m_upperROMBank = 0;
		int m_romBankNumber = 1;
		int m_ramBankNumber = 0;

		static constexpr AddressRange<0x0000, 0x3FFF>  isFirstROMBankAddress = {};
		static constexpr AddressRange<0x0000, 0x1FFF>  isRAMEnableAddress = {};
		static constexpr AddressRange<0x2000, 0x3FFF>  isLowerROMBankingAddress = {};
		static constexpr AddressRange<0x4000, 0x5FFF>  isRAMorUpperROMBankingAddress = {};
		static constexpr AddressRange<0x4000, 0x7FFF>  isROMBankAddress = {};
		static constexpr AddressRange<0x6000, 0x7FFF>  isBankingModeAddress = {};
		static constexpr AddressRange<0xA000, 0xBFFF>  isCartridgeRAM = {};
	};
}