#include "Audio.hpp"

#include <SDL.h>

static constexpr int CHANNEL_COUNT = 2;

Audio::Audio(ggb::SampleBuffer* sampleBuffer)
{
	initializeAudio(sampleBuffer);
}

Audio::~Audio()
{
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

bool Audio::initializeAudio(ggb::SampleBuffer* sampleBuffer)
{
	m_data.sampleBuffer = sampleBuffer;
	uint64_t samples_played = 0;

	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "Error initializing SDL. SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	SDL_AudioSpec audio_spec_want = {};
	SDL_AudioSpec audio_spec = {};
	audio_spec_want.freq = ggb::STANDARD_SAMPLE_RATE;
	audio_spec_want.format = AUDIO_S16;
	audio_spec_want.channels = CHANNEL_COUNT;
	audio_spec_want.samples = 256;
	audio_spec_want.callback = emulatorAudioCallback;
	audio_spec_want.userdata = static_cast<void*>(&m_data);

	SDL_AudioDeviceID audio_device_id = SDL_OpenAudioDevice(nullptr, 0, &audio_spec_want, &audio_spec, 0);

	if (!audio_device_id)
	{
		fprintf(stderr, "Error creating SDL audio device. SDL_Error: %s\n", SDL_GetError());
		SDL_Quit();
		return false;
	}
	SDL_PauseAudioDevice(audio_device_id, 0);

	return true;
}

void Audio::emulatorAudioCallback(void* userdata, uint8_t* stream, int len)
{
	AudioData* audioData = static_cast<AudioData*>(userdata);
	auto audioStream = reinterpret_cast<ggb::AUDIO_FORMAT*>(stream);

	static const int volume = 15;
	const auto count = len / (sizeof(ggb::AUDIO_FORMAT) * CHANNEL_COUNT);

	for (size_t sid = 0; sid < count; ++sid)
	{
		// As a default use the last read value, this prevents audio pops
		audioData->lastReadFrame = audioData->sampleBuffer->pop(audioData->lastReadFrame);

		audioStream[2 * sid + 0] = audioData->lastReadFrame.leftSample * volume; /* L */
		audioStream[2 * sid + 1] = audioData->lastReadFrame.rightSample * volume; /* R */
	}
}
