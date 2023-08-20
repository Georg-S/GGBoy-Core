#include "CPUState.hpp"

#include "Utility.hpp"

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
	return m_stackPointer;
}

uint16_t& ggb::CPUState::InstructionPointer()
{
	return m_instructionPointer;
}

void ggb::CPUState::setZeroFlag(bool value)
{
	setBitToValue(F(), 7, value);
}

bool ggb::CPUState::getZeroFlag() const
{
	return isBitSet(F(), 7);
}

void ggb::CPUState::setSubtractionFlag(bool value) 
{
	setBitToValue(F(), 6, value);
}

bool ggb::CPUState::getSubtractionFlag() const
{
	return isBitSet(F(), 6);

}

void ggb::CPUState::setHalfCarryFlag(bool value) 
{
	setBitToValue(F(), 5, value);
}

bool ggb::CPUState::getHalfCarryFlag() const
{
	return isBitSet(F(), 5);
}

void ggb::CPUState::setCarryFlag(bool value) 
{
	setBitToValue(F(), 4, value);
}

bool ggb::CPUState::getCarryFlag() const
{
	return isBitSet(F(), 4);
}

void ggb::CPUState::disableInterrupts()
{
	m_interruptsEnabled = false;
}

void ggb::CPUState::enableInterrupts()
{
	m_interruptsEnabled = true;
}

bool ggb::CPUState::interruptsEnabled() const
{
	return m_interruptsEnabled;
}

uint8_t ggb::CPUState::F() const
{
	return afUnion.regs[1];
}
