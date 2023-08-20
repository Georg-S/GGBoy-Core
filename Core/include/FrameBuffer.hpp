#pragma once
#include <vector>

#include "BUS.hpp"


namespace ggb 
{
	struct RGB 
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
	};

	struct TileRawData 
	{
		uint8_t lsBits;
		uint8_t msBits;
	};

	struct Tile 
	{
		Tile(BUS* bus, int tileIndex);
		void readTileDataFromBus(BUS* bus, int tileIndex);

		std::vector<std::vector<RGB>> m_data;
		std::vector<TileRawData> m_rawData;
	};

	class FrameBuffer 
	{
	public:
		FrameBuffer(BUS* bus);
		void drawWholeScreen();

	private:
		BUS* m_bus;
		std::vector<std::vector<RGB>> m_buffer;
	};
}