#include "Audio/WaveChannel.hpp"

#include "Utility.hpp"

static constexpr uint16_t CHANNEL3_DAC_ENABLE_ADDRESS = 0xFF1A;
static constexpr uint16_t CHANNEL3_PERIOD_LOW_ADDRESS = 0xFF1D;
static constexpr uint16_t CHANNEL3_PERIOD_HIGH_AND_CONTROL_ADDRESS = 0xFF1E;
static constexpr int SAMPLE_BUFFER_LENGTH = 32; // Wave RAM is 16 Bytes big and one byte holds two samples

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
		m_sampleIndex = (m_sampleIndex + 1) % SAMPLE_BUFFER_LENGTH;
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
	m_lengthCounter = getInitialLengthCounter();
}

ggb::AUDIO_FORMAT ggb::WaveChannel::getSample() const
{
	if (!m_isOn)
		return 0;

	const auto outputLevel = getOutputLevel();
	if (!outputLevel)
		return 0;
	const auto rightShift = outputLevel - 1;

	const int waveRamIndex = m_sampleIndex / 2;
	assert(waveRamIndex < (SAMPLE_BUFFER_LENGTH / 2));
	const auto twoSamples = m_waveRamStart[waveRamIndex];
	uint16_t sample = 0;
	// Two samples are stored in a one byte value of the wave RAM
	// The upper nibble is the first sample and the lower nibble the second
	if (m_sampleIndex % 2 == 0)
		sample = twoSamples >> 4;
	else
		sample = twoSamples & 0b1111;

	return sample >> rightShift;
}

void ggb::WaveChannel::tickLengthShutdown()
{
	if (!isLengthShutdownEnabled())
		return;

	m_lengthCounter++;
	if (m_lengthCounter >= 256)
	{
		m_lengthCounter = 0;
		m_isOn = false;
	}
}

void ggb::WaveChannel::serialization(Serialization* serialization)
{
	serialization->read_write(m_isOn);
	serialization->read_write(m_sampleIndex);
	serialization->read_write(m_periodCounter);
	serialization->read_write(m_lengthCounter);
}

int ggb::WaveChannel::getPeriodCounter() const
{
	const uint16_t high = *m_periodHighAndControl & 0b111;
	const uint16_t low = *m_periodLow;
	const uint16_t num = (high << 8) | low;

	return (2048 - num) * 2; // TODO
}

bool ggb::WaveChannel::isLengthShutdownEnabled() const
{
	return isBitSet(*m_periodHighAndControl, 6);
}

uint8_t ggb::WaveChannel::getInitialLengthCounter() const
{
	return *m_lengthTimer;
}

int ggb::WaveChannel::getOutputLevel() const
{
	return (*m_outputLevel & 0b1100000) >> 5;
}
