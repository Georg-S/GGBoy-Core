#include "Utility.hpp"

#include <chrono>

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

long long ggb::getCurrentTimeInNanoSeconds()
{
	auto current_time = std::chrono::system_clock::now();
	return std::chrono::time_point_cast<std::chrono::nanoseconds>(current_time).time_since_epoch().count();
}

bool ggb::memcpySecure(void* dest, size_t destSize, const void* source, size_t size)
{
    if (destSize < size)
    {
        assert(false);
        return false;
    }
    memcpy(dest, source, size);
    return true;
}