#include "AudioProcessingUnit.hpp"

#include "Utility.hpp"

static bool isChannel2Memory(uint16_t address)
{
	return (address >= 0xFF16 && address <= 0xFF19);
}

ggb::AudioProcessingUnit::AudioProcessingUnit(BUS* bus)
{
	setBus(bus);
	m_channel2 = std::make_unique<SquareWaveChannel>(false, bus);
}

void ggb::AudioProcessingUnit::setBus(BUS* bus)
{
	m_masterVolume = bus->getPointerIntoMemory(AUDIO_MASTER_VOLUME_ADDRESS);
	m_soundPanning = bus->getPointerIntoMemory(AUDIO_PANNING_ADDRESS);
	m_soundOn = bus->getPointerIntoMemory(AUDIO_MAIN_STATE_ADDRESS);
	if (m_channel2)
		m_channel2->setBus(bus);
}

void ggb::AudioProcessingUnit::write(uint16_t address, uint8_t value)
{
	if (isChannel2Memory(address))
		m_channel2->write(address, value);

	if (address == AUDIO_MAIN_STATE_ADDRESS)
		setBitToValue(*m_soundOn, 7, isBitSet(value, 7));
}

uint8_t ggb::AudioProcessingUnit::read(uint16_t address)
{
	if (isChannel2Memory(address))
		int c = 3;
	int b = 3;
	return 0xFF;
}

void ggb::AudioProcessingUnit::step(int cyclesPassed)
{
	constexpr auto sampleGeneratingRate = CPU_BASE_CLOCK / STANDARD_SAMPLE_RATE;

	if (!isBitSet(*m_soundOn, 7))
		return; // TODO reset state?

	m_cycleCounter += cyclesPassed;
	m_channel2->step(cyclesPassed);

	if (m_cycleCounter >= sampleGeneratingRate)
	{
		m_cycleCounter -= sampleGeneratingRate;
		AUDIO_FORMAT sample = 0;
		sample = m_channel2->getSample();

		m_sampleBuffer.push({ sample,sample });
	}
}

ggb::SampleBuffer* ggb::AudioProcessingUnit::getSampleBuffer()
{
	return &m_sampleBuffer;
}
