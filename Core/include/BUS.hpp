#pragma once
#include <cstdint>

#include "Cartridge/Cartridge.hpp"
#include "Serialization.hpp"

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
		uint8_t* getPointerIntoMemory(uint16_t address); // For memory mapped IO only (e.g. the Timer)
		uint8_t* getVRAMStartPointer(size_t bank);
		void requestInterrupt(int interrupt);
		void resetTimerDivider();
		void serialization(Serialization* serialization); // Used for both serialize / deserialize

	private:
		void directMemoryAccess(uint8_t value);
		void directMemoryAccess(uint16_t sourceAddress, uint8_t* destination, size_t sizeInBytes);
		void gbcVRAMDirectMemoryAccess();
		int getActiveVRAMBank() const;

		Cartridge* m_cartridge = nullptr;
		Timer* m_timer = nullptr;
		AudioProcessingUnit* m_audio = nullptr;
		PixelProcessingUnit* m_ppu = nullptr;
		std::vector<uint8_t> m_memory = std::vector<uint8_t>(0xFFFF + 1, 0);
		uint8_t m_vram[GBC_VRAM_BANK_COUNT][VRAM_BANK_MEMORY_SIZE] = {};
	};
	int getVRAMIndexFromAddress(uint16_t address);
}