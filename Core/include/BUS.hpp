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
		uint8_t read(uint16_t address);

	private:
		Cartridge* m_cartridge;
		std::vector<uint8_t> m_memory = std::vector<uint8_t>(uint16_t(0xFFFF), 0);
	};
}