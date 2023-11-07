#pragma once
#include <Emulator.hpp>

class Audio 
{
public:
	Audio(ggb::SampleBuffer* sampleBuffer);
	~Audio();
private:
	bool initializeAudio(ggb::SampleBuffer* sampleBuffer);
};