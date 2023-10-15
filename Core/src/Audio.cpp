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
		if (isBitSet(*m_periodHighAndControl, 6)) 
		{
			m_lenghtCounter = getInitialLengthCounter();
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
		m_lenghtCounter += cyclesPassed;
		if ((m_lenghtCounter / CPU_CLOCKS_PER_LENGTH_INCREASE) >= 64) 
		{
			m_isOn = false;
		}
	}
}

void ggb::SquareWaveChannel::restart()
{
	m_periodDivider = getPeriodValue();

}

int16_t ggb::SquareWaveChannel::getSample() const
{
	if (!m_isOn)
		return 0;

	auto index = getUsedDutyCycleIndex();
	// TODO handle volume
	return DUTY_CYCLES[index][m_dutyCyclePosition]; 
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

	m_cycleCounter += cyclesPassed;
	m_channel2->step(cyclesPassed);

	if (m_cycleCounter >= sampleGeneratingRate) 
	{
		m_cycleCounter -= cyclesPassed;
		int16_t sample = m_channel2->getSample();
		if (sample)
			int c = 3;
		m_sampleBuffer.push(sample);
	}
}
