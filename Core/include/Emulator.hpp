#pragma once
#include "CPU.hpp"

#include <memory>

#include "CPU.hpp"
#include "Cartridge.hpp"
#include "BUS.hpp"
#include "PixelProcessingUnit.hpp"
#include "FrameBuffer.hpp"


namespace ggb
{
	class Emulator
	{
	public:
		Emulator();
		bool loadCartridge(const std::filesystem::path& path);
		void run();
		void step();
		void setDrawTileDataCallback(std::function<void(std::vector<Tile>)>);

	private:
		CPU m_CPU;
		std::unique_ptr<BUS> m_bus;
		std::unique_ptr<Cartridge> m_currentCartridge;
		std::unique_ptr<PixelProcessingUnit> m_ppu;
	};
}