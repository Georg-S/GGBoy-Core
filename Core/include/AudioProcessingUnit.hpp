#pragma once

#include "Constants.hpp"
#include "BUS.hpp"
#include "Ringbuffer.hpp"
#include "SquareWaveChannel.hpp"

namespace ggb
{
	struct Frame
	{
		AUDIO_FORMAT leftSample;
		AUDIO_FORMAT rightSample;
	};
	using SampleBuffer = SingleProducerSingleConsumerRingbuffer<Frame, ggb::STANDARD_SAMPLE_RATE /4 >;

	// Often seen abbreviated as "APU"
	class AudioProcessingUnit 
	{
	public:
		AudioProcessingUnit(BUS* bus);
		void setBus(BUS* bus);
		void write(uint16_t address, uint8_t value);
		uint8_t read(uint16_t address);
		void step(int cyclesPassed);
		SampleBuffer* getSampleBuffer();

	private:
		int m_cycleCounter = 0;
		int m_testCounter = 0;
		
		std::unique_ptr<SquareWaveChannel> m_channel2 = nullptr;
		SampleBuffer m_sampleBuffer;
		uint8_t* m_soundOn = nullptr;
		uint8_t* m_soundPanning = nullptr;
		uint8_t* m_masterVolume = nullptr;
	};
}