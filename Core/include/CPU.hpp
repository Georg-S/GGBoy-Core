#pragma once
#include <cstdint>
#include "BUS.hpp"

namespace ggb
{
	struct CPUState
	{
		uint8_t A = 0;
		uint8_t F = 0; // Flags
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
		void executeOneInstruction();
		void setBus(BUS* bus);

	private:
		CPUState m_cpuState;
		BUS* m_bus = nullptr;
	};
}