#pragma once
#include <Emulator.hpp>
#include <SDL.h>

class Audio 
{
public:
	Audio(ggb::SampleBuffer* sampleBuffer);
	~Audio();
	void setAudioPlaying(bool value);
	bool audioPlaying() const;
	
private:
	struct AudioData 
	{
		ggb::SampleBuffer* sampleBuffer = nullptr;
		ggb::Frame lastReadFrame = {};
	};

	bool initializeAudio(ggb::SampleBuffer* sampleBuffer);
	static void emulatorAudioCallback(void* userdata, uint8_t* stream, int len);
	AudioData m_data = {};
	bool m_audioPlaying = false;
	SDL_AudioDeviceID m_deviceID = 0;
};