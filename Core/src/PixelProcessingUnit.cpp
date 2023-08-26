#include "PixelProcessingUnit.hpp"

#include "Utility.hpp"
#include "Logging.hpp"
#include <iostream>

using namespace ggb;

static constexpr int VRAM_TILE_COUNT = 256;

ggb::PixelProcessingUnit::PixelProcessingUnit(BUS* bus)
	: m_bus(bus)
{
	m_vramTiles = std::vector<Tile>(VRAM_TILE_COUNT, {});
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

	m_cycleCounter %= currentModeDuration;

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
		return;
	}
	case ggb::LCDMode::HBLank: 
	{
		auto line = incrementLine();
		if (line >= 144) 
		{
			setLCDMode(LCDMode::VBLank);
			updateAndRenderTileData();
		}
		return;
	}
	case ggb::LCDMode::VBLank: 
	{
		auto line = incrementLine();
		if (line == 0)
			setLCDMode(LCDMode::OAMBlocked);
		return;
	}
	default:
		assert(!"INVALID mode");
	}
}

bool ggb::PixelProcessingUnit::isEnabled() const
{
	return m_bus->checkBit(LCDControlRegisterAddress, 7);
}

LCDMode ggb::PixelProcessingUnit::getCurrentLCDMode() const
{
	const uint8_t buf = m_bus->read(LCDControlRegisterAddress) & 0b11;
	return LCDMode(buf);
}

void ggb::PixelProcessingUnit::setLCDMode(LCDMode mode)
{
	m_bus->setBitValue(LCDControlRegisterAddress, 0, (static_cast<uint8_t>(mode) & 1));
	m_bus->setBitValue(LCDControlRegisterAddress, 1, (static_cast<uint8_t>(mode) & (1 << 1)));
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
	auto newLinevalue = m_bus->read(lineAddress);
	newLinevalue = (newLinevalue + 1) % 154;
	m_bus->write(lineAddress, newLinevalue);

	auto lycCompare = m_bus->read(lineCompareAddress);
	if (lycCompare == newLinevalue)
	{
		m_bus->setBit(LCDStatusRegisterAddress, 2);

		if (m_bus->checkBit(LCDStatusRegisterAddress, 6))
			m_bus->setBit(0xFF0F, 1);
	}
	else
	{
		m_bus->resetBit(LCDStatusRegisterAddress, 2);
	}

	return newLinevalue;
}

uint8_t ggb::PixelProcessingUnit::getLine() const
{
	return m_bus->read(lineAddress);
}

ColorPalette ggb::PixelProcessingUnit::getBackgroundColorPalette()
{
	ColorPalette result = {};
	constexpr uint16_t backGroundPaletteAddress = 0xFF47;
	const auto res = m_bus->read(backGroundPaletteAddress);

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
