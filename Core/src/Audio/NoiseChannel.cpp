#include "Audio/NoiseChannel.hpp"

#include "Utility.hpp"

ggb::NoiseChannel::NoiseChannel(BUS* bus)
{
	setBus(bus);
	resetLFSR();
}

void ggb::NoiseChannel::setBus(BUS* bus)
{
	m_lengthTimer = bus->getPointerIntoMemory(AUDIO_CHANNEL_4_LENGTH_TIMER_ADDRESS);
	m_volumeAndEnvelope = bus->getPointerIntoMemory(AUDIO_CHANNEL_4_VOLUME_ENVELOPE_ADDRESS);
	m_frequencyAndRandomness = bus->getPointerIntoMemory(AUDIO_CHANNEL_4_FREQUENCY_RANDOMNESS_ADDRESS);
	m_control = bus->getPointerIntoMemory(AUDIO_CHANNEL_4_CONTROL_ADDRESS);
}

bool ggb::NoiseChannel::write(uint16_t address, uint8_t value)
{
	if (address == AUDIO_CHANNEL_4_LENGTH_TIMER_ADDRESS)
	{
		*m_lengthTimer = value;
		m_lengthCounter = getInitialLengthCounter();
		return true;
	}

	if (address == AUDIO_CHANNEL_4_CONTROL_ADDRESS)
	{
		*m_control = value;
		if (isBitSet(*m_control, 7))
			trigger();
		return true;
	}

	return false;
}

std::optional<uint8_t> ggb::NoiseChannel::read(uint16_t address) const
{
	if (address == AUDIO_CHANNEL_4_CONTROL_ADDRESS)
		return *m_control & 0b01000000;

	return std::nullopt;
}

void ggb::NoiseChannel::step(int cyclesPassed)
{
	if (!m_isOn)
		return;

	static constexpr int indexToDivisorMapping[] = { 8, 16, 32, 48, 64, 80, 96, 112 };
	const auto divider = getClockDivider();
	const auto shift = getClockShift();

	const auto timer = indexToDivisorMapping[divider] << shift;
	m_cycleCounter += cyclesPassed;
	if (m_cycleCounter >= timer) 
	{
		m_cycleCounter -= timer;
		stepLFSR();
	}
}

ggb::AUDIO_FORMAT ggb::NoiseChannel::getSample() const
{
	if (!m_isOn)
		return 0;

	if (isBitSet(m_lfsr, 0))
		return 0;

	return m_volume;
}

bool ggb::NoiseChannel::isChannelAddress(uint16_t address) const
{
	return (address >= ggb::AUDIO_CHANNEL_4_LENGTH_TIMER_ADDRESS) && (address <= ggb::AUDIO_CHANNEL_4_CONTROL_ADDRESS);
}

void ggb::NoiseChannel::tickLengthShutdown()
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

void ggb::NoiseChannel::tickVolumeEnvelope()
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

void ggb::NoiseChannel::serialization(Serialization* serialization)
{
	AudioChannel::serialization(serialization);
	serialization->read_write(m_volumeSweepCounter);
	serialization->read_write(m_volume);
	serialization->read_write(m_lengthCounter);
	serialization->read_write(m_cycleCounter);
	serialization->read_write(m_volumeChange);
}

int ggb::NoiseChannel::getInitialLengthCounter() const
{
	return *m_lengthTimer;
}

int ggb::NoiseChannel::getInitialVolume() const
{
	uint8_t mask = static_cast<uint8_t>(0b11110000);
	auto bitAnd = (*m_volumeAndEnvelope & mask);
	return bitAnd >> 4;
}

void ggb::NoiseChannel::stepLFSR()
{
	const bool bit1 = isBitSet(m_lfsr, 0);
	const bool bit2 = isBitSet(m_lfsr, 1);
	const bool newBitValue = bit1 ^ bit2;
	setBitToValue(m_lfsr, 15, newBitValue);
	if (isLFSR7Bit())
		setBitToValue(m_lfsr, 7, newBitValue);

	m_lfsr = m_lfsr >> 1;
}

void ggb::NoiseChannel::trigger()
{
	m_isOn = true;
	m_volume = getInitialVolume();
	m_volumeChange = true;
	resetLFSR();
}

bool ggb::NoiseChannel::isLengthShutdownEnabled() const
{
	return isBitSet(*m_control, 6);
}

bool ggb::NoiseChannel::isLFSR7Bit() const
{
	return isBitSet(*m_frequencyAndRandomness, 3);
}

int ggb::NoiseChannel::getClockShift() const
{
	return (*m_frequencyAndRandomness & 0b11110000) >> 4;
}

int ggb::NoiseChannel::getClockDivider() const
{
	return *m_frequencyAndRandomness & 0b111;
}

void ggb::NoiseChannel::resetLFSR()
{
	m_lfsr = 0xFFFFu;
}
