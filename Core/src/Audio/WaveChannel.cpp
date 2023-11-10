#include "Audio/WaveChannel.hpp"

#include "Utility.hpp"

static constexpr int WAVE_CHANNEL_PERIOD_DIVIDER_FREQUENCY = 2097152;
static constexpr int SAMPLES_PER_BYTE = 2;
static constexpr int WAVE_RAM_SAMPLE_LENGTH = 32; // Wave RAM is 16 Bytes big and one byte holds two samples
static constexpr int WAVE_RAM_MEMORY_SIZE = WAVE_RAM_SAMPLE_LENGTH / SAMPLES_PER_BYTE;

ggb::WaveChannel::WaveChannel(BUS* bus)
{
	setBus(bus);
}

void ggb::WaveChannel::setBus(BUS* bus)
{
	m_enabled = bus->getPointerIntoMemory(AUDIO_CHANNEL_3_DAC_ENABLE_ADDRESS);
	m_lengthTimer = bus->getPointerIntoMemory(AUDIO_CHANNEL_3_LENGTH_TIMER_ADDRESS);
	m_outputLevel = bus->getPointerIntoMemory(AUDIO_CHANNEL_3_OUTPUT_LEVEL_ADDRESS);
	m_periodLow = bus->getPointerIntoMemory(AUDIO_CHANNEL_3_PERIOD_LOW_ADDRESS);
	m_periodHighAndControl = bus->getPointerIntoMemory(AUDIO_CHANNEL_3_PERIOD_HIGH_CONTROL_ADDRESS);
	m_waveRamStart = bus->getPointerIntoMemory(AUDIO_CHANNEL_3_WAVE_PATTERN_RAM_START_ADDRESS);
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
		m_sampleIndex = (m_sampleIndex + 1) % WAVE_RAM_SAMPLE_LENGTH;
	}
}

bool ggb::WaveChannel::write(uint16_t address, uint8_t value)
{
	if (address == AUDIO_CHANNEL_3_DAC_ENABLE_ADDRESS)
	{
		*m_enabled = value;
		m_isOn = isBitSet(*m_enabled, 7);
		return true;
	}

	if (address == AUDIO_CHANNEL_3_PERIOD_HIGH_CONTROL_ADDRESS)
	{
		*m_periodHighAndControl = value;
		if (isBitSet(*m_periodHighAndControl, 7))
			trigger();

		return true;
	}
	return false;
}

std::optional<uint8_t> ggb::WaveChannel::read(uint16_t address) const
{
	if (address == AUDIO_CHANNEL_3_PERIOD_HIGH_CONTROL_ADDRESS)
		return *m_periodHighAndControl & 0b01000000;

	return std::nullopt;
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

	const int waveRamIndex = m_sampleIndex / SAMPLES_PER_BYTE;
	assert(waveRamIndex < WAVE_RAM_MEMORY_SIZE);
	const auto twoSamples = m_waveRamStart[waveRamIndex];
	uint16_t sample = 0;
	// Two samples are stored in a one byte value of the wave RAM
	// The upper nibble is the first sample and the lower nibble the second
	if (m_sampleIndex % SAMPLES_PER_BYTE == 0)
		sample = twoSamples >> 4;
	else
		sample = twoSamples & 0b1111;

	return sample >> rightShift;
}

bool ggb::WaveChannel::isChannelAddress(uint16_t address) const
{
	return (address >= ggb::AUDIO_CHANNEL_3_DAC_ENABLE_ADDRESS && address <= ggb::AUDIO_CHANNEL_3_PERIOD_HIGH_CONTROL_ADDRESS)
		|| (address >= ggb::AUDIO_CHANNEL_3_WAVE_PATTERN_RAM_START_ADDRESS && address <= ggb::AUDIO_CHANNEL_3_WAVE_PATTERN_RAM_END_ADDRESS);
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
	AudioChannel::serialization(serialization);
	serialization->read_write(m_sampleIndex);
	serialization->read_write(m_periodCounter);
	serialization->read_write(m_lengthCounter);
}

int ggb::WaveChannel::getPeriodCounter() const
{
	constexpr auto CPU_CYCLES_PER_PERIOD_TICK = CPU_BASE_CLOCK / WAVE_CHANNEL_PERIOD_DIVIDER_FREQUENCY;
	const uint16_t high = *m_periodHighAndControl & 0b111;
	const uint16_t low = *m_periodLow;
	const uint16_t num = (high << 8) | low;

	return (2048 - num) * CPU_CYCLES_PER_PERIOD_TICK; 
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
