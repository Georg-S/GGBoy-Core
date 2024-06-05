#include "EmulatorMain.hpp"

static std::filesystem::path cartridgePath = "";
static const std::filesystem::path RAM_BASE_PATH = "RAM/";
static const std::filesystem::path GAMES_BASE_PATH = "Roms/Games/";
static const std::filesystem::path TESTROMS_BASE_PATH = "Roms/TestROMs/";
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
	/// Returns whether a new key press occured
	auto handleKeyPress = [this](SDL_Scancode scanCode)
	{
		bool lastTickPressed = m_previousKeyStates[scanCode];
		m_previousKeyStates[scanCode] = m_keyStates[scanCode];

		return m_keyStates[scanCode] && !lastTickPressed;
	};


	if (handleKeyPress(SDL_SCANCODE_R))
		m_emulator->reset();
	if (handleKeyPress(SDL_SCANCODE_F1))
		m_emulator->saveEmulatorState("Savestate1.bin");
	if (handleKeyPress(SDL_SCANCODE_F2))
		m_emulator->saveEmulatorState("Savestate2.bin");
	if (handleKeyPress(SDL_SCANCODE_F3))
		m_emulator->saveEmulatorState("Savestate3.bin");
	if (handleKeyPress(SDL_SCANCODE_F4))
		m_emulator->saveEmulatorState("Savestate4.bin");
	if (handleKeyPress(SDL_SCANCODE_F5))
		m_emulator->loadEmulatorState("Savestate1.bin");
	if (handleKeyPress(SDL_SCANCODE_F6))
		m_emulator->loadEmulatorState("Savestate2.bin");
	if (handleKeyPress(SDL_SCANCODE_F7))
		m_emulator->loadEmulatorState("Savestate3.bin");
	if (handleKeyPress(SDL_SCANCODE_F8))
		m_emulator->loadEmulatorState("Savestate4.bin");
	if (handleKeyPress(SDL_SCANCODE_F9))
		m_emulator->muteChannel(0, !m_emulator->isChannelMuted(0));
	if (handleKeyPress(SDL_SCANCODE_F10))
		m_emulator->muteChannel(1, !m_emulator->isChannelMuted(1));
	if (handleKeyPress(SDL_SCANCODE_F11))
		m_emulator->muteChannel(2, !m_emulator->isChannelMuted(2));
	if (handleKeyPress(SDL_SCANCODE_F12))
		m_emulator->muteChannel(3, !m_emulator->isChannelMuted(3));
	
	if (handleKeyPress(SDL_SCANCODE_ESCAPE))
		m_emulator->saveRAM(RAM_BASE_PATH / cartridgePath.filename());
	if (handleKeyPress(SDL_SCANCODE_T))
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
	std::string fileName = "";

	//fileName = "Dr.Mario.gb";
	//fileName = "Tetris.gb";
	//fileName = "Legend_of_Zelda_Link's_Awakening.gb";
	//fileName = "Legend_of_Zelda_Link's_Awakening.gbc";
	fileName = "Pokemon_Gelbe_Edition.gb";
	//fileName = "Pokemon_Kristall.gbc";
	//fileName = "DragonBallZ.gbc";
	//fileName = "Pokemon_Silberne_Edition.gbc";
	//fileName = "Roms/TestROMs/interrupt_time.gb";
	//fileName = "Roms/TestROMs/instr_timing.gb";
	//fileName = "cpu_instrs.gb";
	//fileName = "01-special.gb";
	//fileName = "02-interrupts.gb";
	//fileName = "03-op sp,hl.gb";
	//fileName = "04-op r,imm.gb";
	//fileName = "05-op rp.gb";
	//fileName = "06-ld r,r.gb";
	//fileName = "07-jr,jp,call,ret,rst.gb";
	//fileName = "08-misc instrs.gb";
	//fileName = "09-op r,r.gb";
	//fileName = "10-bit ops.gb";
	//fileName = "11-op a,(hl).gb";
	//fileName = "halt_bug.gb";
	//fileName = "cgb_sound.gb";
	//fileName = "bg_oam_priority.gbc";
	//fileName = "oam_internal_priority.gbc";

	//m_emulator->setTileDataRenderer(std::move(m_tileDataRenderer));
	cartridgePath = GAMES_BASE_PATH.string() + fileName;
	//cartridgePath = TESTROMS_BASE_PATH.string() + fileName;
	m_emulator->loadCartridge(cartridgePath);
	//m_emulator->loadRAM(RAM_BASE_PATH / cartridgePath.filename());
}
