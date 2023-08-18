#include "CPUInstructions.hpp"

#include "CPU.hpp"

#include <cassert>
#include <exception>

using namespace ggb;

static uint16_t readTwoBytes(CPUState* cpu, BUS* bus) 
{
	auto result = bus->readTwoBytes(cpu->InstructionPointer());
	cpu->InstructionPointer() += 2;
	return result;
}


void ggb::invalidInstruction(CPUInstructionParameters)
{
	assert(!"Tried to execute invalid or not yet implemented instruction");
	throw std::exception("Tried to execute invalid or not yet implemented instruction");
}

void ggb::noop(CPUInstructionParameters)
{
}

void ggb::loadBCValue(CPUInstructionParameters) 
{
	cpu->BC() = readTwoBytes(cpu, bus);
}

void ggb::writeAToAddressBC(CPUInstructionParameters) 
{
	bus->write(cpu->BC(), cpu->A());
}

void ggb::incrementA(CPUInstructionParameters)
{
}

void ggb::jumpToValue(CPUInstructionParameters) 
{
}

void ggb::xorASelf(CPUInstructionParameters) 
{
}
