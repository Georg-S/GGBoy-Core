#pragma once
#include <cassert>
#include <iostream>
#include <Emulator.hpp>
#include <RenderingUtility.hpp>
#include <Input.hpp>
#include "SDL.h"

// Needed because of SDL
#undef main

class SDLRenderer : public ggb::Renderer
{
public:
	SDLRenderer(int width, int height, int scalingFactor = 1)
		: m_textureWidth(width)
		, m_textureHeight(height)
		, m_windowWidth(width* scalingFactor)
		, m_windowHeight(height* scalingFactor)
		, m_scaling(scalingFactor)
	{
		SDL_CreateWindowAndRenderer(m_windowWidth, m_windowHeight, 0, &m_window, &m_renderer);
		SDL_SetWindowTitle(m_window, "GGBoy");
		m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, m_textureWidth, m_textureHeight);
		m_textureTransform = { 0,0, m_windowWidth, m_windowHeight };
	}

	~SDLRenderer()
	{
		SDL_DestroyRenderer(m_renderer);
		SDL_DestroyWindow(m_window);
		SDL_DestroyTexture(m_texture);
	}

	virtual void renderNewFrame(const ggb::FrameBuffer& framebuffer)
	{
		startRendering();

		for (int x = 0; x < framebuffer.m_buffer.size(); x++)
		{
			for (int y = 0; y < framebuffer.m_buffer[0].size(); y++)
			{
				const auto rgba = framebuffer.m_buffer[x][y];

				const uint32_t pixelPosition = (y * m_pitch) + x * 3;
				m_lockedPixels[pixelPosition] = rgba.r;
				m_lockedPixels[pixelPosition + 1] = rgba.g;
				m_lockedPixels[pixelPosition + 2] = rgba.b;
			}
		}
		finishRendering();
	}

	void startRendering()
	{
		SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
		SDL_RenderClear(m_renderer);
		m_pitch = 0;
		m_lockedPixels = nullptr;
		SDL_LockTexture(m_texture, nullptr, reinterpret_cast<void**>(&m_lockedPixels), &m_pitch);
	};

	void finishRendering()
	{
		SDL_UnlockTexture(m_texture);
		SDL_RenderCopy(m_renderer, m_texture, nullptr, &m_textureTransform);
		SDL_RenderPresent(m_renderer);
	};

private:
	SDL_Window* m_window = nullptr;
	SDL_Renderer* m_renderer = nullptr;
	SDL_Texture* m_texture = nullptr;
	SDL_Rect m_textureTransform = {};
	uint8_t* m_lockedPixels = nullptr;

	int m_textureWidth = 0;
	int m_textureHeight = 0;
	int m_windowWidth = 0;
	int m_windowHeight = 0;
	int m_pitch = 0;
	int m_scaling = 1;
};




class InputHandler : public ggb::Input
{
public:
	InputHandler() = default;

	bool isAPressed() override
	{
		return m_keyStates[SDL_SCANCODE_O];
	}
	bool isBPressed() override
	{
		return m_keyStates[SDL_SCANCODE_P];
	}
	bool isStartPressed() override
	{
		return m_keyStates[SDL_SCANCODE_SPACE];
	}
	bool isSelectPressed() override
	{
		return m_keyStates[SDL_SCANCODE_RETURN];
	}
	bool isUpPressed() override
	{
		return m_keyStates[SDL_SCANCODE_W];
	}
	bool isDownPressed() override
	{
		return m_keyStates[SDL_SCANCODE_S];
	}
	bool isLeftPressed() override
	{
		return m_keyStates[SDL_SCANCODE_A];
	}
	bool isRightPressed() override
	{
		return m_keyStates[SDL_SCANCODE_D];
	}
private:
	const Uint8* m_keyStates = SDL_GetKeyboardState(nullptr);
};

//static void audio_callback(void* userdata, uint8_t* stream, int len)
//{
//	uint64_t* samples_played = (uint64_t*)userdata;
//	float* fstream = (float*)(stream);
//
//	static const float volume = 0.05;
//	static float frequency = 200;
//	static float sign = -1;
//
//	//if (frequency <= 100.f || frequency >= 10000.f)
//	//	sign *= -1;
//	//frequency += sign * 0.1;
//	//frequency = std::clamp(frequency, 100.f, 10000.f);
//
//	for (int sid = 0; sid < (len / 8); ++sid)
//	{
//		double time = (*samples_played + sid) / 44100.0;
//		double x = 2.0 * M_PI * time * frequency;
//		double signal = sin(x);
//		if (signal < 0.5)
//			signal = 0.0;
//		else
//			signal = 1.0;
//		double outputValue = signal * volume;
//
//		fstream[2 * sid + 0] = outputValue; /* L */
//		fstream[2 * sid + 1] = outputValue; /* R */
//	}
//
//	*samples_played += (len / 8);
//}

//static void test_audio_callback(void* userdata, uint8_t* stream, int len)
//{
//	static int counter = 0;
//	float* fstream = reinterpret_cast<float*>(stream);
//	counter++;
//	int sample = 0;
//	if (counter == 3)
//	{
//		counter = 0;
//		sample = 1;
//	}
//
//	const auto count = len / 8;
//	for (int sid = 0; sid < count; ++sid)
//	{
//		fstream[2 * sid + 0] = sample; /* L */
//		fstream[2 * sid + 1] = sample; /* R */
//	}
//}
static constexpr int CHANNEL_COUNT = 2;

