#pragma once
#include <vector>
#include <array>
#include <functional>

#include "BUS.hpp"


namespace ggb 
{
	struct RGBA
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
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

	RGBA convertGBColorToRGB(GBColor color);

	struct Tile 
	{
		explicit Tile();
		std::vector<TileRawData> m_rawData;
		std::vector<std::vector<RGBA>> m_data;
	};

	void overWriteTileData(BUS* bus, uint16_t tileIndex, const ColorPalette& palette, Tile* outTile);
	Tile getTileByIndex(BUS* bus, uint16_t tileIndex, const ColorPalette& palette);

	class Renderer 
	{
	public:
		virtual ~Renderer() = default;
		virtual void startRendering() = 0;
		virtual void setPixel(int x, int y, const RGBA& rgba) = 0;
		virtual void finishRendering() = 0;
	};

	class FrameBuffer 
	{
	public:
		FrameBuffer(BUS* bus, int xSize, int ySize);
		void setPixel(int x, int y, const RGBA& pixelValue);
		RGBA getPixel(int x, int y) const;
		void forEachPixel(const std::function<void(int x, int y, const RGBA& rgb)>& func) const;

	private:
		const int m_xSize;
		const int m_ySize;
		BUS* m_bus;
		std::vector<std::vector<RGBA>> m_buffer;
	};
}