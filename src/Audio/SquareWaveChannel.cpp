#include "Audio/SquareWaveChannel.hpp"

#include "Utility.hpp"

ggb::SquareWaveChannel::SquareWaveChannel(bool hasSweep, BUS* bus)
	: m_hasSweep(hasSweep)
{
	setBus(bus);
	reset();
}

void ggb::SquareWaveChannel::step(int cyclesPassed)
{
	if (!m_isOn)
		return;

	// This counter ticks with a quarter of the cpu frequency
	m_periodCounter -= cyclesPassed;

	if (m_periodCounter <= 0)
	{
		m_periodCounter = getInitialPeriodCounter() + m_periodCounter;
		m_dutyCyclePosition = (m_dutyCyclePosition + 1) % DUTY_CYCLE_LENGTH;
	}
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

	if (offset == FREQUENCY_SWEEP_OFFSET)
	{
		assert(m_hasSweep);
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

	if (offset == PERIOD_HIGH_AND_CONTROL_OFFSET)
	{
		*m_periodHighAndControl = value;
		if (isBitSet<7>(*m_periodHighAndControl))
			trigger();

		return true;
	}

	// TODO refactor volume handling into class
	if (offset == VOLUME_OFFSET) 
	{
		*m_volumeAndEnvelope = value;
		if ((*m_volumeAndEnvelope & 0b11111000) == 0)
			m_isOn = false;
		return true;
	}

	return false;
}

std::optional<uint8_t> ggb::SquareWaveChannel::read(uint16_t address) const
{
	const auto offset = address - m_baseAddres;
	if (offset == LENGTH_TIMER_OFFSET)
		return *m_lengthTimerAndDutyCycle & 0b11000000;
	if (offset == PERIOD_HIGH_AND_CONTROL_OFFSET)
		return *m_periodHighAndControl & 0b01000000;

	return std::nullopt;
}

ggb::AUDIO_FORMAT ggb::SquareWaveChannel::getSample() const
{
	if (!m_isOn)
		return 0;

	auto index = getUsedDutyCycleIndex();
	auto sample = DUTY_CYCLES[index][m_dutyCyclePosition];

	return sample * m_volume;
}

bool ggb::SquareWaveChannel::isChannelAddress(uint16_t address) const
{
	if (m_hasSweep)
		return (address >= ggb::AUDIO_CHANNEL_1_FREQUENCY_SWEEP_ADDRESS && address <= ggb::AUDIO_CHANNEL_1_PERIOD_HIGH_CONTROL_ADDRESS);
	else 
		return (address >= ggb::AUDIO_CHANNEL_2_LENGTH_DUTY_ADDRESS && address <= ggb::AUDIO_CHANNEL_2_PERIOD_HIGH_CONTROL_ADDRESS);
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

void ggb::SquareWaveChannel::tickVolumeEnvelope()
{
	m_volumeSweepCounter++;
	const bool increase = isBitSet<3>(*m_volumeAndEnvelope);
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
		m_volume++;
	else
		m_volume--;

	if (m_volume > 15 || m_volume < 0)
	{
		m_volume = std::clamp(m_volume, 0, 15);
		m_volumeChange = false;
	}
}

void ggb::SquareWaveChannel::tickFrequencySweep()
{
	if (!m_isOn)
		return;

	const auto individualStep = *m_sweep & 0b111;
	const bool increase = !isBitSet<3>(*m_sweep);
	if (individualStep == 0 && (m_frequencySweepPace == 0))
	{
		m_frequencySweepCounter = 0;
		return;
	}

	auto calculateNewPeriodValue = [this, individualStep, increase]() -> int
	{
		const int periodValue = getPeriodValue();
		auto newPeriodValue = periodValue >> individualStep;

		if (increase)
			return periodValue + newPeriodValue;
		else
			return periodValue - newPeriodValue;
	};

	auto overflowCheck = [this](auto periodValue) -> bool
	{
		if (periodValue > 0x7FF)
		{
			m_isOn = false; // Overflow check -> disables channel
			return true;
		}
		return false;
	};

	m_frequencySweepCounter++;
	if (m_frequencySweepCounter >= m_frequencySweepPace) 
	{
		m_frequencySweepCounter = 0;
		m_frequencySweepPace = getInitialFrequencySweepPace();
		auto newPeriodValue = calculateNewPeriodValue();
		assert(newPeriodValue >= 0);
		if (overflowCheck(newPeriodValue))
			return;
		setRawPeriodValue(newPeriodValue);
		assert(newPeriodValue == getPeriodValue());
		overflowCheck(calculateNewPeriodValue());
	}
}

void ggb::SquareWaveChannel::serialization(Serialization* serialization)
{
	AudioChannel::serialization(serialization);
	serialization->read_write(m_dutyCyclePosition);
	serialization->read_write(m_baseAddres);
	serialization->read_write(m_periodCounter);
	serialization->read_write(m_lengthCounter);
	serialization->read_write(m_volumeSweepCounter);
	serialization->read_write(m_volume);
	serialization->read_write(m_frequencySweepCounter);
	serialization->read_write(m_frequencySweepPace);
	serialization->read_write(m_hasSweep);
	serialization->read_write(m_volumeChange);
}

void ggb::SquareWaveChannel::reset()
{
	m_dutyCyclePosition = 0;
	m_periodCounter = 0;
	m_lengthCounter = 0;
	m_volumeSweepCounter = 0;
	m_volume = 0;
	m_frequencySweepCounter = 0;
	m_frequencySweepPace = 0;
	// Don't reset m_hasSweep
	m_volumeChange = false;
	if (!m_hasSweep)
	{
		// Base Address for channel2 points into unused memory (because it has no frequency sweep functionalaty)
		// This is done, so that the offset calculation works correctly
		m_baseAddres = AUDIO_CHANNEL_2_LENGTH_DUTY_ADDRESS - 1;
	}
}

void ggb::SquareWaveChannel::trigger()
{
	m_isOn = true;
	m_dutyCyclePosition = 0;
	m_periodCounter = getPeriodValue();
	m_volume = getInitialVolume(); // TODO is this correct?
	m_volumeChange = true;
	m_lengthCounter = getInitialLengthCounter();
	if (m_hasSweep)
		m_frequencySweepPace = getInitialFrequencySweepPace();
}

bool ggb::SquareWaveChannel::isLengthShutdownEnabled() const
{
	return isBitSet<6>(*m_periodHighAndControl);
}

uint16_t ggb::SquareWaveChannel::getPeriodValue() const
{
	const uint16_t high = *m_periodHighAndControl & 0b111;
	const uint16_t low = *m_periodLow;
	const uint16_t num = (high << 8) | low;

	return num;
}

uint16_t ggb::SquareWaveChannel::getInitialPeriodCounter() const
{
	return (2048 - getPeriodValue()) * 4;
}

int ggb::SquareWaveChannel::getUsedDutyCycleIndex() const
{
	bool msb = isBitSet<7>(*m_lengthTimerAndDutyCycle);
	bool lsb = isBitSet<6>(*m_lengthTimerAndDutyCycle);

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
