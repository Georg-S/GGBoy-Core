#pragma once
#include <optional>

#include "BUS.hpp"
#include "Constants.hpp"
#include "Serialization.hpp"

namespace ggb
{
	class NoiseChannel
	{
	public:
		NoiseChannel(BUS* bus);
		void setBus(BUS* bus);
		bool write(uint16_t address, uint8_t value);
		std::optional<uint8_t> read(uint16_t address) const;
		void step(int cyclesPassed);
		AUDIO_FORMAT getSample();
		void tickVolumeEnvelope();
		void tickLengthShutdown();
		bool isOn() const;
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
		bool m_isOn = false;
		int m_cycleCounter = 0;
		bool m_volumeChange = false;
		uint16_t m_lfsr = 0xFFFFu; // LFSR = Linear-feedback shift register
		uint8_t* m_lengthTimer = nullptr;
		uint8_t* m_volumeAndEnvelope = nullptr;
		uint8_t* m_frequencyAndRandomness = nullptr;
		uint8_t* m_control = nullptr;
	};
}