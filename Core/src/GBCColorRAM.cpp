#include "GBCColorRAM.hpp"

#include "Utility.hpp"

ggb::GBCColorRAM::GBCColorRAM(uint16_t specificationAddress)
	: m_specificationAddress(specificationAddress)
{
}

void ggb::GBCColorRAM::reset()
{
	m_colorPalettes = {};
	m_colorRAM = {};
}

void ggb::GBCColorRAM::setBus(BUS* bus)
{
	m_paletteSpecification = bus->getPointerIntoMemory(m_specificationAddress);
}

void ggb::GBCColorRAM::write(uint8_t value)
{
	m_colorRAM[getRAMAddress()] = value;
	if (isBitSet(*m_paletteSpecification, 7))
		incrementAddress();
}

uint8_t ggb::GBCColorRAM::read() const
{
	return m_colorRAM[getRAMAddress()];
}

void ggb::GBCColorRAM::serialization(Serialization* serialization)
{
	serialization->read_write(m_colorRAM);
	serialization->read_write(m_colorPalettes);
}

const ggb::ColorPalette& ggb::GBCColorRAM::getColorPalette(size_t index) const
{
	return m_colorPalettes[index];
}

static uint8_t  convertGameboyColorRGBTo24BitRGB(uint16_t number) 
{
	static constexpr auto gbcSingleColorValueRange = 31; // 5 bits -> 2^5 = 32
	static constexpr auto normalRGBSingleColorValueRange = 255; // 8 bits -> 2 ^8 = 256
	return number * normalRGBSingleColorValueRange / gbcSingleColorValueRange;
}

void ggb::GBCColorRAM::updateColorPalettes()
{
	static constexpr uint16_t colorBitMask = 0b11111;
	size_t ramIndex = 0;

	for (auto& palette : m_colorPalettes) 
	{
		for (auto& color : palette.m_color) 
		{
			const uint16_t combined = (static_cast<uint16_t>(m_colorRAM[ramIndex + 1]) << 8) | m_colorRAM[ramIndex];
			color.r = convertGameboyColorRGBTo24BitRGB(combined & colorBitMask);
			color.g = convertGameboyColorRGBTo24BitRGB((combined >> 5) & colorBitMask);
			color.b = convertGameboyColorRGBTo24BitRGB((combined >> 10) & colorBitMask);
			ramIndex += 2;
		}
	}
}

size_t ggb::GBCColorRAM::getRAMAddress() const
{
	return *m_paletteSpecification & 0b111111;
}

void ggb::GBCColorRAM::incrementAddress()
{
	size_t address = getRAMAddress() + 1;
	address %= GBC_COLOR_RAM_MEMORY_SIZE;
	*m_paletteSpecification = *m_paletteSpecification & ~0b111111;
	*m_paletteSpecification |= address;
}
