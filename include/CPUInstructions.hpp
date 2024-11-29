#pragma once
#include <vector>

#include "CPUState.hpp"
#include "BUS.hpp"

namespace ggb
{
#define CPUInstructionParameters CPUState* cpu, BUS* bus, bool* branchTaken
	// It is possible to use std::function here, however debugging is easier with plain C function pointers (and no performance overhead)
	using InstructionFunction = void (*)(CPUInstructionParameters);

	void callAddress(CPUState* cpu, BUS* bus, uint16_t address);

	class OPCodes
	{
	public:
		struct OPCode
		{
			int id = -1;
			InstructionFunction func = nullptr;
			int baseCycleCount = 0;
			std::string mnemonic;
			int branchCycleCount = 0;
		};
		OPCodes();
		int execute(uint16_t opCode, ggb::CPUState* cpu, ggb::BUS* bus) const;
		const std::string& getMnemonic(uint16_t opCode) const;
	private:
		void setOpcode(OPCode&& opcode);
		void setExtendedOpcode(OPCode&& opcode);
		void initOpcodesArray();

		std::vector<OPCode> m_opcodes;
		std::vector<OPCode> m_extendedOpcodes;
		int m_counter = 0;
		int m_extendedOpcodeCounter = 0;
	};
}