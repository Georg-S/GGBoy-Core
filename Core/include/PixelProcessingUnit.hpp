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

	class PixelProcessingUnit
	{
	public:
		PixelProcessingUnit(BUS* bus);
		void step(int elapsedCycles);
		bool isEnabled() const;
		LCDMode getCurrentLCDMode(); // TODO make const
		void setLCDMode(LCDMode mode);
		void setDrawTileDataCallback(std::function<void(std::vector<Tile>)>);

	private:
		constexpr int getModeDuration(LCDMode mode);
		uint8_t incrementLine();
		ColorPalette getBackgroundColorPalette();

		BUS* m_bus = nullptr;
		int m_cycleCounter = 0;
		static constexpr uint16_t LCDCRegisterAddress = 0xFF40;
		static constexpr uint16_t lineAddress = 0xFF44;
		std::vector<Tile> m_tiles;
		std::function<void(std::vector<Tile>)> m_drawTileDataCallback;
	};
}