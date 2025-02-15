#pragma once
#include <optional>

#include "BUS.hpp"
#include "Constants.hpp"
#include "Serialization.hpp"
#include "AudioCommon.hpp"

namespace ggb
{
	class NoiseChannel : public AudioChannel
	{
	public:
		NoiseChannel(BUS* bus);
		void step(int cyclesPassed);
		void setBus(BUS* bus) override;
		bool write(uint16_t address, uint8_t value) override;
		std::optional<uint8_t> read(uint16_t address) const override;
		AUDIO_FORMAT getSample() const override;
		bool isChannelAddress(uint16_t address) const override;
		void tickLengthShutdown() override;
		void tickVolumeEnvelope();
		void serialization(Serialization* serialization);

	private:
		int getInitialLengthCounter() const;
		int getInitialVolume() const;
		void stepLFSR();
		void trigger();
		bool isLengthShutdownEnabled() const;
		bool isLFSR7Bit() const;
		int getClockShift() const;
		int getClockDivider() const;
		void resetLFSR();

		int m_volumeSweepCounter = 0;
		int m_volume = 0;
		int m_lengthCounter = 0;
		int m_cycleCounter = 0;
		bool m_volumeChange = false;
		uint16_t m_lfsr = 0xFFFFu; // LFSR = Linear-feedback shift register
		uint8_t* m_lengthTimer = nullptr;
		uint8_t* m_volumeAndEnvelope = nullptr;
		uint8_t* m_frequencyAndRandomness = nullptr;
		uint8_t* m_control = nullptr;
	};
}