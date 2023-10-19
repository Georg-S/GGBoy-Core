#pragma once

#include "Constants.hpp"
#include "BUS.hpp"
#include "Ringbuffer.hpp"

namespace ggb
{
	using AUDIO_FORMAT = int16_t;
	static constexpr int DUTY_CYCLE_COUNT = 4;
	static constexpr int DUTY_CYCLE_LENGTH = 8;
	static constexpr int LENGTH_FREQUENCY = 256; // In hz
	static constexpr int VOLUME_FREQUENCY = 64; // In hz
	static constexpr int CPU_CLOCKS_PER_LENGTH_INCREASE = CPU_BASE_CLOCK / LENGTH_FREQUENCY;
	static constexpr auto CPU_CLOCKS_PER_PERIOD_INCREASE = CPU_BASE_CLOCK / PERIOD_DIVIDER_CLOCK;
	static constexpr auto CPU_CLOCKS_PER_VOLUME_INCREASE = CPU_BASE_CLOCK / VOLUME_FREQUENCY;
	static constexpr int DUTY_CYCLES[DUTY_CYCLE_COUNT][DUTY_CYCLE_LENGTH]
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
		void trigger();
		AUDIO_FORMAT getSample() const;
		void tickVolumeEnvelope();
		void tickLengthShutdown();

	private:
		bool isLengthShutdownEnabled() const;
		uint16_t getPeriodValue() const;
		int getUsedDutyCycleIndex() const;
		int getInitialLengthCounter() const;
		int getInitialVolume() const;

		int m_dutyCyclePosition = 0;
		uint16_t m_baseAddres = 0xFF10;
		uint16_t m_periodDivider = 0;
		int m_periodCounter = 0; // TODO find better name
		int m_lengthCounter = 0;
		int m_volumeSweepCounter = 0;
		int m_volume = 0;
		bool m_hasSweep = true;
		bool m_isOn = false;
		bool m_volumeChange = false;
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
}