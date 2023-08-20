#include "CPU.hpp"

#include "CPUInstructions.hpp"
#include "Logging.hpp"

void ggb::CPU::reset()
{
	m_cpuState.setZeroFlag(true);
	m_cpuState.setSubtractionFlag(false);
	m_cpuState.A() = 0x01;
	m_cpuState.B() = 0x00;
	m_cpuState.C() = 0x13;
	m_cpuState.D() = 0x00;
	m_cpuState.E() = 0xD8;
	m_cpuState.H() = 0x01;
	m_cpuState.L() = 0x4D;
	m_cpuState.InstructionPointer() = 0x100;
	m_cpuState.StackPointer() = 0xFFFE;
}

void ggb::CPU::step()
{
	if (m_cpuState.interruptsEnabled())
		handleInterrupts();

	const int instructionPointer = m_cpuState.InstructionPointer();
	auto opCode = m_bus->read(instructionPointer);
	logInfo(m_opcodes.getMnemonic(opCode) + "\t \t" + std::to_string(m_cpuState.B()));
	++m_cpuState.InstructionPointer();
	m_opcodes.execute(opCode, &m_cpuState, m_bus);
	++m_instructionCounter;
}

void ggb::CPU::setBus(BUS* bus)
{
	m_bus = bus;
}

void ggb::CPU::handleInterrupts()
{
}
