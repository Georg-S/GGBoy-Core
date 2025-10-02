#include "Audio/NoiseChannel.hpp"

#include "Utility.hpp"

ggb::NoiseChannel::NoiseChannel(BUS* bus)
{
	setBus(bus);
	resetLFSR();
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
		if (isBitSet<7>(*m_control))
			trigger();
		return true;
	}

	// TODO refactor volume handling into class
	if (address == AUDIO_CHANNEL_4_VOLUME_ENVELOPE_ADDRESS)
	{
		*m_volumeAndEnvelope = value;
		if ((*m_volumeAndEnvelope & 0b11111000) == 0)
			m_isOn = false;
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

ggb::AUDIO_FORMAT ggb::NoiseChannel::getSample() const
{
	if (!m_isOn)
		return 0;

	if (isBitSet<0>(m_lfsr))
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
	serialization->read_write(m_lfsr);
}

void ggb::NoiseChannel::reset()
{
	AudioChannel::reset();
	m_volumeSweepCounter = 0;
	m_volume = 0;
	m_lengthCounter = 0;
	m_cycleCounter = 0;
	m_volumeChange = false;
	m_lfsr = 0xFFFFu; // LFSR = Linear-feedback shift register
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
	const bool bit1 = isBitSet<0>(m_lfsr);
	const bool bit2 = isBitSet<1>(m_lfsr);
	const bool newBitValue = bit1 ^ bit2;
	setBitToValue<15>(m_lfsr, newBitValue);
	if (isLFSR7Bit())
		setBitToValue<7>(m_lfsr, newBitValue);

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
	return isBitSet<6>(*m_control);
}

bool ggb::NoiseChannel::isLFSR7Bit() const
{
	return isBitSet<3>(*m_frequencyAndRandomness);
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
