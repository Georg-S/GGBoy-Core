#pragma once
#include <cassert>
#include <iostream>
#include <Emulator.hpp>
#include <RenderingUtility.hpp>
#include <Input.hpp>

#include "Video.hpp"
#include "Audio.hpp"
#include "Inputhandling.hpp"
#include "SDL.h"

class EmulatorApplication
{
public:
	EmulatorApplication();
	int run();

private:
	void handleEmulatorKeyPresses();
	void loadCartridge();

	const Uint8* m_keyStates = nullptr;
	bool m_previousKeyStates[SDL_NUM_SCANCODES] = {};
	std::unique_ptr<ggb::Emulator> m_emulator = nullptr;
	std::unique_ptr<Audio> m_audioHandler = nullptr;
	InputHandler* m_inputHandler = nullptr;
	std::unique_ptr<SDLRenderer> m_tileDataRenderer = nullptr;
	std::unique_ptr<SDLRenderer> m_gameRenderer = nullptr;
};