#include "Emulator.hpp"
#include "Logging.hpp"
#include "Constants.hpp"

#include "Utility.hpp"


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
	m_ppu->setGBCMode(m_currentCartridge->supportsColor());
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
	const bool doubleSpeed = m_bus->isGBCDoubleSpeedOn();

	int cycles = m_cpu->step();
	assert((cycles % 2) == 0);
	int gbcDoubleSpeedAdjustedCycles = cycles;
	if (doubleSpeed)
		gbcDoubleSpeedAdjustedCycles = cycles / 2;
	m_ppu->step(gbcDoubleSpeedAdjustedCycles);
	m_timer->step(cycles);
	m_input->update();
	m_audio->step(gbcDoubleSpeedAdjustedCycles);
	synchronizeEmulatorMasterClock(gbcDoubleSpeedAdjustedCycles);
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

void ggb::Emulator::saveEmulatorState(const std::filesystem::path& outputPath)
{
	try 
	{
		auto serializeUnique = std::make_unique<ggb::Serialize>(outputPath);
		auto serialize = serializeUnique.get();

		serialization(serialize);
		m_currentCartridge->serialize(serialize);
	}
	catch (const std::exception& e) 
	{
		logError(std::string("Error saving emulator state: ")  + e.what());
	}
}

void ggb::Emulator::loadEmulatorState(const std::filesystem::path& filePath)
{
	try
	{
		auto deserializeUnique = std::make_unique<ggb::Deserialize>(filePath);
		auto deserialize = deserializeUnique.get();

		serialization(deserialize);
		m_currentCartridge->deserialize(deserialize);

		rewire();
	}
	catch (const std::exception& e)
	{
		logError(std::string("Error saving emulator state: ") + e.what());
	}
}

void ggb::Emulator::saveRAM(const std::filesystem::path& outputPath)
{
	m_currentCartridge->saveRAM(outputPath);
}

void ggb::Emulator::loadRAM(const std::filesystem::path& inputPath)
{
	m_currentCartridge->loadRAM(inputPath);
}

void ggb::Emulator::setEmulationSpeed(double emulationSpeed)
{
	m_emulationSpeed = emulationSpeed;
}

double ggb::Emulator::emulationSpeed() const
{
	return m_emulationSpeed;
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

void ggb::Emulator::muteChannel(size_t channelID, bool mute)
{
	m_audio->muteChannel(channelID, mute);
}

bool ggb::Emulator::isChannelMuted(size_t channelID) const
{
	return m_audio->isChannelMuted(channelID);
}

double ggb::Emulator::getMaxSpeedup() const
{
	return m_lastMaxSpeedup;
}

void ggb::Emulator::serialization(ggb::Serialization* serialization)
{
	m_bus->serialization(serialization);
	m_cpu->serialization(serialization);
	m_ppu->serialization(serialization);
	m_timer->serialization(serialization);
	m_audio->serialization(serialization);
	serialization->read_write(m_emulationSpeed);
	serialization->read_write(m_previousTimeStamp);
	serialization->read_write(m_syncCounter);
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
	static constexpr long long nanoSecondsPerSecond = 1000000000;
	static constexpr double masterSynchronizationAfterCycles = static_cast<double>(CPU_BASE_CLOCK) / 100.0; 

	m_speedupCycleCounter += elapsedCycles;
	m_syncCounter += elapsedCycles;
	if (m_syncCounter < masterSynchronizationAfterCycles)
		return;

	const long long nanoSecondsNeedToPass = static_cast<long long>(NANO_SECONDS_PER_CYCLE * m_syncCounter);

	auto currentTime = getCurrentTimeInNanoSeconds();
	long long timeDiff = currentTime - m_previousTimeStamp;

	m_speedupTimeCounter += timeDiff;
	if (m_speedupTimeCounter >= nanoSecondsPerSecond)
	{
		// Calculate the maximum possible speedup to get a more wholistic picture on what refactorings do to the performance
		m_lastMaxSpeedup = (static_cast<double>(m_speedupCycleCounter) / CPU_BASE_CLOCK) / (m_speedupTimeCounter / nanoSecondsPerSecond);
 		m_speedupTimeCounter = 0;
		m_speedupCycleCounter = 0;
	}

	while (timeDiff < (nanoSecondsNeedToPass / m_emulationSpeed))
	{
		currentTime = getCurrentTimeInNanoSeconds();
		timeDiff = currentTime - m_previousTimeStamp;
	};

	m_previousTimeStamp = currentTime;
	m_syncCounter = 0;
}
