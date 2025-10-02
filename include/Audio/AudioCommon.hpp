#pragma once
#include <optional>

#include "Serialization.hpp"
#include "Constants.hpp"
#include "BUS.hpp"

namespace ggb 
{
	class AudioChannel 
	{
	public:
		virtual ~AudioChannel() = default;
		virtual bool write(uint16_t memory, uint8_t value) = 0;
		virtual std::optional<uint8_t> read(uint16_t address) const = 0;
		// Step is called too frequently to call it via dynamic dispatch, therefore don't make it virtual
		//virtual void step(int cyclesPassed) = 0;
		virtual AUDIO_FORMAT getSample() const = 0;
		virtual void setBus(BUS* bus) = 0;
		virtual bool isChannelAddress(uint16_t address) const = 0;
		bool isOn() const;
		bool isMuted() const;
		void mute(bool mute); // This has nothing to do with the originial gameboy (just an emulator feature)
		virtual void serialization(Serialization* serialization);
		virtual void reset();
		virtual void tickLengthShutdown() = 0; 

	protected:
		bool m_isOn = false;
		bool m_muted = false;
	};
}