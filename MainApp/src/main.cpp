#pragma once
#include <cassert>
#include <Emulator.hpp>
#include <FrameBuffer.hpp>
#include "SDL.h"

#undef main

// TODO: maybe this can be made more performant by writing the pixels to a texture instead of using "SDL_RenderDrawPoint"
class SDLRenderer : public ggb::Renderer 
{
public:
	SDLRenderer(int width, int height) 
		: m_width(width)
		, m_height(height)
	{
		SDL_CreateWindowAndRenderer(width, height, 0, &m_window, &m_renderer);
	}

	~SDLRenderer() 
	{
		SDL_DestroyRenderer(m_renderer);
		SDL_DestroyWindow(m_window);
	}

	void startRendering() override
	{
		SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
		SDL_RenderClear(m_renderer);
	};

	virtual void setPixel(int x, int y, const ggb::RGBA& rgba) 
	{
		assert(x < m_width);
		assert(y < m_height);

		SDL_SetRenderDrawColor(m_renderer, rgba.r, rgba.g, rgba.b, rgba.a);
		SDL_RenderDrawPoint(m_renderer, x, y);
	};

	virtual void finishRendering() 
	{
		SDL_RenderPresent(m_renderer);
		SDL_PollEvent(nullptr);
	};

private:
	SDL_Window* m_window = nullptr;
	SDL_Renderer* m_renderer = nullptr;
	int m_width;
	int m_height;
};



int main(int argc, char* argv[])
{
	auto emulator = ggb::Emulator();
	auto tileDataDimensions = emulator.getTileDataDimensions();
	auto sdlRenderer = std::make_unique<SDLRenderer>(tileDataDimensions.width, tileDataDimensions.height);

	emulator.loadCartridge("Roms/Games/Tetris.gb");
	//emulator.loadCartridge("Roms/Games/Super_Mario_Land.gb");
	//emulator.loadCartridge("Roms/TestROMs/interrupt_time.gb");
	//emulator.loadCartridge("Roms/TestROMs/cpu_instrs.gb");
	//emulator.loadCartridge("Roms/TestROMs/01-special.gb");
	//emulator.loadCartridge("Roms/TestROMs/02-interrupts.gb");
	//emulator.loadCartridge("Roms/TestROMs/03-op sp,hl.gb");
	//emulator.loadCartridge("Roms/TestROMs/04-op r,imm.gb");
	//emulator.loadCartridge("Roms/TestROMs/05-op rp.gb");
	//emulator.loadCartridge("Roms/TestROMs/06-ld r,r.gb");
	//emulator.loadCartridge("Roms/TestROMs/07-jr,jp,call,ret,rst.gb");
	//emulator.loadCartridge("Roms/TestROMs/08-misc instrs.gb");
	//emulator.loadCartridge("Roms/TestROMs/09-op r,r.gb");
	//emulator.loadCartridge("Roms/TestROMs/10-bit ops.gb");
	//emulator.loadCartridge("Roms/TestROMs/11-op a,(hl).gb");
	emulator.setTileDataRenderer(std::move(sdlRenderer));
	emulator.run();

	return 0;
}