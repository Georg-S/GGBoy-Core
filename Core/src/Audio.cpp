#include "Audio.hpp"

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
		*m_lengthTimerAndDutyCycle = value;
		return;
	}

	if (offset == VOLUME_OFFSET) 
	{
		*m_volumeAndEnvelope = value;
		m_volume = getInitialVolume();
		if (*m_volumeAndEnvelope != 0x08)
			int b = 3;
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
	m_periodCounter += cyclesPassed;
	m_volumeCounter += cyclesPassed;

	auto calc = (m_periodCounter / CPU_CLOCKS_PER_PERIOD_INCREASE);

	if (calc >= 2048)
	{
		m_periodCounter = getPeriodValue() + ((calc - 2048)*4);
		m_dutyCyclePosition = (m_dutyCyclePosition + 1) % DUTY_CYCLE_LENGTH;
	}

	if (isLengthShutdownEnabled()) 
	{
		m_lengthCounter += cyclesPassed;
		if ((m_lengthCounter / CPU_CLOCKS_PER_LENGTH_INCREASE) >= 64) 
		{
			m_lengthCounter = 0;
			m_isOn = false;
		}
	}

	if ((*m_volumeAndEnvelope & 0b11111000) == 0)
	{
		m_isOn = false;
		return;
	}

	if (m_volumeCounter >= CPU_CLOCKS_PER_VOLUME_INCREASE) 
	{
		m_volumeCounter -= CPU_CLOCKS_PER_LENGTH_INCREASE;
		m_volumeSweepCounter++;
		const bool increase = isBitSet(*m_volumeAndEnvelope, 3);

		auto m_sweepPace = *m_volumeAndEnvelope & 0b111;
		if (m_volumeChange && (m_volumeSweepCounter >= m_sweepPace))
		{
			if (!m_sweepPace) 
			{
				m_volumeSweepCounter = 0;
			}
			else 
			{
				m_volumeSweepCounter -= m_sweepPace;
				if (increase)
					m_volume += 1;
				else
					m_volume -= 1;
			}
		}

		if (m_volume > 15 || m_volume < 0) 
		{
			m_volume = std::clamp(m_volume, 0, 15);
			m_volumeChange = false;
		}
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
	//if (sample == 0)
	//	sample = -1;

	// TODO handle volume
	return sample * m_volume;
}

bool ggb::SquareWaveChannel::isLengthShutdownEnabled() const
{
	return isBitSet(*m_periodHighAndControl, 6);
}

uint16_t ggb::SquareWaveChannel::getPeriodValue() const
{
	uint16_t high = *m_periodHighAndControl & 0b111;
	uint16_t low = *m_periodLow;

	return ((high << 8 | low) * CPU_CLOCKS_PER_PERIOD_INCREASE);
}

int ggb::SquareWaveChannel::getUsedDutyCycleIndex() const
{
	bool msb = isBitSet(*m_lengthTimerAndDutyCycle, 7);
	bool lsb = isBitSet(*m_lengthTimerAndDutyCycle, 6);

	return getNumberFromBits(lsb, msb);
}

int ggb::SquareWaveChannel::getInitialLengthCounter() const
{
	constexpr auto lengthBitMask = static_cast<uint8_t>(0b111111);
	return (*m_lengthTimerAndDutyCycle & lengthBitMask) * CPU_CLOCKS_PER_LENGTH_INCREASE;
}

int ggb::SquareWaveChannel::getInitialVolume() const
{
	uint8_t mask = static_cast<uint8_t>(0b11110000);
	auto bitAnd = (*m_volumeAndEnvelope & mask);
	return bitAnd >> 4;
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
	if (m_channel2)
		m_channel2->setBus(bus);
}

void ggb::Audio::write(uint16_t address, uint8_t value)
{
	if (isChannel2Memory(address))
		m_channel2->write(address, value);

	if (address == AUDIO_MAIN_STATE_ADDRESS) 
		setBitToValue(*m_soundOn, 7, isBitSet(value, 7));
}

uint8_t ggb::Audio::read(uint16_t address)
{
	if (isChannel2Memory(address))
		int c = 3;
	int b = 3;
	return 0xFF;
}

void ggb::Audio::step(int cyclesPassed)
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

ggb::SampleBuffer* ggb::Audio::getSampleBuffer()
{
	return &m_sampleBuffer;
}
