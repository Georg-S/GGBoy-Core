#include "Audio/AudioProcessingUnit.hpp"

#include "Utility.hpp"

static bool isChannel1Memory(uint16_t address)
{
	return (address >= ggb::AUDIO_CHANNEL_1_FREQUENCY_SWEEP_ADDRESS && address <= ggb::AUDIO_CHANNEL_1_PERIOD_HIGH_CONTROL_ADDRESS);
}

static bool isChannel2Memory(uint16_t address)
{
	return (address >= ggb::AUDIO_CHANNEL_2_LENGTH_DUTY_ADDRESS && address <= ggb::AUDIO_CHANNEL_2_PERIOD_HIGH_CONTROL_ADDRESS);
}

static bool isChannel3Memory(uint16_t address)
{
	return (address >= ggb::AUDIO_CHANNEL_3_DAC_ENABLE_ADDRESS && address <= ggb::AUDIO_CHANNEL_3_PERIOD_HIGH_CONTROL_ADDRESS)
		|| (address >= ggb::AUDIO_CHANNEL_3_WAVE_PATTERN_RAM_START_ADDRESS && address <= ggb::AUDIO_CHANNEL_3_WAVE_PATTERN_RAM_END_ADDRESS);
}

static bool isChannel4Memory(uint16_t address)
{
	return (address >= ggb::AUDIO_CHANNEL_4_LENGTH_TIMER_ADDRESS) && (address <= ggb::AUDIO_CHANNEL_4_CONTROL_ADDRESS);
}

ggb::AudioProcessingUnit::AudioProcessingUnit(BUS* bus)
{
	setBus(bus);
	m_channel1 = std::make_unique<SquareWaveChannel>(true, bus);
	m_channel2 = std::make_unique<SquareWaveChannel>(false, bus);
	m_channel3 = std::make_unique<WaveChannel>(bus);
	m_channel4 = std::make_unique<NoiseChannel>(bus);
}

void ggb::AudioProcessingUnit::setBus(BUS* bus)
{
	m_masterVolume = bus->getPointerIntoMemory(AUDIO_MASTER_VOLUME_VIN_PANNING_ADDRESS);
	m_soundPanning = bus->getPointerIntoMemory(AUDIO_SOUND_PANNING_ADDRESS);
	m_soundOn = bus->getPointerIntoMemory(AUDIO_MASTER_CONTROL_ADDRESS);
	if (m_channel1)
		m_channel1->setBus(bus);
	if (m_channel2)
		m_channel2->setBus(bus);
	if (m_channel3)
		m_channel3->setBus(bus);
	if (m_channel4)
		m_channel4->setBus(bus);
}

bool ggb::AudioProcessingUnit::write(uint16_t address, uint8_t value)
{
	if (isChannel1Memory(address))
		return m_channel1->write(address, value);
	if (isChannel2Memory(address))
		return m_channel2->write(address, value);
	if (isChannel3Memory(address))
		return m_channel3->write(address, value);
	if (isChannel4Memory(address))
		return m_channel4->write(address, value);

	if (address == AUDIO_MASTER_CONTROL_ADDRESS)
	{
		setBitToValue(*m_soundOn, 7, isBitSet(value, 7));
		return true;
	}
	return false;
}

std::optional<uint8_t> ggb::AudioProcessingUnit::read(uint16_t address) const
{
	uint8_t result = 0;
	if (isChannel1Memory(address))
		return m_channel1->read(address);
	if (isChannel2Memory(address))
		return m_channel2->read(address);
	if (isChannel3Memory(address))
		return m_channel3->read(address);
	if (isChannel4Memory(address))
		return m_channel4->read(address);

	if (address == AUDIO_MASTER_CONTROL_ADDRESS)
	{
		// TODO maybe all channels should use the audio master register directly when turning the channel on/off
		setBitToValue(result, 7, isBitSet(*m_soundOn, 7));
		setBitToValue(result, 3, m_channel4->isOn());
		setBitToValue(result, 2, m_channel3->isOn());
		setBitToValue(result, 1, m_channel2->isOn());
		setBitToValue(result, 0, m_channel1->isOn());
		return result;
	}

	return {};
}

