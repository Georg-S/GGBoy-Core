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
	const int windowHeight = 300;

	SDL_CreateWindowAndRenderer(windowWidth, windowHeight, 0, &window, &renderer);

	auto clearScreen = [renderer]()
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);
		//SDL_RenderPresent(renderer);
	};

	clearScreen();

	auto callBack = [renderer, &clearScreen](std::vector<ggb::Tile> vec)
	{
		clearScreen();

		int currX = 0;
		int currY = 0;
		const int margin = 10;

		for (const auto& tile : vec)
		{
			for (int x = 0; x < 8; x++)
			{
				for (int y = 0; y < 8; y++)
				{
					auto color = tile.m_data[x][y];
					SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0);
					SDL_RenderDrawPoint(renderer, (currX * margin) + x, (currY * margin) + y);
				}
			}
			currX += 1;

			if (currX == 16) 
			{
				currX = 0;
				currY += 1;
			}
		}
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
	emulator.loadCartridge("Roms/TestROMs/06-ld r,r.gb");
	//emulator.loadCartridge("Roms/TestROMs/07-jr,jp,call,ret,rst.gb");
	//emulator.loadCartridge("Roms/TestROMs/08-misc instrs.gb");
	//emulator.loadCartridge("Roms/TestROMs/09-op r,r.gb");
	//emulator.loadCartridge("Roms/TestROMs/10-bit ops.gb");
	//emulator.loadCartridge("Roms/TestROMs/11-op a,(hl).gb");
	emulator.setDrawTileDataCallback(std::move(callBack));
	emulator.run();

	return 0;
}