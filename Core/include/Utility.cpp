#include "Utility.hpp"

uint16_t ggb::combineUpperAndLower(uint8_t upper, uint8_t lower)
{
	return (static_cast<int>(upper) << 8) | lower;
}
