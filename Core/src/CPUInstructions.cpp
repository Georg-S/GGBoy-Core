#include "CPUInstructions.hpp"

#include "CPU.hpp"

#include <cassert>
#include <exception>

using namespace ggb;

static constexpr uint16_t combine(uint8_t val1, uint8_t val2) 
{
	return (uint16_t(val1) << 8) | val2;
}

static void increment(CPUState* cpuState, uint8_t* toIncrement) 
{
	cpuState->Flags.subtraction = false;
	// TODO: is this implementation of the half carry handling correct?
	//cpuState->Flags.halfCarry = ((*toIncrement & uint8_t(0xFF)) + 1) > 0xFF;
	cpuState->Flags.halfCarry = (*toIncrement == 0xFF);
	++(*toIncrement);
	cpuState->Flags.zero = (*toIncrement == 0);
}

static void jump(CPUState* cpuState, uint16_t jumpAddress) 
{
	cpuState->instructionPointer = jumpAddress;
}

static void xorRegister(CPUState* cpuState, uint8_t* out, uint8_t val)
{
	*out ^= val;
	cpuState->Flags.zero = (*out == 0);
}

static void load(CPUState* cpuState, BUS* bus, uint8_t* register1, uint8_t* register2) 
{
	*register1 = bus->read(cpuState->instructionPointer);
	++cpuState->instructionPointer;
	*register2 = bus->read(cpuState->instructionPointer);
	++cpuState->instructionPointer;
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
	load(cpu, bus, &cpu->B, &cpu->C);
}

void ggb::writeAToAddressBC(CPUInstructionParameters) 
{
	bus->write(combine(cpu->B, cpu->C), cpu->A);
}

void ggb::incrementA(CPUInstructionParameters)
{
	increment(cpu, &cpu->A);
}

void ggb::jumpToValue(CPUInstructionParameters) 
{
	auto res = bus->readTwoBytes(cpu->instructionPointer);
	jump(cpu, res);
}

void ggb::xorASelf(CPUInstructionParameters) 
{
	xorRegister(cpu, &cpu->A, cpu->A);
}
