#include "FrameBuffer.hpp"

#include "Utility.hpp"

using namespace ggb;

ggb::FrameBuffer::FrameBuffer(BUS* bus, int xSize, int ySize)
	: m_xSize(xSize) 
	, m_ySize(ySize)
	, m_bus(bus)
{
	for (size_t x = 0; x < m_xSize; x++) 
		m_buffer.emplace_back(std::vector<RGB>(ySize, {0,0,0}));
}

void ggb::FrameBuffer::setPixel(int x, int y, const RGB& pixelValue)
{
	m_buffer[x][y] = pixelValue;
}

RGB ggb::FrameBuffer::getPixel(int x, int y) const
{
	return m_buffer[x][y];
}

void ggb::FrameBuffer::forEachPixel(const std::function<void(int x, int y, const RGB& rgb)>& func) const 
{
	for (size_t x = 0; x < m_xSize; x++) 
	{
		for (size_t y = 0; y < m_ySize; y++) 
		{
			func(x, y, m_buffer[x][y]);
		}
	}
}


static ggb::RGB getRGBFromNumAndPalette(uint8_t num, const ColorPalette& palette)
{
	return convertGBColorToRGB(palette.m_color[num]);
}

ggb::Tile::Tile()
{
	m_rawData = std::vector<TileRawData>(8, {0,0});
	for (size_t i = 0; i < 8; i++)
		m_data.emplace_back(std::vector<RGB>(8, {0,0,0}));
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

void ggb::overWriteTileData(BUS* bus, uint16_t tileIndex, const ColorPalette& palette, Tile* outTile)
{
	constexpr uint16_t tileDataStartAddress = 0x8000;
	constexpr uint16_t tileSize = 16;
	uint16_t address = tileDataStartAddress + (tileIndex * tileSize);
	const uint16_t endAddress = tileDataStartAddress + ((tileIndex + 1) * tileSize);


	int index = 0;
	while (address < endAddress)
	{
		auto low = bus->read(address++);
		auto high = bus->read(address++);

		outTile->m_rawData[index++] = TileRawData{ low, high };
	}

	for (size_t y = 0; y < outTile->m_rawData.size(); y++)
	{
		const auto& rawData = outTile->m_rawData[y];
		for (int x = 7; x >= 0; x--)
		{
			auto msb = isBitSet(rawData.msBits, x);
			auto lsb = isBitSet(rawData.msBits, x);
			auto num = getNumberFromBits(lsb, msb);
			auto rgb = getRGBFromNumAndPalette(num, palette);
			
			outTile->m_data[y][7 - x] = rgb;
		}
	}
}

Tile ggb::getTileByIndex(BUS* bus, uint16_t tileIndex, const ColorPalette& palette)
{
	Tile tile;
	overWriteTileData(bus, tileIndex, palette, &tile);
	return tile;
}
