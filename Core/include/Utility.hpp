#pragma once
#include <cassert>
#include <cstdint>
#include <fstream>
#include <vector>

namespace ggb 
{
	uint16_t combineUpperAndLower(uint8_t upper, uint8_t lower);
	void swap(uint16_t& num);
	void swap(uint8_t& num);
	long long getCurrentTimeInNanoSeconds();

	/// Returns the lower 4 bits
	constexpr uint8_t lowerNibble(uint8_t number) 
	{
		return number & 0xF;
	}

	/// Returns the upper 4 bits
	constexpr uint8_t upperNibble(uint8_t number) 
	{
		return number & 0xF0;
	}

	template<typename T>
	constexpr void setBit(T& number, int bit) 
	{
		assert(bit < (sizeof(T) * 8));
		const T toSetBit = 1 << bit;
		number |= toSetBit;
	}

	constexpr uint8_t getNumberFromBits(bool bitZero, bool bitOne)
	{
		return static_cast<uint8_t>((bitOne << 1) | static_cast<uint8_t>(bitZero));
	}

	template<typename T>
	constexpr void clearBit(T& number, int bit)
	{
		assert(bit < (sizeof(T) * 8));
		const T bitMask = ~(1 << bit);
		number &= bitMask;
	}

	template<typename T>
	constexpr void setBitToValue(T& out, int bit, bool value)
	{
		assert(bit < (sizeof(T) * 8));
		out = (out & ~(1 << bit)) | (value << bit); // Clear the bit first and then bitwise or it with the bit value
	}

	template<typename T>
	constexpr bool isBitSet(T number, unsigned int bit)
	{
		assert(bit < (sizeof(T) * 8));
		const T bitToCheck = 1 << bit;
		return (number & bitToCheck) == bitToCheck;
	}
} 