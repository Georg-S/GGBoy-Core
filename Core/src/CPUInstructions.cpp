#include "CPUInstructions.hpp"

#include "CPU.hpp"

#include <cassert>
#include <exception>

using namespace ggb;

static uint8_t read(CPUState* cpu, BUS* bus) 
{
	auto result = bus->read(cpu->InstructionPointer());
	++cpu->InstructionPointer();
	return result;
}

static uint16_t readTwoBytes(CPUState* cpu, BUS* bus) 
{
	auto result = bus->readTwoBytes(cpu->InstructionPointer());
	cpu->InstructionPointer() += 2;
	return result;
}

static void increment(CPUState* cpu, uint8_t& toIncrement) 
{
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag((toIncrement & 0XF) == 0xF); // TODO double check if this is correct
	toIncrement++;
	cpu->setZeroFlag(toIncrement == 0);
}

static void decrement(CPUState* cpu, uint8_t& toDecrement)
{
	cpu->setSubtractionFlag(true);
	cpu->setHalfCarryFlag((toDecrement & 0XF) == 0x00); // TODO double check if this is correct
	--toDecrement;
	cpu->setZeroFlag(toDecrement == 0);
}

static void rotateLeft(CPUState* cpu, uint8_t& out) 
{
	// TODO double check
	constexpr uint8_t lastBit = uint8_t(1) << uint8_t(7);
	uint8_t carry = (out & lastBit) == lastBit;
	cpu->setZeroFlag(false);
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(false);
	cpu->setCarryFlag(static_cast<bool>(carry));
	out = out << 1;
	out |= carry;
}

static void rotateRight(CPUState* cpu, uint8_t& out)
{
	// TODO double check
	constexpr uint8_t firstbit = uint8_t(1);
	uint8_t carry = (out & firstbit) == firstbit;
	cpu->setZeroFlag(false);
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(false);
	cpu->setCarryFlag(static_cast<bool>(carry));
	out = out >> 1;
	out |= (carry << 7);
}

static void add(CPUState* cpu, uint16_t& reg1, const uint16_t& reg2) 
{
	const uint32_t res = uint32_t(reg1) + reg2;
	const bool halfCarry = (((reg1 & 0xFFF) + (reg2 & 0xFFF)) & 0x1000) == 0x1000;
	const bool carry = (res & (0x1 << 16)) == (0x1 << 16);
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(halfCarry);
	cpu->setCarryFlag(carry);
	reg1 += reg2;
}

void ggb::invalidInstruction(CPUInstructionParameters)
{
	assert(!"Tried to execute an invalid instruction");
	throw std::exception("Tried to execute an invalid instruction");
}

void ggb::notImplementedInstruction(CPUInstructionParameters) 
{
	assert(!"Tried to execute a not yet implemented instruction");
	throw std::exception("Tried to execute a not yet implemented instruction");
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

void ggb::incrementBC(CPUInstructionParameters) 
{
	cpu->BC()++; // No flags set on purpose
}

void ggb::incrementB(CPUInstructionParameters)
{
	increment(cpu, cpu->B());
}

void ggb::decrementB(CPUInstructionParameters) 
{
	decrement(cpu, cpu->B());
}

void ggb::loadNumberIntoB(CPUInstructionParameters) 
{
	cpu->B() = bus->read(cpu->InstructionPointer());
	++cpu->InstructionPointer();
}

void ggb::rotateALeft(CPUInstructionParameters) 
{
	rotateLeft(cpu, cpu->A());
}

void ggb::loadStackPointerIntoAddress(CPUInstructionParameters) 
{
	auto address = readTwoBytes(cpu, bus);
	bus->write(address, cpu->StackPointer());
}

void ggb::addBCToHL(CPUInstructionParameters) 
{
	add(cpu, cpu->HL(), cpu->BC());
}

void ggb::loadValuePointedByBCIntoA(CPUInstructionParameters) 
{
	cpu->A() = bus->read(cpu->BC());
}

void ggb::decrementBC(CPUInstructionParameters) 
{
	--cpu->BC();
}

void ggb::incrementC(CPUInstructionParameters) 
{
	increment(cpu, cpu->C());
}

void ggb::decrementC(CPUInstructionParameters) 
{
	decrement(cpu, cpu->C());
}

void ggb::loadValueIntoC(CPUInstructionParameters) 
{
	cpu->C() = read(cpu, bus);
}

void ggb::rotateARight(CPUInstructionParameters) 
{
	rotateRight(cpu, cpu->A());
}

//void ggb::incrementA(CPUInstructionParameters)
//{
//}
//
//void ggb::jumpToValue(CPUInstructionParameters) 
//{
//}
//
//void ggb::xorASelf(CPUInstructionParameters) 
//{
//}
