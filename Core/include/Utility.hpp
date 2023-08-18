#pragma once

namespace ggb 
{
	template<typename T>
	constexpr void setBit(T& number, int bit) 
	{
		constexpr T toSetBit = T(1) << bit;
		number |= toSetBit;
	}

	template<typename T>
	constexpr void clearBit(T& number, int bit)
	{
		constexpr T bitMask = ~(T(1) << bit);
		number &= bitMask;
	}

	template<typename T>
	constexpr void setBitToValue(T& out, int bit, bool value)
	{
		// TODO double check if this is correct
		out = out & ~(1 << bit) | (value << bit);
	}

	template<typename T>
	constexpr bool isBitSet(T number, int bit)
	{
		const T bitToCheck = T(1) << bit;
		return (number & bitToCheck) == bitToCheck;
	}
}