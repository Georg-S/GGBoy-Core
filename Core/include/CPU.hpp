#pragma once
#include <cstdint>
#include "BUS.hpp"
#include "CPUState.hpp"
#include "CPUInstructions.hpp"

namespace ggb
{
	class CPU
	{
	public:
		void reset();
		void step();
		void setBus(BUS* bus);

	private:
		void handleInterrupts();

		OPCodes m_opcodes;
		CPUState m_cpuState;
		BUS* m_bus = nullptr;
		uint64_t m_instructionCounter = 0;
	};
}