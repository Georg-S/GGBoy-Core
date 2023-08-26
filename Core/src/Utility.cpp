#include "Utility.hpp"

uint16_t ggb::combineUpperAndLower(uint8_t upper, uint8_t lower)
{
	return (static_cast<uint16_t>(upper) << 8) | lower;
}

void ggb::swap(uint16_t& num)
{
	const uint16_t upper = num >> 8;
	const uint16_t lower = num << 8;
	num = lower | upper;
}

void ggb::swap(uint8_t& num)
{
	const uint8_t upper = num >> 4;
	const uint8_t lower = num << 4;
	num = lower | upper;
}
