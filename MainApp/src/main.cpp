#pragma once
#include <cassert>
#include <Emulator.hpp>
#include <FrameBuffer.hpp>
#include <Input.hpp>
#include "SDL.h"

// Needed because of SDL
#undef main


// TODO: improve even more e.g. by just memcopying the framebuffer
class SDLRenderer : public ggb::Renderer
{
public:
	SDLRenderer(int width, int height, int scalingFactor = 1)
		: m_width(width* scalingFactor)
		, m_height(height* scalingFactor)
		, m_scaling(scalingFactor)
	{
		SDL_CreateWindowAndRenderer(m_width, m_height, 0, &m_window, &m_renderer);
		SDL_SetWindowTitle(m_window, "GGBoy");
		m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, m_width, m_height);
	}

	~SDLRenderer()
	{
		SDL_DestroyRenderer(m_renderer);
		SDL_DestroyWindow(m_window);
		SDL_DestroyTexture(m_texture);
	}

	void startRendering() override
	{
		SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
		SDL_RenderClear(m_renderer);
		m_pitch = 0;
		m_lockedPixels = nullptr;
		SDL_LockTexture(m_texture, nullptr, reinterpret_cast<void**>(&m_lockedPixels), &m_pitch);
	};

	virtual void setPixel(int x, int y, const ggb::RGBA& rgba)
	{
		assert(x < m_width);
		assert(y < m_height);

		for (int xPos = 0; xPos < m_scaling; xPos++)
		{
			for (int yPos = 0; yPos < m_scaling; yPos++)
			{
				int xPixel = (x * m_scaling) + xPos;
				int yPixel = (y * m_scaling) + yPos;

				const uint32_t pixelPosition = (yPixel * m_pitch) + xPixel * 3;
				m_lockedPixels[pixelPosition] = rgba.r;
				m_lockedPixels[pixelPosition+1] = rgba.g;
				m_lockedPixels[pixelPosition+2] = rgba.b;
			}
		}
	};

	virtual void finishRendering()
	{
		SDL_UnlockTexture(m_texture);
		SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
		SDL_RenderPresent(m_renderer);
	};

private:
	SDL_Window* m_window = nullptr;
	SDL_Renderer* m_renderer = nullptr;
	SDL_Texture* m_texture = nullptr;
	uint8_t* m_lockedPixels = nullptr;

	int m_pitch = 0;
	int m_width = 0;
	int m_height = 0;
	int m_scaling = 1;
};




class InputHandler : public ggb::Input
{
public:
	InputHandler() = default;

	bool isAPressed() override
	{
		return m_keyStates[SDL_SCANCODE_O];
	}
	bool isBPressed() override
	{
		return m_keyStates[SDL_SCANCODE_P];
	}
	bool isStartPressed() override
	{
		return m_keyStates[SDL_SCANCODE_SPACE];
	}
	bool isSelectPressed() override
	{
		return m_keyStates[SDL_SCANCODE_RETURN];
	}
	bool isUpPressed() override
	{
		return m_keyStates[SDL_SCANCODE_W];
	}
	bool isDownPressed() override
	{
		return m_keyStates[SDL_SCANCODE_S];
	}
	bool isLeftPressed() override
	{
		return m_keyStates[SDL_SCANCODE_A];
	}
	bool isRightPressed() override
	{
		return m_keyStates[SDL_SCANCODE_D];
	}
private:
	const Uint8* m_keyStates = SDL_GetKeyboardState(NULL);
};


int main(int argc, char* argv[])
{
	auto emulator = ggb::Emulator();
	auto appInputHandling = std::make_unique<InputHandler>();
	auto tileDataDimensions = emulator.getTileDataDimensions();
	auto gameWindowDimensions = emulator.getGameWindowDimensions();
	auto tileDataRenderer = std::make_unique<SDLRenderer>(tileDataDimensions.width, tileDataDimensions.height);
	auto gameWindowRenderer = std::make_unique<SDLRenderer>(gameWindowDimensions.width, gameWindowDimensions.height, 3);

	//emulator.loadCartridge("Roms/Games/Dr.Mario.gb");
	//emulator.loadCartridge("Roms/Games/Tetris.gb");
	emulator.loadCartridge("Roms/Games/Super_Mario_Land.gb");
	//emulator.loadCartridge("Roms/Games/Legend_of_Zelda_Link's_Awakening.gb");
	//emulator.loadCartridge("Roms/Games/Pokemon_Yellow.gb");
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
	//emulator.setTileDataRenderer(std::move(tileDataRenderer));
	emulator.setGameRenderer(std::move(gameWindowRenderer));
	emulator.setInput(std::move(appInputHandling));

	int counter = 0;
	while (true)
	{
		emulator.step();
		if (counter++ == 10)
		{
			counter -= 10;
			SDL_PumpEvents(); // Don't pump events on every step -> not really needed and improves performance
		}
	}

	return 0;
}