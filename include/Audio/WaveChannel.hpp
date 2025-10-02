#pragma once

#include <optional>

#include "BUS.hpp"
#include "Constants.hpp"
#include "Serialization.hpp"
#include "AudioCommon.hpp"

namespace ggb 
{
	class WaveChannel : public AudioChannel
	{
	public:
		WaveChannel(BUS* bus);
		void step(int cyclesPassed);
		void setBus(BUS* bus) override;
		bool write(uint16_t address, uint8_t value) override;
		std::optional<uint8_t> read(uint16_t address) const override;
		AUDIO_FORMAT getSample() const override;
		bool isChannelAddress(uint16_t address) const override;
		void tickLengthShutdown() override;
		void serialization(Serialization* serialization) override;
		void reset() override;

	private:
		void trigger();
		int getPeriodCounter() const;
		bool isLengthShutdownEnabled() const;
		uint8_t getInitialLengthCounter() const;
		int getOutputLevel() const;

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