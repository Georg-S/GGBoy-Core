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
		void setBus(BUS* bus);
		int step();
		void serialization(Serialization* serialization); // Used for both serialize / deserialize

	private:
		bool handleInterrupts();

		BUS* m_bus = nullptr;
		OPCodes m_opcodes; // TODO maybe not make this part of cpu but instead a static variable
		CPUState m_cpuState;
		uint8_t* m_requestedInterrupts = nullptr;
		const uint8_t* m_enabledInterrupts = nullptr;
	};
}