static void emulator_audio_callback(void* userdata, uint8_t* stream, int len)
{
	ggb::SampleBuffer* sampleBuffer = reinterpret_cast<ggb::SampleBuffer*>(userdata);
	auto audioStream = reinterpret_cast<ggb::AUDIO_FORMAT*>(stream);

	static const int volume = 60;
	const auto count = len / (sizeof(ggb::AUDIO_FORMAT) * CHANNEL_COUNT);

	for (size_t sid = 0; sid < count; ++sid)
	{
		auto frame = sampleBuffer->pop(ggb::Frame{});

		audioStream[2 * sid + 0] = frame.leftSample * volume; /* L */
		audioStream[2 * sid + 1] = frame.rightSample * volume; /* R */
	}
}

static bool intializeAudio(ggb::Emulator* emu)
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
	audio_spec_want.userdata = static_cast<void*>(emu->getSampleBuffer());

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

static void handleEmulatorKeyPresses(ggb::Emulator* emulator, const Uint8* keyStates)
{
	if (keyStates[SDL_SCANCODE_R])
		emulator->reset();
	if (keyStates[SDL_SCANCODE_F1])
		emulator->saveEmulatorState("Savestate1.bin");
	if (keyStates[SDL_SCANCODE_F2])
		emulator->saveEmulatorState("Savestate2.bin");
	if (keyStates[SDL_SCANCODE_F3])
		emulator->saveEmulatorState("Savestate3.bin");
	if (keyStates[SDL_SCANCODE_F4])
		emulator->saveEmulatorState("Savestate4.bin");
	if (keyStates[SDL_SCANCODE_F5])
		emulator->loadEmulatorState("Savestate1.bin");
	if (keyStates[SDL_SCANCODE_F6])
		emulator->loadEmulatorState("Savestate2.bin");
	if (keyStates[SDL_SCANCODE_F7])
		emulator->loadEmulatorState("Savestate3.bin");
	if (keyStates[SDL_SCANCODE_F8])
		emulator->loadEmulatorState("Savestate4.bin");
}

int main(int argc, char* argv[])
{
	auto emulator = ggb::Emulator();
	auto appInputHandling = std::make_unique<InputHandler>();
	auto tileDataDimensions = emulator.getTileDataDimensions();
	auto gameWindowDimensions = emulator.getGameWindowDimensions();
	//auto tileDataRenderer = std::make_unique<SDLRenderer>(tileDataDimensions.width, tileDataDimensions.height, 4);
	auto gameWindowRenderer = std::make_unique<SDLRenderer>(gameWindowDimensions.width, gameWindowDimensions.height, 5);
	const Uint8* keyStates = SDL_GetKeyboardState(nullptr);

	//emulator.loadCartridge("Roms/Games/Dr.Mario.gb");
	//emulator.loadCartridge("Roms/Games/Tetris.gb");
	//emulator.loadCartridge("Roms/Games/Super_Mario_Land.gb");
	emulator.loadCartridge("Roms/Games/Legend_of_Zelda_Link's_Awakening.gb");
	//emulator.loadCartridge("Roms/Games/Pokemon_Yellow.gb");
	//emulator.loadCartridge("Roms/TestROMs/interrupt_time.gb");
	//emulator.loadCartridge("Roms/TestROMs/instr_timing.gb");
	//emulator.loadCartridge("Roms/TestROMs/cpu_instrs.gb");
	//emulator.loadCartridge("Roms/TestROMs/01-special.gb");
	//emulator.loadCartridge("Roms/TestROMs/02-interrupts.gb");
	//emulator.loadCartridge("Roms/TestROMs/03-op sp,hl.gb");
	//emulator.loadCartridge("Roms/TestROMs/04-op r,imm.gb");
	//emulator.loadCartridge("Roms/TestROMs/05-op rp.gb");
	//emulator.loadCartridge("Roms/TestROMs/06-ld r,r.gb");
	//emulator.loadCartridge("Roms/TestROMs/07-jr,jp,call,ret,rst.gb");
	//emulator.loadCartridge("Roms/TestROMs/08-misc instrs.gb");
	//emulator.loadCartridge("Roms/TestROMs/09-op r,r.gb");
	//emulator.loadCartridge("Roms/TestROMs/10-bit ops.gb");
	//emulator.loadCartridge("Roms/TestROMs/11-op a,(hl).gb");
	//emulator.loadCartridge("Roms/TestROMs/cgb_sound.gb");
	//emulator.setTileDataRenderer(std::move(tileDataRenderer));
	emulator.setGameRenderer(std::move(gameWindowRenderer));
	emulator.setInput(std::move(appInputHandling));
	intializeAudio(&emulator);

	static constexpr int UPDATE_INPUT_TICKS = 10;
	int counter = 0;
	while (true)
	{
		emulator.step();
		if (counter++ == UPDATE_INPUT_TICKS)
		{
			counter -= UPDATE_INPUT_TICKS;
			SDL_PumpEvents(); // Don't pump events on every step -> not really needed and improves performance
			handleEmulatorKeyPresses(&emulator, keyStates);
		}
	}

	return 0;
}