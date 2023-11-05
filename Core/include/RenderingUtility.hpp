#pragma once
#include <vector>
#include <array>
#include <functional>

#include "BUS.hpp"
#include "Serialization.hpp"


namespace ggb 
{
	struct RGBA
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};

	class FrameBuffer
	{
	public:
		FrameBuffer(int xSize, int ySize);
		void setPixel(int x, int y, const RGBA& pixelValue);
		RGBA getPixel(int x, int y) const;
		void serialization(Serialization* serialization);

		std::vector<std::vector<RGBA>> m_buffer;
	private:
		int m_xSize;
		int m_ySize;
	};

	class Renderer
	{
	public:
		virtual ~Renderer() = default;
		virtual void renderNewFrame(const FrameBuffer& frameBuffer) = 0;
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
		std::array<RGBA, 4> m_color = {};
		const RGBA& getColor(size_t index) const;
	};

	struct Tile 
	{
		explicit Tile() = default;
		void serialization(Serialization* serialization);

		RGBA m_data[TILE_HEIGHT][TILE_WIDTH] = {};
	};

	RGBA convertGBColorToRGB(GBColor color);
	void overWriteTileData(BUS* bus, uint16_t tileIndex, const ColorPalette& palette, Tile* outTile, std::vector<uint8_t>& bufVec);
	void getTileRowData(uint8_t* vramPtr, uint16_t tileAddress, uint8_t tileRow, std::vector<uint8_t>& outVec);
}