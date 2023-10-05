#pragma once

#include "BUS.hpp"

namespace ggb 
{
	struct AudioChannel 
	{

	};


	class Audio 
	{
	public:
		Audio(BUS* bus);
		void setBus(BUS* bus);

		void step(int cyclesPassed);
	private:
		uint8_t* m_soundOn = nullptr;
		uint8_t* m_soundPanning = nullptr;
		uint8_t* m_masterVolume = nullptr;
	};
}