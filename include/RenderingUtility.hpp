#pragma once
#include <vector>
#include <array>
#include <functional>

#include "BUS.hpp"
#include "Serialization.hpp"


namespace ggb 
{
	struct /*alignas(4)*/ RGB
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
		// As the name suggests, only here for padding
		// seems to have a somewhat significant performance impact (roughly 10%)
		// 'alignas' doesn't give the same performance benefit
		uint8_t padding; 
	};

	class FrameBuffer
	{
	public:
		FrameBuffer(size_t width, size_t height);
		RGB* getRow(size_t y);
		void setPixel(size_t x, size_t y, const RGB& pixelValue);
		RGB getPixel(size_t x, size_t y) const;
        std::vector<RGB> getRawData() const;
		void serialization(Serialization* serialization);
		std::vector<RGB>& getRawData();
		size_t width() const;
		size_t height() const;

	private:
		size_t calculateIndex(size_t x, size_t y) const;

		size_t m_width;
		size_t m_height;
		std::vector<RGB> m_buffer;
	};

	class Renderer
	{
	public:
		virtual ~Renderer() = default;
		virtual void renderNewFrame(const FrameBuffer& frameBuffer) = 0;
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
		std::array<RGB, 4> m_color = {};
		const RGB& getColor(size_t index) const;
	};

	struct Tile 
	{
		explicit Tile() = default;
		void serialization(Serialization* serialization);

		RGB m_data[TILE_HEIGHT][TILE_WIDTH] = {};
	};

	// The colors on a gameboy color screen appear different than on a VGA / HDMI PC monitor
	// Therefore color correction is needed to get (closer to) the look of a gameboy color
	RGB colorCorrection(ggb::RGB rgb);
	RGB convertGBColorToRGB(GBColor color);
	void overWriteTileData(BUS* bus, uint16_t tileIndex, const ColorPalette& palette, Tile* outTile, std::vector<uint8_t>& bufVec);
	void getTileRowData(uint8_t* vramPtr, uint16_t tileAddress, uint8_t tileRow, std::vector<uint8_t>& outVec);
}