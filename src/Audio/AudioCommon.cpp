#include "Audio/AudioCommon.hpp"

bool ggb::AudioChannel::isOn() const
{
	return m_isOn;
}

bool ggb::AudioChannel::isMuted() const
{
	return m_muted;
}

void ggb::AudioChannel::mute(bool mute)
{
	m_muted = mute;
}

void ggb::AudioChannel::serialization(Serialization* serialization)
{
	serialization->read_write(m_isOn);
}
