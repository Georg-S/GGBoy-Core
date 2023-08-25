#pragma once
#include <Emulator.hpp>
#include <FrameBuffer.hpp>
#include "SDL.h"

#undef main



int main(int argc, char* argv[])
{
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	const int windowWidth = 300;
	const int windowHeight = 200;

	SDL_CreateWindowAndRenderer(windowWidth, windowHeight, 0, &window, &renderer);

	auto clearScreen = [renderer]()
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);
		//SDL_RenderPresent(renderer);
	};

	clearScreen();

	auto callBack = [renderer, &clearScreen](const ggb::FrameBuffer& buffer)
	{
		clearScreen();
		buffer.forEachPixel([renderer](int x, int y, const ggb::RGB& rgb)
			{
				SDL_SetRenderDrawColor(renderer, rgb.r, rgb.g, rgb.b, 0);
				SDL_RenderDrawPoint(renderer, x, y);
			});


		SDL_RenderPresent(renderer);
	};



	auto emulator = ggb::Emulator();
	//emulator.loadCartridge("Roms/Games/Tetris.gb");
	//emulator.loadCartridge("Roms/Games/Super_Mario_Land.gb");
	//emulator.loadCartridge("Roms/TestROMs/cpu_instrs.gb");
	//emulator.loadCartridge("Roms/TestROMs/01-special.gb");
	//emulator.loadCartridge("Roms/TestROMs/02-interrupts.gb");
	//emulator.loadCartridge("Roms/TestROMs/03-op sp,hl.gb");
	//emulator.loadCartridge("Roms/TestROMs/04-op r,imm.gb");
	//emulator.loadCartridge("Roms/TestROMs/05-op rp.gb");
	//emulator.loadCartridge("Roms/TestROMs/06-ld r,r.gb");
	//emulator.loadCartridge("Roms/TestROMs/07-jr,jp,call,ret,rst.gb");
	//emulator.loadCartridge("Roms/TestROMs/08-misc instrs.gb");
	emulator.loadCartridge("Roms/TestROMs/09-op r,r.gb");
	//emulator.loadCartridge("Roms/TestROMs/10-bit ops.gb");
	//emulator.loadCartridge("Roms/TestROMs/11-op a,(hl).gb");
	emulator.setDrawTileDataCallback(std::move(callBack));
	emulator.run();

	return 0;
}