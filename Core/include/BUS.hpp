#pragma once
#include <cstdint>

#include "Cartridge.hpp"

namespace ggb 
{
	class BUS 
	{
	public:
		BUS() = default;
		void setCartridge(Cartridge* cartridge);
		uint8_t read(uint16_t address) const;
		void write(uint16_t address, uint8_t value);
		void write(uint16_t address, uint16_t value);
		void setBitValue(uint16_t address, int bit, bool bitValue);
		void setBit(uint16_t address, int bit);
		void resetBit(uint16_t address, int bit);
		bool checkBit(uint16_t address, int bit) const;
		void printVRAM();

	private:
		Cartridge* m_cartridge = nullptr;
		std::vector<uint8_t> m_memory = std::vector<uint8_t>(uint16_t(0xFFFF)+1, 0);
	};
}