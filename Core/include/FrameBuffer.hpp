#pragma once
#include <vector>
#include <array>

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

	enum class GBColor
	{
		WHITE = 0,
		LIGHT_GREY = 1,
		DARK_GREY = 2,
		BLACK = 3,
	};

	struct ColorPalette 
	{
		std::array<GBColor, 4> m_color = {};
	};

	RGB convertGBColorToRGB(GBColor color);

	struct Tile 
	{
		Tile(BUS* bus, int tileIndex, const ColorPalette& palette);
		void readTileDataFromBus(BUS* bus, int tileIndex, const ColorPalette& palette);

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