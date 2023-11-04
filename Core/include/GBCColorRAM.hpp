#pragma once
#include <array>

#include "BUS.hpp"
#include "Serialization.hpp"
#include "Constants.hpp"

namespace ggb 
{
	class GBCColorRAM
	{
	public:
		GBCColorRAM(uint16_t specificationAddress);
		void reset();
		void setBus(BUS* bus);
		void write(uint8_t value);
		uint8_t read() const;
		void serialization(Serialization* serialization);

	private:
		size_t getRAMAddress() const;
		void incrementAddress();

		uint8_t* m_paletteSpecification = nullptr;
		const uint16_t m_specificationAddress = 0;
		std::array<uint8_t, GBC_COLOR_RAM_MEMORY_SIZE> m_colorRAM = {};
	};
}