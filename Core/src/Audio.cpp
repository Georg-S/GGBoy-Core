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
}

void ggb::SquareWaveChannel::step(int cyclesPassed)
{
	if (!m_isOn)
		return;

	// This counter ticks with a quarter of the cpu frequency
	m_periodCounter += cyclesPassed;

	if ((m_periodCounter / CPU_CLOCKS_PER_PERIOD_INCREASE) > 2047)
	{
		m_periodCounter = getPeriodValue();
		m_dutyCyclePosition = (m_dutyCyclePosition + 1) % DUTY_CYCLE_LENGTH;
	}

	if (isLengthShutdownEnabled()) 
	{
		m_lengthCounter += cyclesPassed;
		if ((m_lengthCounter / CPU_CLOCKS_PER_LENGTH_INCREASE) >= 64) 
		{
			m_isOn = false;
		}
	}
}

void ggb::SquareWaveChannel::trigger()
{
	m_isOn = true;
	m_periodDivider = getPeriodValue();
	m_dutyCyclePosition = 0;
}

int16_t ggb::SquareWaveChannel::getSample() const
{
	if (!m_isOn)
		return 0;

	auto index = getUsedDutyCycleIndex();
	auto sample = DUTY_CYCLES[index][m_dutyCyclePosition];

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
	constexpr auto lengthBitMask = static_cast<uint8_t>(0xb111111);
	return (*m_lengthTimerAndDutyCycle & lengthBitMask) * CPU_CLOCKS_PER_LENGTH_INCREASE;
}

int ggb::SquareWaveChannel::getInitialVolume() const
{
	return (*m_volumeAndEnvelope & 0xb11110000) >> 4;
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
		int16_t sample = m_channel2->getSample();
		//int16_t sample = 0;
		//m_testCounter++;
		//if (m_testCounter == 10) 
		//{
		//	m_testCounter = 0;
		//	sample = 1;
		//}
		m_sampleBuffer.push(sample);
	}
}

ggb::SampleBuffer* ggb::Audio::getSampleBuffer()
{
	return &m_sampleBuffer;
}
