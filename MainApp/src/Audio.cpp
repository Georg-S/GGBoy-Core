#include "Audio.hpp"

#include <SDL.h>

Audio::Audio(ggb::SampleBuffer* sampleBuffer)
{
	initializeAudio(sampleBuffer);
}

Audio::~Audio()
{
	// TODO correctly quit the audio
}

static constexpr int CHANNEL_COUNT = 2;

static void emulator_audio_callback(void* userdata, uint8_t* stream, int len)
{
	ggb::SampleBuffer* sampleBuffer = reinterpret_cast<ggb::SampleBuffer*>(userdata);
	auto audioStream = reinterpret_cast<ggb::AUDIO_FORMAT*>(stream);

	static const int volume = 15;
	const auto count = len / (sizeof(ggb::AUDIO_FORMAT) * CHANNEL_COUNT);

	for (size_t sid = 0; sid < count; ++sid)
	{
		auto frame = sampleBuffer->pop(ggb::Frame{});

		audioStream[2 * sid + 0] = frame.leftSample * volume; /* L */
		audioStream[2 * sid + 1] = frame.rightSample * volume; /* R */
	}
}

bool Audio::initializeAudio(ggb::SampleBuffer* sampleBuffer)
{
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
	audio_spec_want.callback = emulator_audio_callback;
	audio_spec_want.userdata = static_cast<void*>(sampleBuffer);

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
