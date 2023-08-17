#include "CPU.hpp"
#include "CPUInstructions.hpp"

void ggb::CPU::executeOneInstruction()
{
	auto opCode = m_bus->read(m_cpuState.instructionPointer);
	++m_cpuState.instructionPointer;
	ggb::opcodes[opCode](&m_cpuState, m_bus); // Execute instruction

	int buf = 3;
}

void ggb::CPU::setBus(BUS* bus)
{
	m_bus = bus;
}
