#include "FrameBuffer.hpp"

#include "Utility.hpp"

using namespace ggb;

ggb::FrameBuffer::FrameBuffer(BUS* bus)
	: m_bus(bus)
{
	for (int i = 0; i < 256; i++)
	{
		m_buffer.emplace_back(std::vector<RGB>(256, { 0,0,0 }));
	}
}

void ggb::FrameBuffer::drawWholeScreen()
{
}

ggb::Tile::Tile(BUS* bus, int tileIndex, const ColorPalette& palette)
{
	readTileDataFromBus(bus, tileIndex, palette);
}

static ggb::RGB getRGBFromNumAndPalette(uint8_t num, const ColorPalette& palette)
{
	return convertGBColorToRGB(palette.m_color[num]);
}

void ggb::Tile::readTileDataFromBus(BUS* bus, int tileIndex, const ColorPalette& palette)
{
	constexpr uint16_t tileDataStartAddress = 0x8000;
	constexpr uint16_t tileSize = 16;
	uint16_t address = tileDataStartAddress + (tileIndex * tileSize);
	const uint16_t endAddress = tileDataStartAddress + ((tileIndex + 1) * tileSize);
	int index = 0;

	m_rawData.reserve(8);
	while (address < endAddress)
	{
		auto low = bus->read(address++);
		auto high = bus->read(address++);
		if (low != 0 || high != 0)
			int b = 3;

		m_rawData.emplace_back(TileRawData{ low, high });
	}

	m_data.reserve(8);
	for (const auto& rawData : m_rawData)
	{
		m_data.emplace_back();
		m_data.back().reserve(8);
		for (int x = 7; x >= 0; x--)
		{
			auto msb = isBitSet(rawData.msBits, x);
			auto lsb = isBitSet(rawData.msBits, x);
			auto num = getNumberFromBits(lsb, msb);

			auto test = getRGBFromNumAndPalette(num, palette);
			m_data.back().emplace_back(test);
		}
	}
}

RGB ggb::convertGBColorToRGB(GBColor color)
{
	switch (color)
	{
	case ggb::GBColor::WHITE:		return { 0,0,0 };
	case ggb::GBColor::LIGHT_GREY:	return { 85, 85, 85 };
	case ggb::GBColor::DARK_GREY:	return { 170, 170, 170 };
	case ggb::GBColor::BLACK:		return { 255, 255, 255 };
	default:
		assert(!"Invalid value entered");
		return {};
	}
}
