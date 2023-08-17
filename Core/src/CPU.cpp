#include "CPU.hpp"
#include "CPUInstructions.hpp"

void ggb::CPU::reset()
{
	m_cpuState.Flags.zero = true;
	m_cpuState.Flags.subtraction = false;
	m_cpuState.A = 0x01;
	m_cpuState.B = 0x00;
	m_cpuState.C = 0x13;
	m_cpuState.D = 0x00;
	m_cpuState.E = 0xD8;
	m_cpuState.E = 0x01;
	m_cpuState.L = 0x4D;
	m_cpuState.instructionPointer = 0x100;
	m_cpuState.stackPointer = 0xFFFE;
}

void ggb::CPU::executeOneInstruction()
{
	auto test = sizeof(CPUState::FlagRegister);

	auto opCode = m_bus->read(m_cpuState.instructionPointer);
	++m_cpuState.instructionPointer;
	ggb::opcodes[opCode](&m_cpuState, m_bus); // Execute instruction

	int buf = 3;
}

void ggb::CPU::setBus(BUS* bus)
{
	m_bus = bus;
}
