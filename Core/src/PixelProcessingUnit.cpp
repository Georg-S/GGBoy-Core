#include "PixelProcessingUnit.hpp"

#include <iostream>

#include "Utility.hpp"
#include "Logging.hpp"
#include "Constants.hpp"


using namespace ggb;

static constexpr int VRAM_TILE_COUNT = 256;

ggb::PixelProcessingUnit::PixelProcessingUnit(BUS* bus)
	: m_bus(bus)
{
	m_vramTiles = std::vector<Tile>(VRAM_TILE_COUNT, {});
	m_LCDControl = m_bus->getPointerIntoMemory(LCD_CONTROL_REGISTER_ADDRESS);
	m_LCDStatus = m_bus->getPointerIntoMemory(LCD_STATUS_REGISTER_ADDRESS);
	m_LCDYCoordinate = m_bus->getPointerIntoMemory(LCD_Y_COORDINATE_ADDRESS);
	m_LYCompare = m_bus->getPointerIntoMemory(LCD_Y_COMPARE_ADDRESS);
	m_backgroundPalette = m_bus->getPointerIntoMemory(BACKGROUND_PALETTE_ADDRESS);
	m_objectPalette0 = m_bus->getPointerIntoMemory(OBJECT_PALETTE_0_ADDRESS);
	m_objectPalette1 = m_bus->getPointerIntoMemory(OBJECT_PALETTE_1_ADDRESS);
	m_backgroundXPos = m_bus->getPointerIntoMemory(LCD_VIEWPORT_X_ADDRESS);
	m_backgroundYPos = m_bus->getPointerIntoMemory(LCD_VIEWPORT_Y_ADDRESS);
	m_windowXPos = m_bus->getPointerIntoMemory(LCD_WINDOW_X_ADDRESS);
	m_windowYPos = m_bus->getPointerIntoMemory(LCD_WINDOW_Y_ADDRESS);

	//static constexpr uint16_t LCD_VIEWPORT_Y_ADDRESS = 0xFF42;
	//static constexpr uint16_t LCD_VIEWPORT_X_ADDRESS = 0xFF43;
	//static constexpr uint16_t LCD_Y_COORDINATE_ADDRESS = 0xFF44;
	//static constexpr uint16_t LCD_Y_COMPARE_ADDRESS = 0xFF45;
	//static constexpr uint16_t BACKGROUND_PALETTE_ADDRESS = 0xFF47;
	//static constexpr uint16_t OBJECT_PALETTE_0_ADDRESS = 0xFF48;
	//static constexpr uint16_t OBJECT_PALETTE_1_ADDRESS = 0xFF49;
	//static constexpr uint16_t LCD_WINDOW_Y_ADDRESS = 0xFF4A;
	//static constexpr uint16_t LCD_WINDOW_X_ADDRESS = 0xFF4B;
}

void ggb::PixelProcessingUnit::step(int elapsedCycles)
{
	if (!isEnabled())
		return;

	const auto currentMode = getCurrentLCDMode();
	const auto currentModeDuration = getModeDuration(currentMode);
	m_cycleCounter += elapsedCycles;

	if (m_cycleCounter < currentModeDuration)
		return;

	m_cycleCounter -= currentModeDuration;

	switch (currentMode)
	{
	case ggb::LCDMode::OAMBlocked:
	{
		setLCDMode(LCDMode::VRAMBlocked);
		return;
	}
	case ggb::LCDMode::VRAMBlocked:
	{
		setLCDMode(LCDMode::HBLank);
		handleModeTransitionInterrupt(LCDInterrupt::HBlank);
		writeCurrentScanLineIntoFrameBuffer();
		return;
	}
	case ggb::LCDMode::HBLank:
	{
		auto line = incrementLine();
		if (line >= 144)
		{
			setLCDMode(LCDMode::VBLank);
			handleModeTransitionInterrupt(LCDInterrupt::VBlank);
			updateAndRenderTileData();
		}
		else 
		{
			setLCDMode(LCDMode::OAMBlocked);
			handleModeTransitionInterrupt(LCDInterrupt::OAM);
		}
		return;
	}
	case ggb::LCDMode::VBLank:
	{
		auto line = incrementLine();
		if (line == 0) 
		{
			setLCDMode(LCDMode::OAMBlocked);
			handleModeTransitionInterrupt(LCDInterrupt::OAM);
		}
		return;
	}
	default:
		assert(!"INVALID mode");
	}
}

