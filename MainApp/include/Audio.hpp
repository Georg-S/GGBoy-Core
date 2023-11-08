#pragma once
#include <Emulator.hpp>

class Audio 
{
public:
	Audio(ggb::SampleBuffer* sampleBuffer);
	~Audio();
private:
	struct AudioData 
	{
		ggb::SampleBuffer* sampleBuffer = nullptr;
		ggb::Frame lastReadFrame = {};
	};

	bool initializeAudio(ggb::SampleBuffer* sampleBuffer);
	static void emulatorAudioCallback(void* userdata, uint8_t* stream, int len);
	AudioData m_data = {};
};