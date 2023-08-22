#pragma once

#include <cassert>
#include <cstdint>

namespace ggb 
{
	uint16_t combineUpperAndLower(uint8_t upper, uint8_t lower);

	template<typename T>
	constexpr void setBit(T& number, int bit) 
	{
		assert(bit < (sizeof(T) * 8));
		const T toSetBit = 1 << bit;
		number |= toSetBit;
	}

	constexpr uint8_t getNumberFromBits(bool bitZero, bool bitOne)
	{
		return static_cast<uint8_t>((bitOne << 1) | static_cast<int>(bitZero));
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
		// TODO double check if this is correct
		out = (out & ~(1 << bit)) | (value << bit);
	}

	template<typename T>
	constexpr bool isBitSet(T number, int bit)
	{
		assert(bit < (sizeof(T) * 8));
		const T bitToCheck = 1 << bit;
		return (number & bitToCheck) == bitToCheck;
	}

}