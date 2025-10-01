#include "RenderingUtility.hpp"

#include "Utility.hpp"

using namespace ggb;

ggb::FrameBuffer::FrameBuffer(size_t width, size_t height)
	: m_width(width)
	, m_height(height)
{
	m_buffer.resize(width * height);
}

RGB* ggb::FrameBuffer::getRow(size_t y)
{
	return &(m_buffer[y * m_width]);
}

void ggb::FrameBuffer::setPixel(size_t x, size_t y, const RGB& pixelValue)
{
	m_buffer[calculateIndex(x, y)] = pixelValue;
}

RGB ggb::FrameBuffer::getPixel(size_t x, size_t y) const
{
	return m_buffer[calculateIndex(x, y)];
}

std::vector<RGB> ggb::FrameBuffer::getRawData() const
{
    return m_buffer;
};


void ggb::FrameBuffer::serialization(Serialization* serialization)
{
	serialization->read_write(m_width);
	serialization->read_write(m_height);
	serialization->read_write(m_buffer);
}

std::vector<RGB>& ggb::FrameBuffer::getRawData()
{
	return m_buffer;
}

size_t ggb::FrameBuffer::width() const
{
	return m_width;
}

size_t ggb::FrameBuffer::height() const
{
	return m_height;
}

size_t ggb::FrameBuffer::calculateIndex(size_t x, size_t y) const
{
	return y * m_width + x;
}

void ggb::Tile::serialization(Serialization* serialization)
{
	serialization->read_write(m_data);
}

RGB ggb::colorCorrection(ggb::RGB rgb)
{
	// Color correction factors where found by bruteforcing the factors so that the result is somewhat similiar to BGB
	int r = static_cast<int>(9 * rgb.r + rgb.g + rgb.b);
	int g = static_cast<int>(2 * rgb.r + 6 * rgb.g + 3 * rgb.b);
	int b = static_cast<int>(rgb.r + rgb.g + 9 * rgb.b);

	rgb.r = std::min(255, r / 10);
	rgb.g = std::min(255, g / 10);
	rgb.b = std::min(255, b / 10);
	return rgb;
}

RGB ggb::convertGBColorToRGB(GBColor color)
{
	switch (color)
	{
	case ggb::GBColor::BLACK:		return { 0,0,0 };
	case ggb::GBColor::DARK_GREY:	return { 85, 85, 85 };
	case ggb::GBColor::LIGHT_GREY:	return { 170, 170, 170 };
	case ggb::GBColor::WHITE:		return { 255, 255, 255 };
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

template <int number>
static inline constexpr void fillData(uint8_t low, uint8_t high, std::vector<uint8_t>& outVec) 
{
	auto lsb = isBitSet<number>(low);
	auto msb = isBitSet<number>(high);
	auto num = getNumberFromBits(lsb, msb);
	outVec[7 - number] = num;
}

void ggb::getTileRowData(uint8_t* vramPtr, uint16_t tileAddress, uint8_t tileRow, std::vector<uint8_t>& outVec)
{
	assert(outVec.size() >= 8);
	auto vramIndex = getVRAMIndexFromAddress(tileAddress + (tileRow * 2));
	auto low = vramPtr[vramIndex];
	auto high = vramPtr[vramIndex + 1];

	fillData<7>(low, high, outVec);
	fillData<6>(low, high, outVec);
	fillData<5>(low, high, outVec);
	fillData<4>(low, high, outVec);
	fillData<3>(low, high, outVec);
	fillData<2>(low, high, outVec);
	fillData<1>(low, high, outVec);
	fillData<0>(low, high, outVec);
}

const RGB& ggb::ColorPalette::getColor(size_t index) const
{
	return m_color[index];
}
