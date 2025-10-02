#include "Audio/AudioProcessingUnit.hpp"

#include "Utility.hpp"

ggb::AudioProcessingUnit::AudioProcessingUnit(BUS* bus)
{
	m_sampleBuffer = std::make_unique<SampleBuffer>();
	m_channel1 = std::make_unique<SquareWaveChannel>(true, bus);
	m_channel2 = std::make_unique<SquareWaveChannel>(false, bus);
	m_channel3 = std::make_unique<WaveChannel>(bus);
	m_channel4 = std::make_unique<NoiseChannel>(bus);
	m_channels[0] = m_channel1.get();
	m_channels[1] = m_channel2.get();
	m_channels[2] = m_channel3.get();
	m_channels[3] = m_channel4.get();
	setBus(bus);
	reset();
}

void ggb::AudioProcessingUnit::setBus(BUS* bus)
{
	m_masterVolume = bus->getPointerIntoMemory(AUDIO_MASTER_VOLUME_VIN_PANNING_ADDRESS);
	m_soundPanning = bus->getPointerIntoMemory(AUDIO_SOUND_PANNING_ADDRESS);
	m_soundOn = bus->getPointerIntoMemory(AUDIO_MASTER_CONTROL_ADDRESS);
	for (auto channel : m_channels)
		channel->setBus(bus);
}

bool ggb::AudioProcessingUnit::write(uint16_t address, uint8_t value)
{
	for (auto channel : m_channels)
	{
		if (channel->isChannelAddress(address))
			return channel->write(address, value);
	}

	if (address == AUDIO_MASTER_CONTROL_ADDRESS)
	{
		setBitToValue<7>(*m_soundOn, isBitSet<7>(value));
		return true;
	}
	return false;
}

std::optional<uint8_t> ggb::AudioProcessingUnit::read(uint16_t address) const
{
	for (auto channel : m_channels)
	{
		if (channel->isChannelAddress(address))
			return channel->read(address);
	}

	if (address == AUDIO_MASTER_CONTROL_ADDRESS)
	{
		uint8_t result = 0;
		setBitToValue<7>(result, isBitSet<7>(*m_soundOn));
		setBitToValue<0>(result, m_channels[0]->isOn());
		setBitToValue<1>(result, m_channels[1]->isOn());
		setBitToValue<2>(result, m_channels[2]->isOn());
		setBitToValue<3>(result, m_channels[3]->isOn());

		return result;
	}

	return {};
}

void ggb::AudioProcessingUnit::step(int cyclesPassed)
{
	if (!isBitSet<7>(*m_soundOn))
		return; // TODO reset state?

	// Call step directly without dynamic dispatch to improve performance
	m_channel1->step(cyclesPassed);
	m_channel2->step(cyclesPassed);
	m_channel3->step(cyclesPassed);
	m_channel4->step(cyclesPassed);

	frameSequencerStep(cyclesPassed);
	sampleGeneratorStep(cyclesPassed);
}

ggb::SampleBuffer* ggb::AudioProcessingUnit::getSampleBuffer()
{
	return m_sampleBuffer.get();
}

void ggb::AudioProcessingUnit::serialization(Serialization* serialization)
{
	serialization->read_write(m_frameSequencerStep);
	serialization->read_write(m_frameFrequencerCounter);
	serialization->read_write(m_cycleCounter);
	serialization->read_write(m_sampleGeneratingRate);
	for (auto channel : m_channels)
		channel->serialization(serialization);
}

void ggb::AudioProcessingUnit::reset() 
{
	m_frameSequencerStep = 0;
	m_frameFrequencerCounter = 0;
	m_cycleCounter = 0;
	m_sampleGeneratingRate = baseSampleGeneratingRate;
	for (auto channel : m_channels)
		channel->reset();
}

void ggb::AudioProcessingUnit::muteChannel(size_t channelID, bool mute)
{
	m_channels[channelID]->mute(mute);
}

bool ggb::AudioProcessingUnit::isChannelMuted(size_t channelID) const
{
	return m_channels[channelID]->isMuted();
}

void ggb::AudioProcessingUnit::sampleGeneratorStep(int cyclesPassed)
{
	Frame outFrame = {};
	auto soundPanning = [this, &outFrame](int leftBit, int rightBit, const AudioChannel* channel)
	{
		if (channel->isMuted())
			return;

		auto channelSample = channel->getSample();
		if (isBitSet(*m_soundPanning, leftBit))
			outFrame.leftSample += channelSample;
		if (isBitSet(*m_soundPanning, rightBit))
			outFrame.rightSample += channelSample;
	};

	m_cycleCounter += cyclesPassed;
	if (m_cycleCounter >= m_sampleGeneratingRate)
	{
		m_cycleCounter -= m_sampleGeneratingRate;
		soundPanning(BIT4, BIT0, m_channel1.get());
		soundPanning(BIT5, BIT1, m_channel2.get());
		soundPanning(BIT6, BIT2, m_channel3.get());
		soundPanning(BIT7, BIT3, m_channel4.get());

		const auto masterVolume = getMasterVolume();
		// Mixing is done by simply adding up the channel outputs
		outFrame.leftSample = outFrame.leftSample * masterVolume;
		outFrame.rightSample = outFrame.rightSample * masterVolume;

		// Try to stay in the "middle" of the sampling buffer
		constexpr double upperSampleGeneratingRate = baseSampleGeneratingRate * 1.0001;
		constexpr double lowerSampleGeneratingRate = baseSampleGeneratingRate * 0.9999;
		const auto remainingSize = m_sampleBuffer->push(std::move(outFrame));
		if (remainingSize >= (m_sampleBuffer->size() / 2))
			m_sampleGeneratingRate = lowerSampleGeneratingRate;
		else
			m_sampleGeneratingRate = upperSampleGeneratingRate;
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
	for (auto channel : m_channels)
		channel->tickLengthShutdown();
}

int ggb::AudioProcessingUnit::getMasterVolume() const
{
	// A master volume of 0 = very quiet (but not silent) therefore we just add one and call it a day
	return (*m_masterVolume & 0b111) + 1;
}
