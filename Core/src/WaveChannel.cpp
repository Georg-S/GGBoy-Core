#include "WaveChannel.hpp"

#include "Utility.hpp"

static constexpr uint16_t CHANNEL3_DAC_ENABLE_ADDRESS = 0xFF1A;
static constexpr uint16_t CHANNEL3_PERIOD_LOW_ADDRESS = 0xFF1D;
static constexpr uint16_t CHANNEL3_PERIOD_HIGH_AND_CONTROL_ADDRESS = 0xFF1E;
static constexpr int WAVE_RAM_LENGTH = 32; // In bytes

ggb::WaveChannel::WaveChannel(BUS* bus)
{
	setBus(bus);
}

void ggb::WaveChannel::setBus(BUS* bus)
{
	m_enabled = bus->getPointerIntoMemory(CHANNEL3_DAC_ENABLE_ADDRESS);
	m_lengthTimer = bus->getPointerIntoMemory(0xFF1B);
	m_outputLevel = bus->getPointerIntoMemory(0xFF1C);
	m_periodLow = bus->getPointerIntoMemory(CHANNEL3_PERIOD_LOW_ADDRESS);
	m_periodHighAndControl = bus->getPointerIntoMemory(CHANNEL3_PERIOD_HIGH_AND_CONTROL_ADDRESS);
	m_waveRamStart = bus->getPointerIntoMemory(0xFF30);
}

void ggb::WaveChannel::step(int cyclesPassed)
{
	if (!m_isOn)
		return;

	// This counter ticks with a quarter of the cpu frequency
	m_periodCounter -= cyclesPassed;

	if (m_periodCounter <= 0)
	{
		m_periodCounter = getPeriodCounter() + m_periodCounter;
		m_sampleIndex = (m_sampleIndex + 1) % WAVE_RAM_LENGTH;
	}
}

bool ggb::WaveChannel::write(uint16_t address, uint8_t value)
{
	if (address == CHANNEL3_DAC_ENABLE_ADDRESS) 
	{
		*m_enabled = value;
		m_isOn = isBitSet(*m_enabled, 7);
		return true;
	}

	if (address == CHANNEL3_PERIOD_LOW_ADDRESS)
		return false;

	//if (address == CHANNEL3_PERIOD_LOW_ADDRESS) 
	//{
	//	*m_periodLow = value;
	//	return;
	//}

	if (address == CHANNEL3_PERIOD_HIGH_AND_CONTROL_ADDRESS) 
	{
		*m_periodHighAndControl = value;
		if (isBitSet(*m_periodHighAndControl, 7))
			trigger();

		//if (isBitSet(*m_periodHighAndControl, 6))
		//	m_lengthCounter = getInitialLengthCounter();
		return true;
	}
	return false;
}

void ggb::WaveChannel::trigger()
{
	m_isOn = true;
	m_sampleIndex = 0;
	m_periodCounter = getPeriodCounter();
	//m_volume = getInitialVolume(); // TODO is this correct?
	//m_volumeChange = true;
}

ggb::AUDIO_FORMAT ggb::WaveChannel::getSample() const
{
	if (!m_isOn)
		return 0;

	const int waveRamIndex = m_sampleIndex / 2;
	const auto twoSamples = m_waveRamStart[waveRamIndex];
	uint16_t sample = 0;
	// Two samples are stored in a one byte value of the wave RAM
	// The upper nibble is the first sample and the lower nibble the second
	if (m_sampleIndex % 2 == 0)
		sample = twoSamples >> 4;
	else
		sample = twoSamples & 0b1111;

	// TODO handle audio
	return sample;
}

int ggb::WaveChannel::getPeriodCounter() const
{
	const uint16_t high = *m_periodHighAndControl & 0b111;
	const uint16_t low = *m_periodLow;
	const uint16_t num = (high << 8) | low;

	return (2048 - num) * 2; // TODO
}
