#include "EmulatorMain.hpp"

static std::filesystem::path cartridgePath = "";
static const std::filesystem::path RAM_BASE_PATH = "RAM/";
static const std::filesystem::path GAMES_BASE_PATH = "Roms/Games/";
static const std::filesystem::path SAVESTATES_BASE_PATH = "SAVESTATES/";

EmulatorApplication::EmulatorApplication()
{
	m_emulator = std::make_unique<ggb::Emulator>();
	m_audioHandler = std::make_unique<Audio>(m_emulator->getSampleBuffer());
	auto inputHandler = std::make_unique<InputHandler>();
	m_inputHandler = inputHandler.get();

	auto tileDataDimensions = m_emulator->getTileDataDimensions();
	auto gameWindowDimensions = m_emulator->getGameWindowDimensions();
	//m_tileDataRenderer = std::make_unique<SDLRenderer>(tileDataDimensions.width, tileDataDimensions.height, 4);
	auto gameRenderer = std::make_unique<SDLRenderer>(gameWindowDimensions.width, gameWindowDimensions.height, 5);
	m_keyStates = SDL_GetKeyboardState(nullptr);
	loadCartridge();

	m_emulator->setGameRenderer(std::move(gameRenderer));
	m_emulator->setInput(std::move(inputHandler));
}

int EmulatorApplication::run()
{
	long long lastTimeStamp = ggb::getCurrentTimeInNanoSeconds();
	static constexpr long long INPUT_UPDATE_AFTER_NANOSECONDS = 10000000;
	long long nanoSecondsCounter = 0;
	int counter = 0;
	while (true)
	{
		m_emulator->step();
		auto currentTime = ggb::getCurrentTimeInNanoSeconds();
		auto timePast = currentTime - lastTimeStamp;
		lastTimeStamp = currentTime;
		nanoSecondsCounter += timePast;
		if (nanoSecondsCounter > INPUT_UPDATE_AFTER_NANOSECONDS)
		{
			nanoSecondsCounter -= INPUT_UPDATE_AFTER_NANOSECONDS;
			SDL_PumpEvents(); // Don't pump events on every step -> not really needed and improves performance
			m_inputHandler->update(timePast);
			handleEmulatorKeyPresses();
		}
	}
}

void EmulatorApplication::handleEmulatorKeyPresses()
{
	if (m_keyStates[SDL_SCANCODE_R])
		m_emulator->reset();
	if (m_keyStates[SDL_SCANCODE_F1])
		m_emulator->saveEmulatorState("Savestate1.bin");
	if (m_keyStates[SDL_SCANCODE_F2])
		m_emulator->saveEmulatorState("Savestate2.bin");
	if (m_keyStates[SDL_SCANCODE_F3])
		m_emulator->saveEmulatorState("Savestate3.bin");
	if (m_keyStates[SDL_SCANCODE_F4])
		m_emulator->saveEmulatorState("Savestate4.bin");
	if (m_keyStates[SDL_SCANCODE_F5])
		m_emulator->loadEmulatorState("Savestate1.bin");
	if (m_keyStates[SDL_SCANCODE_F6])
		m_emulator->loadEmulatorState("Savestate2.bin");
	if (m_keyStates[SDL_SCANCODE_F7])
		m_emulator->loadEmulatorState("Savestate3.bin");
	if (m_keyStates[SDL_SCANCODE_F8])
		m_emulator->loadEmulatorState("Savestate4.bin");
	if (m_keyStates[SDL_SCANCODE_ESCAPE])
		m_emulator->saveRAM(RAM_BASE_PATH / cartridgePath.filename());
	if (m_keyStates[SDL_SCANCODE_T])
	{
		if (m_emulator->emulationSpeed() == 1.0)
		{
			m_emulator->setEmulationSpeed(10.0);
			m_audioHandler->setAudioPlaying(false);
		}
		else
		{
			m_emulator->setEmulationSpeed(1.0);
			m_audioHandler->setAudioPlaying(true);
		}
	}

}

void EmulatorApplication::loadCartridge()
{
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
	m_emulator->loadCartridge(cartridgePath);
	m_emulator->loadRAM(RAM_BASE_PATH / cartridgePath.filename());
}
