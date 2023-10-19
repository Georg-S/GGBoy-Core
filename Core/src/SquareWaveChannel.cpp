#include "SquareWaveChannel.hpp"

#include "Utility.hpp"

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

	m_lengthTimerAndDutyCycle = bus->getPointerIntoMemory(m_baseAddres + LENGTH_TIMER_OFFSET);
	m_volumeAndEnvelope = bus->getPointerIntoMemory(m_baseAddres + VOLUME_OFFSET);
	m_periodLow = bus->getPointerIntoMemory(m_baseAddres + PERIOD_LOW_OFFSET);
	m_periodHighAndControl = bus->getPointerIntoMemory(m_baseAddres + PERIOD_HIGH_OFFSET);
}

void ggb::SquareWaveChannel::write(uint16_t memory, uint8_t value)
{
	auto offset = memory - m_baseAddres;
	if (offset == LENGTH_TIMER_OFFSET)
	{
		*m_lengthTimerAndDutyCycle = value;
		m_lengthCounter = getInitialLengthCounter();
		return;
	}

	if (offset == VOLUME_OFFSET)
	{
		*m_volumeAndEnvelope = value;
		return;
	}

	if (offset == PERIOD_LOW_OFFSET)
	{
		*m_periodLow = value;
		return;
	}

	if (offset == PERIOD_HIGH_OFFSET)
	{
		*m_periodHighAndControl = value;
		if (isBitSet(*m_periodHighAndControl, 7))
		{
			trigger();
		}
		if (isBitSet(*m_periodHighAndControl, 6))
		{
			m_lengthCounter = getInitialLengthCounter();
		}
		return;
	}
	assert(!"");
}

void ggb::SquareWaveChannel::step(int cyclesPassed)
{
	if (!m_isOn)
		return;

	// This counter ticks with a quarter of the cpu frequency
	m_periodCounter -= cyclesPassed;

	if (m_periodCounter <= 0)
	{
		m_periodCounter = getPeriodValue() + m_periodCounter;
		m_dutyCyclePosition = (m_dutyCyclePosition + 1) % DUTY_CYCLE_LENGTH;
	}
}

void ggb::SquareWaveChannel::trigger()
{
	m_isOn = true;
	m_dutyCyclePosition = 0;
	m_periodCounter = getPeriodValue();
	m_volume = getInitialVolume(); // TODO is this correct?
	m_volumeChange = true;
}

ggb::AUDIO_FORMAT ggb::SquareWaveChannel::getSample() const
{
	if (!m_isOn)
		return 0;

	auto index = getUsedDutyCycleIndex();
	auto sample = DUTY_CYCLES[index][m_dutyCyclePosition];

	return sample * m_volume;
}

void ggb::SquareWaveChannel::tickVolumeEnvelope()
{
	if ((*m_volumeAndEnvelope & 0b11111000) == 0)
	{
		m_isOn = false;
		return;
	}

	m_volumeSweepCounter++;
	const bool increase = isBitSet(*m_volumeAndEnvelope, 3);
	const auto m_sweepPace = *m_volumeAndEnvelope & 0b111;
	if (m_volumeSweepCounter < m_sweepPace)
		return;

	if (!m_sweepPace || !m_volumeChange)
	{
		m_volumeSweepCounter = 0;
		return;
	}

	m_volumeSweepCounter -= m_sweepPace;
	if (increase)
		m_volume += 1;
	else
		m_volume -= 1;

	if (m_volume > 15 || m_volume < 0)
	{
		m_volume = std::clamp(m_volume, 0, 15);
		m_volumeChange = false;
	}
}

void ggb::SquareWaveChannel::tickLengthShutdown()
{
	if (!isLengthShutdownEnabled())
		return;

	m_lengthCounter++;

	if (m_lengthCounter >= 64)
	{
		m_lengthCounter = 0;
		m_isOn = false;
	}
}

bool ggb::SquareWaveChannel::isLengthShutdownEnabled() const
{
	return isBitSet(*m_periodHighAndControl, 6);
}

uint16_t ggb::SquareWaveChannel::getPeriodValue() const
{
	const uint16_t high = *m_periodHighAndControl & 0b111;
	const uint16_t low = *m_periodLow;
	const uint16_t num = (high << 8) | low;


	return (2048 - num) * 4;
}

int ggb::SquareWaveChannel::getUsedDutyCycleIndex() const
{
	bool msb = isBitSet(*m_lengthTimerAndDutyCycle, 7);
	bool lsb = isBitSet(*m_lengthTimerAndDutyCycle, 6);

	return getNumberFromBits(lsb, msb);
}

int ggb::SquareWaveChannel::getInitialLengthCounter() const
{
	return *m_lengthTimerAndDutyCycle & 0b111111;
}

int ggb::SquareWaveChannel::getInitialVolume() const
{
	uint8_t mask = static_cast<uint8_t>(0b11110000);
	auto bitAnd = (*m_volumeAndEnvelope & mask);
	return bitAnd >> 4;
}
