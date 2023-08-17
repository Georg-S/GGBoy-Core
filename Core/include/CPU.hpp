#pragma once
#include <cstdint>
#include "BUS.hpp"

namespace ggb
{
	struct CPUState
	{
		struct FlagRegister 
		{
			// TODO: Is this order correct or should it be reversed?
			uint8_t unused : 4;
			uint8_t carry : 1;
			uint8_t halfCarry : 1;
			uint8_t subtraction : 1;
			uint8_t zero : 1;
		};
		static_assert(sizeof(FlagRegister) == 1);

		uint8_t A = 0;
		FlagRegister Flags = {}; // Flags
		uint8_t B = 0;
		uint8_t C = 0;
		uint8_t D = 0;
		uint8_t E = 0;
		uint8_t H = 0;
		uint8_t L = 0;
		uint16_t stackPointer = 0;
		uint16_t instructionPointer = 0;
	};

	class CPU
	{
	public:
		void reset();
		void executeOneInstruction();
		void setBus(BUS* bus);

	private:
		CPUState m_cpuState;
		BUS* m_bus = nullptr;
	};
}