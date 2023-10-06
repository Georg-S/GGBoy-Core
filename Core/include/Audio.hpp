#pragma once

#include "BUS.hpp"

namespace ggb
{
	static constexpr int DUTY_CYCLES[4][8]
	{
		{0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,1},
		{1,0,0,0,0,1,1,1},
		{0,1,1,1,1,1,1,0}
	};

	struct SquareWaveChannel
	{
		SquareWaveChannel(bool hasSweep, BUS* bus);
		void setBus(BUS* bus);
		void write(uint16_t memory, uint8_t value);
		void step(int cyclesPassed);

	private:
		int m_dutyCycleIndex = 0;
		uint16_t m_baseAddres = 0xFF10;
		bool m_hasSweep = true;
		bool m_isOn = false;
		uint8_t* m_sweep = nullptr;
		uint8_t* m_lengthTimerAndDutyCycle = nullptr;
		uint8_t* m_volumeAndEnvelope = nullptr;
		uint8_t* m_periodLow = nullptr;
		uint8_t* m_periodHighAndControl = nullptr;

		static constexpr int LENGTH_TIMER_OFFSET = 1;
		static constexpr int VOLUME_OFFSET = 2;
		static constexpr int PERIOD_LOW_OFFSET = 3;
		static constexpr int PERIOD_HIGH_OFFSET = 4;
	};


	class Audio
	{
	public:
		Audio(BUS* bus);
		void setBus(BUS* bus);
		void write(uint16_t address, uint8_t value);
		void step(int cyclesPassed);

	private:
		std::unique_ptr<SquareWaveChannel> m_channel2 = nullptr;
		uint8_t* m_soundOn = nullptr;
		uint8_t* m_soundPanning = nullptr;
		uint8_t* m_masterVolume = nullptr;
	};
}