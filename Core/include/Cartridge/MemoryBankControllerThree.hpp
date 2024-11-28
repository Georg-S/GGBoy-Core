#pragma once

#include "MemoryBankController.hpp"

namespace ggb
{
	class MemoryBankControllerThree : public MemoryBankController
	{
	public:
		virtual void write(uint16_t address, uint8_t value) override;
		virtual uint8_t read(uint16_t address) const override;
		void initialize(std::vector<uint8_t>&& cartridgeData) override;
		virtual void serialization(Serialization* serialization) override;

	private:
		void setROMBank(uint8_t value);

		class RealTimeClock 
		{
		public:
			void serialize(Serialization* serialization);
			void selectRegister(uint8_t value);
			void writeToSelectedRegister(uint8_t value);
			uint8_t getSelectedRegisterValue();
			bool registerSelected() const;
			void resetRegisterSelection();
			void handleLatching(uint8_t value);

		private:
			struct Registers
			{
				uint8_t m_seconds = 0;
				uint8_t m_minutes = 0;
				uint8_t m_hours = 0;
				uint8_t m_daysLower = 0;
				uint8_t m_daysUpperAndFlags = 0;
			};

			enum class RegisterType { NONE, SECONDS, MINUTES, HOURS, DAYSLOWER, DAYSUPPERFLAGS };
			uint8_t& getRegister(Registers& registers, RegisterType selectedRegister) const;
			//uint8_t& getRegister(RegisterType selectedRegister);
			RegisterType getRegisterFromValue(uint8_t value);
			bool isStopped() const;
			void update();

			long long m_lastTimeStamp = 0;
			long long m_subSeconds = 0;
			Registers m_latchedRegisters;
			Registers m_registers;
			bool m_isLatched = false;
			uint8_t m_lastLatchValue = 0x1;
			RegisterType m_selectedRegister = RegisterType::NONE;
		};

		bool m_ramAndTimerEnabled = false;
		int m_romBank = 0;
		int m_ramBank = 0;
		// TODO not the best design decision to make this mutable
		// maybe change it so that update is called from the emulator
		mutable RealTimeClock m_rtc = {};

		static constexpr AddressRange<0x0000, 0x3FFF>  isFirstROMBankAddress = {};
		static constexpr AddressRange<0x4000, 0x7FFF>  isROMBankAddress = {};
		static constexpr AddressRange<0xA000, 0xBFFF>  isCartridgeRAMOrRTCRegister = {}; // RTC = Real time clock

		static constexpr AddressRange<0x0000, 0x1FFF>  isRAMOrTimerEnableAddress = {};
		static constexpr AddressRange<0x2000, 0x3FFF>  isROMBankingAddress = {};
		static constexpr AddressRange<0x4000, 0x5FFF>  isRAMBankingOrRTCRegisterSelectAddress = {};
		static constexpr AddressRange<0x6000, 0x7FFF>  isLatchClockDataAddress = {};
	};
}