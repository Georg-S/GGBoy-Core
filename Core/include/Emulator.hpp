#pragma once
#include "CPU.hpp"

#include <memory>

#include "BUS.hpp"
#include "CPU.hpp"
#include "Cartridge.hpp"
#include "Timer.hpp"
#include "Input.hpp"
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
		void setTileDataRenderer(std::unique_ptr<ggb::Renderer> renderer);
		void setGameRenderer(std::unique_ptr<ggb::Renderer> renderer);
		void setInput(std::unique_ptr<Input> input);
		Dimensions getTileDataDimensions() const;
		Dimensions getGameWindowDimensions() const;

	private:
		CPU m_CPU;
		std::unique_ptr<BUS> m_bus;
		std::unique_ptr<Cartridge> m_currentCartridge;
		std::unique_ptr<PixelProcessingUnit> m_ppu;
		std::unique_ptr<Timer> m_timer;
		std::unique_ptr<Input> m_input;
	};
}