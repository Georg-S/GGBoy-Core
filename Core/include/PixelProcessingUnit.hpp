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

	enum class LCDInterrupt
	{
		LYCEqualsLY = 6,
		OAM = 5,
		VBlank = 4,
		HBlank = 3,
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
		void setGameRenderer(std::unique_ptr<Renderer> renderer);
		Dimensions getTileDataDimensions() const;
		void setDrawTileData(bool enable);
		void setDrawWholeBackground(bool enable);

	private:
		void writeCurrentScanLineIntoFrameBuffer();
		void writeCurrentBackgroundLineIntoFrameBuffer();
		void writeCurrentWindowLineIntoBuffer();
		void handleModeTransitionInterrupt(LCDInterrupt type);
		constexpr int getModeDuration(LCDMode mode);
		uint8_t incrementLine();
		ColorPalette getBackgroundAndWindowColorPalette();
		void updateAndRenderTileData();
		void renderGame();

		BUS* m_bus = nullptr;
		int m_cycleCounter = 0;
		bool m_drawWholeBackground = false;
		bool m_drawTileData = false;
		std::vector<Tile> m_vramTiles;
		std::vector<RGBA> m_currentRowBuffer;
		std::unique_ptr<Renderer> m_tileDataRenderer;
		std::unique_ptr<Renderer> m_gameRenderer;
		std::unique_ptr<FrameBuffer> m_gamePicture;
		uint8_t* m_LCDControl = nullptr;
		uint8_t* m_LCDYCoordinate = nullptr;
		uint8_t* m_LYCompare = nullptr;
		uint8_t* m_LCDStatus = nullptr;
		uint8_t* m_backgroundPalette = nullptr;
		uint8_t* m_objectPalette0 = nullptr;
		uint8_t* m_objectPalette1 = nullptr;
		uint8_t* m_viewPortXPos = nullptr;
		uint8_t* m_viewPortYPos = nullptr;
		uint8_t* m_windowXPos = nullptr;
		uint8_t* m_windowYPos = nullptr;
	};
}