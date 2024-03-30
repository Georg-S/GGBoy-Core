#include "RenderingUtility.hpp"

#include "Utility.hpp"

using namespace ggb;

ggb::FrameBuffer::FrameBuffer(int xSize, int ySize)
	: m_xSize(xSize) 
	, m_ySize(ySize)
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

void ggb::FrameBuffer::serialization(Serialization* serialization)
{
	serialization->read_write(m_xSize);
	serialization->read_write(m_ySize);
	serialization->read_write(m_buffer);
}

void ggb::Tile::serialization(Serialization* serialization)
{
	serialization->read_write(m_data);
}

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

void ggb::overWriteTileData(BUS* bus, uint16_t tileIndex, const ColorPalette& palette, Tile* outTile, std::vector<uint8_t>& bufVec)
{
	// TODO adjust to GBC
	//constexpr uint16_t tileDataStartAddress = 0x8000;
	//uint16_t address = tileDataStartAddress + (tileIndex * TILE_MEMORY_SIZE);
	//for (uint8_t y = 0; y < TILE_HEIGHT; y++) 
	//{
	//	getTileRowData(bus, address,y, bufVec);

	//	for (size_t i = 0; i < TILE_WIDTH; i++)
	//		outTile->m_data[y][i] = palette.getColor(bufVec[i]);
	//}
}

void ggb::getTileRowData(uint8_t* vramPtr, uint16_t tileAddress, uint8_t tileRow, std::vector<uint8_t>& outVec)
{
	assert(outVec.size() >= 8);
	auto vramIndex = getVRAMIndexFromAddress((tileAddress + (tileRow * 2)));
	auto low = vramPtr[vramIndex];
	auto high = vramPtr[vramIndex + 1];

	for (int x = 7; x >= 0; x--)
	{
		auto lsb = isBitSet(low, x);
		auto msb = isBitSet(high, x);
		auto num = getNumberFromBits(lsb, msb);
		outVec[7 - x] = num;
	}
}

const RGBA& ggb::ColorPalette::getColor(size_t index) const
{
	return m_color[index];
}
