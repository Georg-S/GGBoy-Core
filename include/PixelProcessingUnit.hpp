#pragma once
#include <functional>

#include "BUS.hpp"
#include "RenderingUtility.hpp"
#include "Utility.hpp"
#include "Serialization.hpp"
#include "GBCColorRAM.hpp"

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
		bool usePalette1() const
		{
			return isBitSet(*attributes, 4);
		}

		bool isFlipXSet() const
		{
			return isBitSet(*attributes, 5);
		}

		bool isFlipYSet() const 
		{
			return isBitSet(*attributes, 6);
		}

		bool drawBackgroundOverObject() const 
		{
			return isBitSet(*attributes, 7); // background and window
		}

		size_t getGBCPaletteIndex() const 
		{
			return *attributes & 0b111;
		}

		uint8_t* yPosition = nullptr;
		uint8_t* xPosition = nullptr;
		uint8_t* tileIndex = nullptr;
		uint8_t* attributes = nullptr;
	};

	struct BackgroundAndWindowPixel 
	{
		RGB rgb = {};
		uint8_t rawColorValue = 0;
		// GBC only: Basically does the same as the object setting with the same name, always false for DMG
		bool backgroundOverObj = false;  
	};

	struct ObjectPixel 
	{
		RGB rgb = {};
		bool backgroundOverObj = false; 
		bool pixelSet = false;
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
		void setGBCMode(bool value);
		Dimensions getTileDataDimensions() const;
		void setDrawTileData(bool enable);
		void setDrawWholeBackground(bool enable);
		void serialization(Serialization* serialization);
		void GBCWriteToColorRAM(uint16_t address, uint8_t value);
		uint8_t GBCReadColorRAM(uint16_t address) const;

	private:
		// Helper struct for rendering the current scanline
		struct RenderingScanlineData 
		{
			uint16_t tileIndexAddress = 0;
			bool signedAddressingMode = false;
			int tileRow = 0;
			int tileColumn = 0;
			int screenXPos = 0;
		};

		void renderGame();
		void writeCurrentScanLineIntoFrameBuffer();
		void updateCurrentScanlineObjects();
		void writeCurrentBackgroundLineIntoFrameBuffer();
		void writeTileIntoBuffer(RenderingScanlineData* inOutData);
		void writeCurrentWindowLineIntoBuffer();
		void writeCurrentObjectLineIntoBuffer();
		uint8_t* getVRAMBankPointer(uint8_t attributes);
		uint16_t getTileAddress(uint16_t tileIndexAddress, bool useSignedAddressing);
		void handleModeTransitionInterrupt(LCDInterrupt type);
		constexpr int getModeDuration(LCDMode mode) const;
		uint8_t scanLine() const;
		uint8_t incrementScanline();
		ColorPalette getBackgroundAndWindowColorPalette() const;
		ColorPalette GBCGetBackgroundAndWindowColorPalette(size_t index) const;
		ColorPalette GBCGetObjectColorPalette(const Object& obj) const;
		void updateAndRenderTileData();
		int getObjectHeight() const;
		uint8_t getBackgroundTileAttributes(uint16_t address) const;

		BUS* m_bus = nullptr;
		int m_cycleCounter = 0;
		bool m_drawWholeBackground = false;
		bool m_drawTileData = false;
		bool m_GBCMode = true;
		std::vector<Object> m_objects;
		std::vector<Object> m_currentScanlineObjects;
		std::vector<Tile> m_vramTiles;
		std::vector<uint8_t> m_objColorBuffer;
		std::vector<BackgroundAndWindowPixel> m_backgroundAndWindowPixelBuffer;
		std::vector<ObjectPixel> m_currentObjectRowPixelBuffer;
		GBCColorRAM m_GBCBackgroundColorRAM = GBCColorRAM(GBC_BACKGROUND_PALETTE_SPECIFICATION_ADDRESS);
		GBCColorRAM m_GBCObjectColorRAM = GBCColorRAM(GBC_OBJECT_COLOR_PALETTE_SPECIFICATION_ADDRESS);
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
		uint8_t* m_VRAMBank0Ptr = nullptr;
		uint8_t* m_VRAMBank1Ptr = nullptr;
		uint8_t* m_GBCObjectPriorityMode = nullptr;
	};
}