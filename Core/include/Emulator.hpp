#pragma once
#include "CPU.hpp"

#include <memory>

#include "CPU.hpp"
#include "Cartridge.hpp"
#include "BUS.hpp"


namespace ggb
{
	class Emulator
	{
	public:
		Emulator();
		bool loadCartridge(const std::filesystem::path& path);
		void run();

	private:
		CPU m_CPU;
		std::unique_ptr<BUS> m_bus;
		std::unique_ptr<Cartridge> m_currentCartridge;
	};
}