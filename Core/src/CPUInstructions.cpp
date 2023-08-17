#include "CPUInstructions.hpp"

#include <cassert>
#include <exception>

void ggb::invalidInstruction(CPUState* cpu, BUS* bus)
{
	assert(!"Tried to execute invalid or not yet implemented instruction");
	throw std::exception("Tried to execute invalid or not yet implemented instruction");
}

void ggb::noop(CPUState* cpu, BUS* bus)
{
}
