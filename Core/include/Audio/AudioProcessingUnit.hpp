#pragma once

#include <optional>

#include "Constants.hpp"
#include "BUS.hpp"
#include "Ringbuffer.hpp"
#include "SquareWaveChannel.hpp"
#include "WaveChannel.hpp"
#include "NoiseChannel.hpp"
#include "Serialization.hpp"

namespace ggb
{
	struct Frame
	{
		AUDIO_FORMAT leftSample;
		AUDIO_FORMAT rightSample;
	};
	using SampleBuffer = SingleProducerSingleConsumerRingbuffer<Frame, ggb::STANDARD_SAMPLE_RATE / 10 >;

	// Often seen abbreviated as "APU"
	class AudioProcessingUnit 
	{
	public:
		AudioProcessingUnit(BUS* bus);
		void setBus(BUS* bus);
		// Returns true if the write was handled, false if a raw memory write should be made
		bool write(uint16_t address, uint8_t value);
		std::optional<uint8_t> read(uint16_t address) const;
		void step(int cyclesPassed);
		SampleBuffer* getSampleBuffer();
		void serialization(Serialization* serialization);

	private:
		void sampleGeneratorStep(int cyclesPassed);
		void frameSequencerStep(int cyclesPassed);
		void tickChannelsLengthShutdown();
		int getMasterVolume() const;
		
		int m_frameSequencerStep = 0;
		int m_frameFrequencerCounter = 0;
		double m_cycleCounter = 0;
		uint8_t* m_soundOn = nullptr;
		uint8_t* m_soundPanning = nullptr;
		uint8_t* m_masterVolume = nullptr;
		AudioChannel* m_channels[4] = {};
		std::unique_ptr<SampleBuffer> m_sampleBuffer = nullptr;
		std::unique_ptr<SquareWaveChannel> m_channel1 = nullptr;
		std::unique_ptr<SquareWaveChannel> m_channel2 = nullptr;
		std::unique_ptr<WaveChannel> m_channel3 = nullptr;
		std::unique_ptr<NoiseChannel> m_channel4 = nullptr;
	};
}