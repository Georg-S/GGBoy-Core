#include "FrameBuffer.hpp"

#include "Utility.hpp"

using namespace ggb;

ggb::FrameBuffer::FrameBuffer(BUS* bus, int xSize, int ySize)
	: m_xSize(xSize) 
	, m_ySize(ySize)
	, m_bus(bus)
{
	for (size_t x = 0; x < m_xSize; x++) 
		m_buffer.emplace_back(std::vector<RGBA>(ySize, {0,0,0}));
}

void ggb::FrameBuffer::setPixel(int x, int y, const RGBA& pixelValue)
{
	m_buffer[x][y] = pixelValue;
}

RGBA ggb::FrameBuffer::getPixel(int x, int y) const
{
	return m_buffer[x][y];
}

void ggb::FrameBuffer::forEachPixel(const std::function<void(int x, int y, const RGBA& rgb)>& func) const
{
	for (int x = 0; x < m_xSize; x++) 
	{
		for (int y = 0; y < m_ySize; y++)
		{
			func(x, y, m_buffer[x][y]);
		}
	}
}


static ggb::RGBA getRGBFromNumAndPalette(uint8_t num, const ColorPalette& palette)
{
	return convertGBColorToRGB(palette.m_color[num]);
}

ggb::Tile::Tile()
{
	m_rawData = std::vector<TileRawData>(8, {0,0});
	for (size_t i = 0; i < 8; i++)
		m_data.emplace_back(std::vector<RGBA>(8, {0,0,0}));
}

// TODO differentiate between background/window and sprites/obj
RGBA ggb::convertGBColorToRGB(GBColor color)
{
	switch (color)
	{
	case ggb::GBColor::BLACK:		return { 0,0,0,0 };
	case ggb::GBColor::DARK_GREY:	return { 85, 85, 85, 0 };
	case ggb::GBColor::LIGHT_GREY:	return { 170, 170, 170, 0 };
	case ggb::GBColor::WHITE:		return { 255, 255, 255, 0 };
	default:
		assert(!"Invalid value entered");
		return {};
	}
}

void ggb::overWriteTileData(BUS* bus, uint16_t tileIndex, const ColorPalette& palette, Tile* outTile)
{
	constexpr uint16_t tileDataStartAddress = 0x8000;
	constexpr uint16_t tileSize = 16;
	uint16_t address = tileDataStartAddress + (tileIndex * tileSize);
	const uint16_t endAddress = tileDataStartAddress + ((tileIndex + 1) * tileSize);

	for (uint8_t y = 0; y < 8; y++)
		getTileRowRGBData(bus, address,y, palette, outTile->m_data[y]);
}

Tile ggb::getTileByIndex(BUS* bus, uint16_t tileIndex, const ColorPalette& palette)
{
	Tile tile;
	overWriteTileData(bus, tileIndex, palette, &tile);
	return tile;
}

void ggb::getTileRowRGBData(BUS* bus, uint16_t tileAddress, uint8_t tileRow, const ColorPalette& palette, std::vector<RGBA>& outVec)
{
	assert(outVec.size() >= 8);
	auto low = bus->read(tileAddress + (tileRow * 2));
	auto high = bus->read(tileAddress + (tileRow * 2) + 1);

	for (int x = 7; x >= 0; x--)
	{
		auto lsb = isBitSet(low, x);
		auto msb = isBitSet(high, x);
		auto num = getNumberFromBits(lsb, msb);
		auto rgb = getRGBFromNumAndPalette(num, palette);

		outVec[7 - x] = std::move(rgb);
	}
}
