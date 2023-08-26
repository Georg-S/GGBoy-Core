#pragma once
#include "BUS.hpp"
#include "FrameBuffer.hpp"

#include <functional>

namespace ggb
{
	enum class LCDMode
	{
		HBLank = 0,
		VBLank = 1,
		OAMBlocked = 2,
		VRAMBlocked = 3,
	};

	struct Dimensions 
	{
		int width = 0;
		int height = 0;
	};

	class PixelProcessingUnit
	{
	public:
		PixelProcessingUnit(BUS* bus);
		void step(int elapsedCycles);
		bool isEnabled() const;
		LCDMode getCurrentLCDMode() const; 
		void setLCDMode(LCDMode mode);
		void setTileDataRenderer(std::unique_ptr<Renderer> renderer);
		Dimensions getTileDataDimensions() const;
		void setDrawTileData(bool enable);
		void setDrawWholeBackground(bool enable);

	private:
		constexpr int getModeDuration(LCDMode mode);
		uint8_t incrementLine();
		uint8_t getLine() const;
		ColorPalette getBackgroundColorPalette();
		void updateAndRenderTileData();

		BUS* m_bus = nullptr;
		int m_cycleCounter = 0;
		bool m_drawWholeBackground = false;
		bool m_drawTileData = false;
		std::vector<Tile> m_vramTiles;
		std::unique_ptr<Renderer> m_tileDataRenderer;
	};
}