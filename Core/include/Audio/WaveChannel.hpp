#pragma once
#include "BUS.hpp"
#include "Constants.hpp"
#include "Serialization.hpp"

namespace ggb 
{
	class WaveChannel 
	{
	public:
		WaveChannel(BUS* bus);
		void setBus(BUS* bus);
		void step(int cyclesPassed);
		bool write(uint16_t address, uint8_t value);
		AUDIO_FORMAT getSample() const;
		void tickLengthShutdown();
		void serialization(Serialization* serialization);

	private:
		void trigger();
		int getPeriodCounter() const;
		bool isLengthShutdownEnabled() const;
		uint8_t getInitialLengthCounter() const;
		int getOutputLevel() const;

		bool m_isOn = false;
		int m_sampleIndex = 0;
		int m_periodCounter = 0;
		int m_lengthCounter = 0;
		uint8_t* m_enabled = nullptr;
		uint8_t* m_lengthTimer = nullptr;
		uint8_t* m_outputLevel = nullptr;
		uint8_t* m_periodLow = nullptr;
		uint8_t* m_periodHighAndControl = nullptr;
		uint8_t* m_waveRamStart = nullptr;
	};
}