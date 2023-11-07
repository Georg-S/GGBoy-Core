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

// Needed because of SDL
#undef main

static std::filesystem::path cartridgePath = "";
static const std::filesystem::path RAM_BASE_PATH = "RAM/";
static const std::filesystem::path GAMES_BASE_PATH = "Roms/Games/";
static const std::filesystem::path SAVESTATES_BASE_PATH = "SAVESTATES/";

static void handleEmulatorKeyPresses(ggb::Emulator* emulator, const Uint8* keyStates)
{
	if (keyStates[SDL_SCANCODE_R])
		emulator->reset();
	if (keyStates[SDL_SCANCODE_F1])
		emulator->saveEmulatorState("Savestate1.bin");
	if (keyStates[SDL_SCANCODE_F2])
		emulator->saveEmulatorState("Savestate2.bin");
	if (keyStates[SDL_SCANCODE_F3])
		emulator->saveEmulatorState("Savestate3.bin");
	if (keyStates[SDL_SCANCODE_F4])
		emulator->saveEmulatorState("Savestate4.bin");
	if (keyStates[SDL_SCANCODE_F5])
		emulator->loadEmulatorState("Savestate1.bin");
	if (keyStates[SDL_SCANCODE_F6])
		emulator->loadEmulatorState("Savestate2.bin");
	if (keyStates[SDL_SCANCODE_F7])
		emulator->loadEmulatorState("Savestate3.bin");
	if (keyStates[SDL_SCANCODE_F8])
		emulator->loadEmulatorState("Savestate4.bin");

	if (keyStates[SDL_SCANCODE_ESCAPE])
		emulator->saveRAM(RAM_BASE_PATH / cartridgePath.filename());
}

static void emulatorMainLoop(ggb::Emulator* emulator, InputHandler* inputHandler) 
{
	const Uint8* keyStates = SDL_GetKeyboardState(nullptr);

	long long lastTimeStamp = ggb::getCurrentTimeInNanoSeconds();
	static constexpr long long INPUT_UPDATE_AFTER_NANOSECONDS = 10000000;
	static constexpr int UPDATE_INPUT_TICKS = 10;
	long long nanoSecondsCounter = 0;
	int counter = 0;
	while (true)
	{
		emulator->step();
		auto currentTime = ggb::getCurrentTimeInNanoSeconds();
		auto timePast = currentTime - lastTimeStamp;
		lastTimeStamp = currentTime;
		nanoSecondsCounter += timePast;
		if (nanoSecondsCounter > INPUT_UPDATE_AFTER_NANOSECONDS) 
		{
			nanoSecondsCounter -= INPUT_UPDATE_AFTER_NANOSECONDS;
			SDL_PumpEvents(); // Don't pump events on every step -> not really needed and improves performance
			inputHandler->update(timePast);
			handleEmulatorKeyPresses(emulator, keyStates);
		}
	}
}

int main(int argc, char* argv[])
{
	auto emulator = ggb::Emulator();
	auto audioHandler = std::make_unique<Audio>(emulator.getSampleBuffer());
	auto appInputHandling = std::make_unique<InputHandler>();
	auto inputHandlingPtr = appInputHandling.get();
	auto tileDataDimensions = emulator.getTileDataDimensions();
	auto gameWindowDimensions = emulator.getGameWindowDimensions();
	//auto tileDataRenderer = std::make_unique<SDLRenderer>(tileDataDimensions.width, tileDataDimensions.height, 4);
	auto gameWindowRenderer = std::make_unique<SDLRenderer>(gameWindowDimensions.width, gameWindowDimensions.height, 5);

	//cartridgePath = GAMES_BASE_PATH / "Dr.Mario.gb";
	//cartridgePath = GAMES_BASE_PATH + "Tetris.gb";
	//cartridgePath = "Roms/Games/Super_Mario_Land.gb";
	//cartridgePath = GAMES_BASE_PATH / "Legend_of_Zelda_Link's_Awakening.gb";
	cartridgePath = GAMES_BASE_PATH / "Pokemon_Gelbe_Edition.gb";
	//cartridgePath = "Roms/TestROMs/interrupt_time.gb";
	//cartridgePath = "Roms/TestROMs/instr_timing.gb";
	//cartridgePath = "Roms/TestROMs/cpu_instrs.gb";
	//cartridgePath = "Roms/TestROMs/01-special.gb";
	//cartridgePath = "Roms/TestROMs/02-interrupts.gb";
	//cartridgePath = "Roms/TestROMs/03-op sp,hl.gb";
	//cartridgePath = "Roms/TestROMs/04-op r,imm.gb";
	//cartridgePath = "Roms/TestROMs/05-op rp.gb";
	//cartridgePath = "Roms/TestROMs/06-ld r,r.gb";
	//cartridgePath = "Roms/TestROMs/07-jr,jp,call,ret,rst.gb";
	//cartridgePath = "Roms/TestROMs/08-misc instrs.gb";
	//cartridgePath = "Roms/TestROMs/09-op r,r.gb";
	//cartridgePath = "Roms/TestROMs/10-bit ops.gb";
	//cartridgePath = "Roms/TestROMs/11-op a,(hl).gb";
	//cartridgePath = "Roms/TestROMs/halt_bug.gb";
	//cartridgePath = "Roms/TestROMs/cgb_sound.gb";
	//emulator.setTileDataRenderer(std::move(tileDataRenderer));
	emulator.loadCartridge(cartridgePath);
	emulator.loadRAM(RAM_BASE_PATH / cartridgePath.filename());

	emulator.setGameRenderer(std::move(gameWindowRenderer));
	emulator.setInput(std::move(appInputHandling));
	emulatorMainLoop(&emulator, inputHandlingPtr);
	return 0;
}