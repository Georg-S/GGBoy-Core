#pragma once

#include <optional>

#include "AudioCommon.hpp"
#include "Constants.hpp"
#include "BUS.hpp"
#include "Ringbuffer.hpp"
#include "Serialization.hpp"

namespace ggb
{
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

	class SquareWaveChannel : public AudioChannel
	{
	public:
		SquareWaveChannel(bool hasSweep, BUS* bus);
		void step(int cyclesPassed);
		void setBus(BUS* bus) override;
		bool write(uint16_t memory, uint8_t value) override;
		std::optional<uint8_t> read(uint16_t address) const override;
		AUDIO_FORMAT getSample() const override;
		bool isChannelAddress(uint16_t address) const override;
		void tickLengthShutdown() override;
		void tickVolumeEnvelope();
		void tickFrequencySweep();
		void serialization(Serialization* serialization) override;

	private:
		void trigger();
		bool isLengthShutdownEnabled() const;
		uint16_t getPeriodValue() const;
		uint16_t getInitialPeriodCounter() const; // In CPU cycles
		int getUsedDutyCycleIndex() const;
		int getInitialLengthCounter() const;
		int getInitialVolume() const;
		int getInitialFrequencySweepPace() const;
		void setRawPeriodValue(uint16_t val);

		int m_dutyCyclePosition = 0;
		uint16_t m_baseAddres = AUDIO_CHANNEL_1_FREQUENCY_SWEEP_ADDRESS;
		int m_periodCounter = 0;
		int m_lengthCounter = 0;
		int m_volumeSweepCounter = 0;
		int m_volume = 0;
		int m_frequencySweepCounter = 0;
		int m_frequencySweepPace = 0;
		bool m_hasSweep = true;
		bool m_volumeChange = false;
		uint8_t* m_sweep = nullptr;
		uint8_t* m_lengthTimerAndDutyCycle = nullptr;
		uint8_t* m_volumeAndEnvelope = nullptr;
		uint8_t* m_periodLow = nullptr;
		uint8_t* m_periodHighAndControl = nullptr;

		static constexpr int FREQUENCY_SWEEP_OFFSET = 0;
		static constexpr int LENGTH_TIMER_OFFSET = 1;
		static constexpr int VOLUME_OFFSET = 2;
		static constexpr int PERIOD_LOW_OFFSET = 3;
		static constexpr int PERIOD_HIGH_AND_CONTROL_OFFSET = 4;
		static constexpr uint16_t PERIOD_LOW_BITMASK = 0b111;
	};
}