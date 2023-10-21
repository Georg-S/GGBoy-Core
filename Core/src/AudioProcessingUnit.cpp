#include "AudioProcessingUnit.hpp"

#include "Utility.hpp"

static bool isChannel1Memory(uint16_t address)
{
	return (address >= 0xFF10 && address <= 0xFF15);
}

static bool isChannel2Memory(uint16_t address)
{
	return (address >= 0xFF16 && address <= 0xFF19);
}

static bool isChannel3Memory(uint16_t address)
{
	return (address >= 0xFF1A && address <= 0xFF1E) || (address >= 0xFF30 && address <= 0xFF3F);
}

static bool isChannel4Memory(uint16_t address) 
{
	return (address >= 0xFF20) && (address <= 0xFF23);
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
	m_masterVolume = bus->getPointerIntoMemory(AUDIO_MASTER_VOLUME_ADDRESS);
	m_soundPanning = bus->getPointerIntoMemory(AUDIO_PANNING_ADDRESS);
	m_soundOn = bus->getPointerIntoMemory(AUDIO_MAIN_STATE_ADDRESS);
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

	if (address == AUDIO_MAIN_STATE_ADDRESS) 
	{
		setBitToValue(*m_soundOn, 7, isBitSet(value, 7));
		return true;
	}
	return false;
}

uint8_t ggb::AudioProcessingUnit::read(uint16_t address)
{
	assert(!"Not yet correctly implemented");
	int b = 3;
	return 0xFF;
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

void ggb::AudioProcessingUnit::sampleGeneratorStep(int cyclesPassed)
{
	constexpr auto sampleGeneratingRate = CPU_BASE_CLOCK / STANDARD_SAMPLE_RATE;

	m_cycleCounter += cyclesPassed;
	if (m_cycleCounter >= sampleGeneratingRate)
	{
		m_cycleCounter -= sampleGeneratingRate;
		AUDIO_FORMAT sample = 0;
		//sample += m_channel1->getSample();
		//sample += m_channel2->getSample();
		//sample += m_channel3->getSample();
		sample += m_channel4->getSample();
		//sample /= 3;

		m_sampleBuffer.push({ sample,sample });
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
