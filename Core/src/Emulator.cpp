#include "Emulator.hpp"
#include "Logging.hpp"
#include "Constants.hpp"

using namespace ggb;

ggb::Emulator::Emulator()
{
	m_cpu = std::make_unique<CPU>();
	m_bus = std::make_unique<BUS>();
	m_ppu = std::make_unique<PixelProcessingUnit>(m_bus.get());
	m_timer = std::make_unique<Timer>(m_bus.get());
	m_audio = std::make_unique<AudioProcessingUnit>(m_bus.get());
	m_previousTimeStamp = getCurrentTimeInNanoSeconds();
}

bool ggb::Emulator::loadCartridge(const std::filesystem::path& path)
{
	m_currentCartridge = ggb::loadCartridge(path);
	if (!m_currentCartridge)
	{
		logError("Was not able to read ROM!");
		return false;
	}
	reset();

	return true;
}

void ggb::Emulator::run()
{
	while (true)
		step();
}

void ggb::Emulator::step()
{
	int cycles = m_cpu->step();
	m_ppu->step(cycles);
	m_timer->step(cycles);
	m_input->update();
	m_audio->step(cycles);
	synchronizeEmulatorMasterClock(cycles);
}

void ggb::Emulator::reset()
{
	m_bus->reset();
	m_cpu->reset();
	m_ppu->reset();
	m_timer->reset();
	m_syncCounter = 0;
	m_previousTimeStamp = getCurrentTimeInNanoSeconds();
	rewire();
}

void ggb::Emulator::setTileDataRenderer(std::unique_ptr<ggb::Renderer> renderer)
{
	m_ppu->setTileDataRenderer(std::move(renderer));
	m_ppu->setDrawTileData(true);
}

void ggb::Emulator::setGameRenderer(std::unique_ptr<ggb::Renderer> renderer)
{
	m_ppu->setGameRenderer(std::move(renderer));
}

void ggb::Emulator::setInput(std::unique_ptr<Input> input)
{
	m_input = std::move(input);
	m_input->setBus(m_bus.get());
}

SampleBuffer* ggb::Emulator::getSampleBuffer()
{
	return m_audio->getSampleBuffer();
}

Dimensions ggb::Emulator::getTileDataDimensions() const
{
	return m_ppu->getTileDataDimensions();
}

Dimensions ggb::Emulator::getGameWindowDimensions() const
{
	return Dimensions{ GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT };
}

void ggb::Emulator::rewire()
{
	m_bus->setCartridge(m_currentCartridge.get());
	m_bus->setTimer(m_timer.get());
	m_bus->setPixelProcessingUnit(m_ppu.get());
	m_bus->setAudio(m_audio.get());
	m_ppu->setBus(m_bus.get());
	m_cpu->setBus(m_bus.get());
	m_timer->setBus(m_bus.get());
	m_audio->setBus(m_bus.get());
	if (m_input)
		m_input->setBus(m_bus.get());
}

void ggb::Emulator::synchronizeEmulatorMasterClock(int elapsedCycles)
{
	static constexpr double masterSynchronizationAfterCycles = static_cast<double>(CPU_BASE_CLOCK) / 100.0; 

	m_syncCounter += elapsedCycles;
	if (m_syncCounter >= masterSynchronizationAfterCycles)
	{
		long long nanoSecondsNeedToPass = static_cast<long long>(NANO_SECONDS_PER_CYCLE * m_syncCounter);
		long long timeDiff = 0;

		do
		{
			auto currentTime = getCurrentTimeInNanoSeconds();
			timeDiff = currentTime - m_previousTimeStamp;
		} while (timeDiff < (nanoSecondsNeedToPass / m_emulationSpeed));

		m_previousTimeStamp = getCurrentTimeInNanoSeconds();
		m_syncCounter = 0;
	}
}
