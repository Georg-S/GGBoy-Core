#include "Audio/AudioCommon.hpp"

bool ggb::AudioChannel::isOn() const
{
	return m_isOn;
}

void ggb::AudioChannel::serialization(Serialization* serialization)
{
	serialization->read_write(m_isOn);
}
