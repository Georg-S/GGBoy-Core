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

	struct Object 
	{
		uint8_t* yPosition = nullptr;
		uint8_t* xPosition = nullptr;
		uint8_t* tileIndex = nullptr;
		uint8_t* attributes = nullptr;
	};

	class PixelProcessingUnit
	{
	public:
		PixelProcessingUnit(BUS* bus);
		void reset();
		void setBus(BUS* bus);
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
		void writeCurrentObjectLineIntoBuffer();
		void handleModeTransitionInterrupt(LCDInterrupt type);
		constexpr int getModeDuration(LCDMode mode) const;
		uint8_t incrementLine();
		ColorPalette getBackgroundAndWindowColorPalette() const;
		ColorPalette getObjectColorPalette(const Object& obj) const;
		void updateAndRenderTileData();
		void updateCurrentScanlineObjects();
		void renderGame();

		BUS* m_bus = nullptr;
		int m_cycleCounter = 0;
		bool m_drawWholeBackground = false;
		bool m_drawTileData = false;
		std::vector<Object> m_objects;
		std::vector<Object> m_currentScanlineObjects;
		std::vector<Tile> m_vramTiles;
		std::vector<RGBA> m_currentRowBuffer;
		std::unique_ptr<Renderer> m_tileDataRenderer;
		std::unique_ptr<Renderer> m_gameRenderer;
		std::unique_ptr<FrameBuffer> m_gameFrameBuffer;
		std::unique_ptr<FrameBuffer> m_tileDataFrameBuffer;
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