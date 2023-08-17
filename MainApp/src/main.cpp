#pragma once
#include <Emulator.hpp>

int main(int argc, char* argv[]) 
{
	auto emulator = ggb::Emulator();
	emulator.loadCartridge("Roms/Games/Tetris.gb");
	emulator.run();

	return 0;
}