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
	m_periodHighAndControl = bus->getPointerIntoMemory(m_baseAddres + PERIOD_HIGH_AND_CONTROL_OFFSET);
}

bool ggb::SquareWaveChannel::write(uint16_t memory, uint8_t value)
{
	const auto offset = memory - m_baseAddres;

	if (offset == 0) 
	{
		*m_sweep = value;
		// Value of 0 (=disable frequency sweep) gets set instantly or any value if the frequency sweep is currently disabled
		// other values are set on channel triggering or after  a frequency sweep iteration
		auto initialSweep = getInitialFrequencySweepPace();
		if (!initialSweep || (m_frequencySweepPace == 0))
			m_frequencySweepPace = initialSweep;
		return true;
	}

	if (offset == LENGTH_TIMER_OFFSET)
	{
		*m_lengthTimerAndDutyCycle = value;
		m_lengthCounter = getInitialLengthCounter();
		return true;
	}

	if (offset == VOLUME_OFFSET)
		return false;

	if (offset == PERIOD_LOW_OFFSET)
		return false;

	//if (offset == VOLUME_OFFSET)
	//{
	//	*m_volumeAndEnvelope = value;
	//	return false;
	//}

	//if (offset == PERIOD_LOW_OFFSET)
	//{
	//	*m_periodLow = value;
	//	return false;
	//}

	if (offset == PERIOD_HIGH_AND_CONTROL_OFFSET)
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
		return true;
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
	if (m_hasSweep)
		m_frequencySweepPace = getInitialFrequencySweepPace();
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

void ggb::SquareWaveChannel::tickFrequencySweep()
{
	const auto individualStep = *m_sweep & 0b111;
	if (individualStep == 0 && (m_frequencySweepPace == 0))
	{
		m_frequencySweepCounter = 0;
		return;
	}

	m_frequencySweepCounter++;
	if (m_frequencySweepCounter >= m_frequencySweepPace) 
	{
		m_frequencySweepPace = getInitialFrequencySweepPace();
		m_frequencySweepCounter = 0;
		const bool increase = !isBitSet(*m_sweep, 3);

		const int high = *m_periodHighAndControl & 0b111;
		const int low = *m_periodLow;
		const int periodValue = (high << 8) | low;
		auto newPeriodValue = periodValue >> individualStep;

		if (increase)
			newPeriodValue = periodValue + newPeriodValue;
		else
			newPeriodValue = periodValue - newPeriodValue;
		assert(newPeriodValue >= 0);
		if (newPeriodValue > 0x7FF) 
		{
			m_isOn = false; // Overflow check -> disables channel
			return;
		}
		setRawPeriodValue(newPeriodValue);
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

int ggb::SquareWaveChannel::getInitialFrequencySweepPace() const
{
	const uint8_t num = (*m_sweep & 0b01110000) >> 4;

	return num;
}

void ggb::SquareWaveChannel::setRawPeriodValue(uint16_t val)
{
	const auto highNum = val >> 8;
	const auto lowNum = val & 0b11111111;
	assert((highNum & ~PERIOD_LOW_BITMASK) == 0);

	*m_periodHighAndControl = *m_periodHighAndControl & ~PERIOD_LOW_BITMASK;
	*m_periodHighAndControl = *m_periodHighAndControl | highNum;
	*m_periodLow = lowNum;
}
