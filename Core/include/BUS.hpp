#pragma once
#include <cstdint>

#include "Cartridge/Cartridge.hpp"

namespace ggb
{
	class Timer;
	class AudioProcessingUnit;
	class PixelProcessingUnit;

	class BUS
	{
	public:
		BUS() = default;
		void reset();
		void setCartridge(Cartridge* cartridge);
		void setTimer(Timer* cartridge);
		void setAudio(AudioProcessingUnit* audio);
		void setPixelProcessingUnit(PixelProcessingUnit* ppu);
		uint8_t read(uint16_t address) const;
		int8_t readSigned(uint16_t address) const;
		void write(uint16_t address, uint8_t value);
		void write(uint16_t address, uint16_t value);
		void setBitValue(uint16_t address, int bit, bool bitValue);
		void setBit(uint16_t address, int bit);
		void resetBit(uint16_t address, int bit);
		bool checkBit(uint16_t address, int bit) const;
		uint8_t* getPointerIntoMemory(uint16_t address); // For memory mapped IO only (e.g. the Timer)
		void requestInterrupt(int interrupt);
		void resetTimerDivider();

	private:
		void directMemoryAccess(uint8_t value);

		Cartridge* m_cartridge = nullptr;
		Timer* m_timer = nullptr;
		AudioProcessingUnit* m_audio = nullptr;
		PixelProcessingUnit* m_ppu = nullptr;
		std::vector<uint8_t> m_memory = std::vector<uint8_t>(0xFFFF + 1, 0);
	};
}