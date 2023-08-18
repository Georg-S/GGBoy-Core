#include "CPUState.hpp"

static void setBitToValue(uint8_t& out,int bit, uint8_t value) 
{
	// TODO double check if this is correct
	//out ^= (-value ^ out) & (uint8_t(1) << bit);
	out = out & ~(1 << bit) | (value << bit);
}

uint8_t& ggb::CPUState::A()
{
	return afUnion.regs[0];
}

uint8_t& ggb::CPUState::F()
{
	return afUnion.regs[1];
}

uint8_t& ggb::CPUState::B()
{
	return bcUnion.regs[0];
}

uint8_t& ggb::CPUState::C()
{
	return bcUnion.regs[1];
}

uint8_t& ggb::CPUState::D()
{
	return deUnion.regs[0];
}

uint8_t& ggb::CPUState::E()
{
	return deUnion.regs[1];
}

uint8_t& ggb::CPUState::H()
{
	return hlUnion.regs[0];
}

uint8_t& ggb::CPUState::L()
{
	return hlUnion.regs[1];
}

uint16_t& ggb::CPUState::AF()
{
	return afUnion.AF;
}

uint16_t& ggb::CPUState::BC()
{
	return bcUnion.BC;
}

uint16_t& ggb::CPUState::DE()
{
	return deUnion.DE;
}

uint16_t& ggb::CPUState::HL()
{
	return hlUnion.HL;
}

uint16_t& ggb::CPUState::StackPointer()
{
	return stackPointer;
}

uint16_t& ggb::CPUState::InstructionPointer()
{
	return instructionPointer;
}

void ggb::CPUState::setZeroFlag(bool value)
{
	setBitToValue(F(), 7, value);
}

void ggb::CPUState::setSubtractionFlag(bool value) 
{
	setBitToValue(F(), 6, value);
}

void ggb::CPUState::setHalfCarryFlag(bool value) 
{
	setBitToValue(F(), 5, value);
}

void ggb::CPUState::setCarryFlag(bool value) 
{
	setBitToValue(F(), 4, value);
}