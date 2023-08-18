#pragma once
#include <array>

#include "CPUState.hpp"
#include "BUS.hpp"

namespace ggb
{
	//class BUS;
	// TODO: Is this really cleaner than having it in every method???
#define CPUInstructionParameters CPUState* cpu, BUS* bus
	// It is possible to use std::function here, however debugging is easier with plain C function pointers (and no performance overhead)
	using InstructionFunction = void (*)(CPUInstructionParameters);

	class OPCodes 
	{
	public:
		struct OPCode 
		{
			int id;
			InstructionFunction func;
			int baseCycleCount;
		};
		OPCodes();
		void execute(uint16_t opCode, ggb::CPUState* cpu, ggb::BUS* bus);
	private:
		void setOpcode(OPCode&& opcode);
		void initOpcodesArray();

		std::array<OPCode, 512> m_opcodes;
		int m_counter = 0;
	};
}