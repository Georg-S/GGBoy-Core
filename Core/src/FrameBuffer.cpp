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

ggb::Tile::Tile(BUS* bus, int tileIndex)
{
	readTileDataFromBus(bus, tileIndex);
}

static ggb::RGB getRGBFromNum(uint8_t num) 
{
	if (num == 3)
		return { 0, 0, 0 };
	if (num == 0)
		return { 255, 255, 255 };
	return { 128, 128, 128 };
}

void ggb::Tile::readTileDataFromBus(BUS* bus, int tileIndex)
{
	constexpr uint16_t tileDataStartAddress = 0x8000;
	constexpr uint16_t tileSize = 16;
	uint16_t address = tileDataStartAddress + (tileIndex * tileSize);
	const uint16_t endAddress = tileDataStartAddress + ((tileIndex+1) * tileSize);
	int index = 0;

	while (address < endAddress) 
	{
		auto low = bus->read(address++);
		auto high = bus->read(address++);
		if (low != 0 || high != 0)
			int b = 3;
		
		m_rawData.emplace_back(TileRawData{low, high});
	}

	for (const auto& rawData : m_rawData) 
	{
		m_data.emplace_back();
		for (int x = 7; x >= 0; x--) 
		{
			auto msb = isBitSet(rawData.msBits, x);
			auto lsb = isBitSet(rawData.msBits, x);
			auto num = getNumberFromBits(lsb, msb);

			m_data.back().emplace_back(getRGBFromNum(num));
		}
	}
}