bool ggb::PixelProcessingUnit::isEnabled() const
{
	return isBitSet(*m_LCDControl, 7);
}

LCDMode ggb::PixelProcessingUnit::getCurrentLCDMode() const
{
	const uint8_t buf = *m_LCDControl & 0b11;
	return LCDMode(buf);
}

void ggb::PixelProcessingUnit::setLCDMode(LCDMode mode)
{
	setBitToValue(*m_LCDControl, 0, static_cast<uint8_t>(mode) & 1);
	setBitToValue(*m_LCDControl, 1, static_cast<uint8_t>(mode) & (1 << 1));
}

void ggb::PixelProcessingUnit::setTileDataRenderer(std::unique_ptr<Renderer> renderer)
{
	m_tileDataRenderer = std::move(renderer);
}

Dimensions ggb::PixelProcessingUnit::getTileDataDimensions() const
{
	return Dimensions{ 300, 200 };
}

void ggb::PixelProcessingUnit::setDrawTileData(bool enable)
{
	m_drawTileData = enable;
}

void ggb::PixelProcessingUnit::setDrawWholeBackground(bool enable)
{
	m_drawWholeBackground = enable;
}

void ggb::PixelProcessingUnit::writeCurrentScanLineIntoFrameBuffer()
{
}

void ggb::PixelProcessingUnit::handleModeTransitionInterrupt(LCDInterrupt type)
{
	if (isBitSet(*m_LCDStatus, static_cast<int>(type)))
		m_bus->requestInterrupt(INTERRUPT_LCD_STAT_BIT);
}

constexpr int ggb::PixelProcessingUnit::getModeDuration(LCDMode mode)
{
	switch (mode)
	{
	case ggb::LCDMode::HBLank:		return 204;
	case ggb::LCDMode::VBLank:		return 456;
	case ggb::LCDMode::OAMBlocked:	return 80;
	case ggb::LCDMode::VRAMBlocked:	return 172;
	default:
		assert(!"FAILURE");
		return 0;
	}
}

uint8_t ggb::PixelProcessingUnit::incrementLine()
{
	*m_LCDYCoordinate = (*m_LCDYCoordinate + 1) % 154;

	if (*m_LYCompare == *m_LCDYCoordinate)
	{
		setBit(*m_LCDStatus, 2);
		
		if (isBitSet(*m_LCDStatus, 6))
			m_bus->requestInterrupt(INTERRUPT_LCD_STAT_BIT);
	}
	else
	{
		clearBit(*m_LCDStatus, 2);
	}

	return *m_LCDYCoordinate;
}

ColorPalette ggb::PixelProcessingUnit::getBackgroundColorPalette()
{
	ColorPalette result = {};
	const auto res = *m_backgroundPalette;

	result.m_color[0] = GBColor(res & 0b11);
	result.m_color[1] = GBColor((res >> 2) & 0b11);
	result.m_color[2] = GBColor((res >> 4) & 0b11);
	result.m_color[3] = GBColor((res >> 6) & 0b11);

	return result;
}

static void renderTileData(const std::vector<Tile>& tiles, Renderer* renderer)
{
	renderer->startRendering();
	int currX = 0;
	int currY = 0;
	constexpr int margin = 10;

	for (const auto& tile : tiles)
	{
		for (int x = 0; x < 8; x++)
		{
			for (int y = 0; y < 8; y++)
			{
				const auto& color = tile.m_data[x][y];
				renderer->setPixel(currX * margin + x, currY * margin + y, color);
			}
		}
		currX += 1;

		if (currX == 16)
		{
			currX = 0;
			currY += 1;
		}
	}
	renderer->finishRendering();
}

void ggb::PixelProcessingUnit::updateAndRenderTileData()
{
	if (!m_drawTileData || !m_tileDataRenderer)
		return; // We don't want to render the tile data -> therefore we don't update the data as well

	const auto colorPalette = getBackgroundColorPalette();
	for (uint16_t i = 0; i < VRAM_TILE_COUNT; i++)
		overWriteTileData(m_bus, i, colorPalette, &m_vramTiles[i]);

	renderTileData(m_vramTiles, m_tileDataRenderer.get());
}
