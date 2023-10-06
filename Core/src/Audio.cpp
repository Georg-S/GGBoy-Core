#include "Audio.hpp"

#include "Utility.hpp"
#include "Constants.hpp"

static bool isChannel2Memory(uint16_t address) 
{
	return (address >= 0xFF16 && address <= 0xFF19);
}

ggb::SquareWaveChannel::SquareWaveChannel(bool hasSweep, BUS* bus)
	: m_hasSweep(hasSweep)
{
	if (!hasSweep)
		m_baseAddres = 0xFF15;

	setBus(bus);
}

void ggb::SquareWaveChannel::setBus(BUS* bus)
{
	if (m_hasSweep)
		m_sweep = bus->getPointerIntoMemory(m_baseAddres);

	m_lengthTimerAndDutyCycle = bus->getPointerIntoMemory(m_baseAddres+ LENGTH_TIMER_OFFSET);
	m_volumeAndEnvelope = bus->getPointerIntoMemory(m_baseAddres+VOLUME_OFFSET);
	m_periodLow = bus->getPointerIntoMemory(m_baseAddres+PERIOD_LOW_OFFSET);
	m_periodHighAndControl = bus->getPointerIntoMemory(m_baseAddres+PERIOD_HIGH_OFFSET);
}

void ggb::SquareWaveChannel::write(uint16_t memory, uint8_t value)
{
	auto offset = memory - m_baseAddres;
	if (offset == LENGTH_TIMER_OFFSET) 
	{
		return;
	}

	if (offset == VOLUME_OFFSET) 
	{
		return;
	}

	if (offset == PERIOD_LOW_OFFSET) 
	{
		return;
	}

	if (offset == PERIOD_HIGH_OFFSET) 
	{
		*m_periodHighAndControl = value;
		if (isBitSet(*m_periodHighAndControl, 7))
			m_isOn = true;
		return;
	}
}

void ggb::SquareWaveChannel::step(int cyclesPassed)
{
}

ggb::Audio::Audio(BUS* bus)
{
	setBus(bus);
	m_channel2 = std::make_unique<SquareWaveChannel>(false, bus);
}

void ggb::Audio::setBus(BUS* bus)
{
	m_masterVolume = bus->getPointerIntoMemory(AUDIO_MASTER_VOLUME_ADDRESS);
	m_soundPanning = bus->getPointerIntoMemory(AUDIO_PANNING_ADDRESS);
	m_soundOn = bus->getPointerIntoMemory(AUDIO_MAIN_STATE_ADDRESS);
}

void ggb::Audio::write(uint16_t address, uint8_t value)
{
	if (isChannel2Memory(address))
		m_channel2->write(address, value);

	if (address == AUDIO_MAIN_STATE_ADDRESS)
		setBitToValue(*m_soundOn, 7, isBitSet(value, 7));
}

void ggb::Audio::step(int cyclesPassed)
{
	constexpr auto sampleGeneratingRate = CPU_BASE_CLOCK / STANDARD_SAMPLE_RATE;

	if (!isBitSet(*m_soundOn, 7))
		return; // TODO reset state?

	int b = 3;

}
