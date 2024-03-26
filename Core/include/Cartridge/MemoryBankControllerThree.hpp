#pragma once

#include "MemoryBankController.hpp"

namespace ggb
{
	class MemoryBankControllerThree : public MemoryBankController
	{
	public:
		virtual void write(uint16_t address, uint8_t value) override;
		virtual uint8_t read(uint16_t address) const override;
		virtual void executeDMATransfer(uint16_t startAddress, uint8_t* oam, size_t sizeInBytes) const override;
		void initialize(std::vector<uint8_t>&& cartridgeData) override;
		virtual void serialization(Serialization* serialization) override;

	private:
		void setROMBank(uint8_t value);

		struct RealTimeClock 
		{
			uint8_t seconds = 0;
			uint8_t minutes = 0;
			uint8_t hours = 0;
			uint8_t daysLower = 0;
			uint8_t daysUpperAndFlags = 0;
		};

		bool m_ramAndTimerEnabled = false;
		int m_romBank = 0;
		int m_ramBank = 0;
		uint8_t* m_selectedRegister = nullptr;
		RealTimeClock m_rtc = {};

		static constexpr AddressRange<0x0000, 0x3FFF>  isFirstROMBankAddress = {};
		static constexpr AddressRange<0x4000, 0x7FFF>  isROMBankAddress = {};
		static constexpr AddressRange<0xA000, 0xBFFF>  isCartridgeRAMOrRTCRegister = {}; // RTC = Real time clock

		static constexpr AddressRange<0x0000, 0x1FFF>  isRAMOrTimerEnableAddress = {};
		static constexpr AddressRange<0x2000, 0x3FFF>  isROMBankingAddress = {};
		static constexpr AddressRange<0x4000, 0x5FFF>  isRAMBankingOrRTCRegisterSelectAddress = {};
		static constexpr AddressRange<0x6000, 0x7FFF>  isLatchClockDataAddress = {};
	};
}