void ggb::AudioProcessingUnit::step(int cyclesPassed)
{
	if (!isBitSet(*m_soundOn, 7))
		return; // TODO reset state?

	m_channel1->step(cyclesPassed);
	m_channel2->step(cyclesPassed);
	m_channel3->step(cyclesPassed);
	m_channel4->step(cyclesPassed);
	frameSequencerStep(cyclesPassed);
	sampleGeneratorStep(cyclesPassed);
}

ggb::SampleBuffer* ggb::AudioProcessingUnit::getSampleBuffer()
{
	return &m_sampleBuffer;
}

void ggb::AudioProcessingUnit::serialization(Serialization* serialization)
{
	serialization->read_write(m_frameSequencerStep);
	serialization->read_write(m_frameFrequencerCounter);
	serialization->read_write(m_cycleCounter);
	m_channel1->serialization(serialization);
	m_channel2->serialization(serialization);
	m_channel3->serialization(serialization);
	m_channel4->serialization(serialization);
}

void ggb::AudioProcessingUnit::sampleGeneratorStep(int cyclesPassed)
{
	constexpr double sampleGeneratingRate = static_cast<double>(CPU_BASE_CLOCK) / static_cast<double>(STANDARD_SAMPLE_RATE);
	const auto masterVolume = getMasterVolume();
	Frame outFrame = {};

	auto soundPanning = [this, &outFrame](int leftBit, int rightBit, AUDIO_FORMAT channelSample)
	{
		if (isBitSet(*m_soundPanning, leftBit))
			outFrame.leftSample += channelSample;
		if (isBitSet(*m_soundPanning, rightBit))
			outFrame.rightSample += channelSample;
	};

	m_cycleCounter += cyclesPassed;
	if (m_cycleCounter >= sampleGeneratingRate)
	{
		m_cycleCounter -= sampleGeneratingRate;
		soundPanning(4, 0, m_channel1->getSample());
		soundPanning(5, 1, m_channel2->getSample());
		soundPanning(6, 2, m_channel3->getSample());
		soundPanning(7, 3, m_channel4->getSample());
		// Mixing is done by simply adding up the channel outputs
		outFrame.leftSample = outFrame.leftSample * masterVolume;
		outFrame.rightSample = outFrame.rightSample * masterVolume;

		m_sampleBuffer.push(std::move(outFrame));
	}
}

void ggb::AudioProcessingUnit::frameSequencerStep(int cyclesPassed)
{
	constexpr auto FRAME_SEQUENCER_FREQUENCY = 512;
	constexpr auto CPU_CLOCKS_PER_FRAME_SEQUENCER_INCREASE = CPU_BASE_CLOCK / FRAME_SEQUENCER_FREQUENCY;

	m_frameFrequencerCounter += cyclesPassed;
	if (m_frameFrequencerCounter >= CPU_CLOCKS_PER_FRAME_SEQUENCER_INCREASE)
	{
		m_frameFrequencerCounter -= CPU_CLOCKS_PER_FRAME_SEQUENCER_INCREASE;
		m_frameSequencerStep = (m_frameSequencerStep + 1) % 8;

		if ((m_frameSequencerStep == 0) || (m_frameSequencerStep == 4))
		{
			tickChannelsLengthShutdown();
		}
		else if (m_frameSequencerStep == 2)
		{
			m_channel1->tickFrequencySweep();
			tickChannelsLengthShutdown();
		}
		else if (m_frameSequencerStep == 6)
		{
			m_channel1->tickFrequencySweep();
			tickChannelsLengthShutdown();
		}
		else if (m_frameSequencerStep == 7)
		{
			m_channel1->tickVolumeEnvelope();
			m_channel2->tickVolumeEnvelope();
			m_channel4->tickVolumeEnvelope();
		}
	}
}

void ggb::AudioProcessingUnit::tickChannelsLengthShutdown()
{
	m_channel1->tickLengthShutdown();
	m_channel2->tickLengthShutdown();
	m_channel3->tickLengthShutdown();
	m_channel4->tickLengthShutdown();
}

int ggb::AudioProcessingUnit::getMasterVolume() const
{
	// A master volume of 0 = very quiet (but not silent) therefore we just add one and call it a day
	return (*m_masterVolume & 0b111) + 1;
}
