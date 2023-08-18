#pragma once
#include <cstdint>
#include "BUS.hpp"
#include "CPUState.hpp"

namespace ggb
{
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