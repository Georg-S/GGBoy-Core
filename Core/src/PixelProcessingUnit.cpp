#include "PixelProcessingUnit.hpp"

#include "Utility.hpp"
#include <iostream>

using namespace ggb;

ggb::PixelProcessingUnit::PixelProcessingUnit(BUS* bus)
	: m_bus(bus)
{
	constexpr uint16_t LCDCRegisterAddress = 0xFF40;
	constexpr uint16_t lineAddress = 0xFF44;
	m_LCDControlRegister = &bus->read(LCDCRegisterAddress);
	m_currentLine = &bus->read(lineAddress);
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
			setLCDMode(LCDMode::VBLank);
		return;
	}
	case ggb::LCDMode::VBLank: 
	{
		auto line = incrementLine();
		if (line == 0)
			setLCDMode(LCDMode::OAMBlocked);

		m_tiles.clear();
		m_tiles.reserve(256);
		auto colorPalette = getBackgroundColorPalette();
		for (int i = 0; i < 256; i++)
			m_tiles.emplace_back(m_bus, i, colorPalette);

		if (m_drawTileDataCallback)
			m_drawTileDataCallback(m_tiles);

		return;
	}
	default:
		assert(!"INVALID mode");
	}
}

bool ggb::PixelProcessingUnit::isEnabled() const
{
	return isBitSet(*m_LCDControlRegister, 7);
}

LCDMode ggb::PixelProcessingUnit::getCurrentLCDMode()
{
	uint8_t buf = *m_LCDControlRegister & 0b11;
	return LCDMode(buf);
}

void ggb::PixelProcessingUnit::setLCDMode(LCDMode mode)
{
	setBitToValue(*m_LCDControlRegister, 0, (static_cast<uint8_t>(mode) & 1));
	setBitToValue(*m_LCDControlRegister, 1, (static_cast<uint8_t>(mode) & (1 << 1)));
}

void ggb::PixelProcessingUnit::setDrawTileDataCallback(std::function<void(std::vector<Tile>)> func)
{
	m_drawTileDataCallback = std::move(func);
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
	++(*m_currentLine);
	*m_currentLine %= 154;
	return *m_currentLine;
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
