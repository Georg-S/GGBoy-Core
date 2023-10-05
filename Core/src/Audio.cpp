#include "Audio.hpp"

#include "Utility.hpp"
#include "Constants.hpp"

ggb::Audio::Audio(BUS* bus)
{
	setBus(bus);
}

void ggb::Audio::setBus(BUS* bus)
{
	m_masterVolume = bus->getPointerIntoMemory(AUDIO_MASTER_VOLUME_ADDRESS);
	m_soundPanning = bus->getPointerIntoMemory(AUDIO_PANNING_ADDRESS);
	m_soundOn = bus->getPointerIntoMemory(AUDIO_MAIN_STATE_ADDRESS);
}

void ggb::Audio::step(int cyclesPassed)
{
	if (!isBitSet(*m_soundOn, 7))
		return; // TODO reset state?

	int b = 3; 

}
