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
    bool memcpySecure(void* dest, size_t destSize, const void* source, size_t size);

	/// Returns the lower 4 bits
	inline constexpr uint8_t lowerNibble(uint8_t number) 
	{
		return number & 0xF;
	}

	/// Returns the upper 4 bits
	inline constexpr uint8_t upperNibble(uint8_t number) 
	{
		return number & 0xF0;
	}

	inline constexpr uint8_t getNumberFromBits(bool bitZero, bool bitOne)
	{
		return static_cast<uint8_t>((bitOne << 1) | static_cast<uint8_t>(bitZero));
	}

	template<typename T>
	inline constexpr void setBit(T& number, int bit)
	{
		number |= bit;
	}

	template<int bitNumber, typename T>
	inline constexpr void setBit(T& number) 
	{
		static_assert(bitNumber < sizeof(T) * 8);
		constexpr T toSetBit = 1 << bitNumber;
		number |= toSetBit;
	}

	template<typename T>
	inline constexpr void clearBit(T& number, int bitValue)
	{
		const auto bitMask = ~bitValue;
		number &= bitMask;
	}

	template <int bitNumber, typename T>
	inline constexpr void clearBit(T& number) 
	{
		static_assert(bitNumber < sizeof(T) * 8);
		constexpr T bitMask = ~(1 << bitNumber);
		number &= bitMask;
	}

	template<int bitNumber, typename T>
	inline constexpr void setBitToValue(T& out, bool value)
	{
		static_assert(bitNumber < sizeof(T) * 8);
		out = (out & ~(1 << bitNumber)) | (value << bitNumber); // Clear the bit first and then bitwise or it with the bit value
	}

	template<int bitNumber, typename T>
	inline constexpr bool isBitSet(T number)
	{
		static_assert(bitNumber < sizeof(T) * 8);
		constexpr T bitToCheck = 1 << bitNumber;
		return static_cast<bool>(number & bitToCheck);
	}

	template<typename T>
	inline constexpr bool isBitSet(T number, int bit)
	{
		return static_cast<bool>(number & bit);
	}
} 