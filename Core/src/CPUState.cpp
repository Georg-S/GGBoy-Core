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

void ggb::increment(CPUState* cpu, uint8_t& toIncrement)
{
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag((toIncrement & 0XF) == 0xF);
	toIncrement++;
	cpu->setZeroFlag(toIncrement == 0);
}

void ggb::decrement(CPUState* cpu, uint8_t& toDecrement)
{
	cpu->setSubtractionFlag(true);
	cpu->setHalfCarryFlag((toDecrement & 0XF) == 0x00);
	--toDecrement;
	cpu->setZeroFlag(toDecrement == 0);
}

void ggb::add(CPUState* cpu, uint8_t& outNum, uint8_t num2)
{
	// TODO probably correct but maybe check again
	const uint8_t initialVal = outNum;
	outNum += num2;
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag((outNum & 0xF) < (initialVal & 0xF));
	cpu->setCarryFlag(outNum < initialVal);
	cpu->setZeroFlag(outNum == 0);
}

void ggb::add(CPUState* cpu, uint8_t& outNum, uint8_t num2, uint8_t carryFlag)
{
	// TODO probably correct but maybe check again
	const uint8_t initialVal = outNum;
	outNum += num2 + carryFlag;
	const bool halfCarry = (((initialVal & 0xF) + (num2 & 0xF) + carryFlag) & 0x10) == 0x10;
	const bool carry = ((initialVal + num2) < initialVal) || (outNum < initialVal);
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(halfCarry);
	cpu->setCarryFlag(carry);
	cpu->setZeroFlag(outNum == 0);
}

// 16 bit addition is (as far as I know) composed of two 8 bit additions
// The low byte is added first, therefore the carry / half carry flag are a result of the second addition (upper byte)
void ggb::add(CPUState* cpu, uint16_t& outNum, uint16_t num2)
{
	const uint16_t initialVal = outNum;
	outNum += num2;
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag((outNum & 0xFFF) < (initialVal & 0xFFF));
	cpu->setCarryFlag(outNum < initialVal);
}

void ggb::sub(CPUState* cpu, uint8_t& outReg, uint8_t reg2)
{
	const uint8_t initialValue = outReg;
	outReg -= reg2;
	cpu->setSubtractionFlag(true);
	cpu->setHalfCarryFlag((outReg & 0xF) >(initialValue & 0xF));
	cpu->setCarryFlag(outReg > initialValue);
	cpu->setZeroFlag(outReg == 0);
}

void ggb::sub(CPUState* cpu, uint8_t& outNum, uint8_t num2, uint8_t carryFlag)
{
	// TODO probably correct but maybe check again
	const uint8_t initialVal = outNum;
	outNum = outNum - num2 - carryFlag;
	const bool halfCarry = (((initialVal - num2) & 0xF ) > (initialVal & 0xF)) || ((outNum & 0xF) > (initialVal & 0xF));
	const bool carry = ((initialVal - num2) > initialVal) || (outNum > initialVal);
	cpu->setSubtractionFlag(true);
	cpu->setHalfCarryFlag(halfCarry);
	cpu->setCarryFlag(carry);
	cpu->setZeroFlag(outNum == 0);
}

void ggb::compare(CPUState* cpu, uint8_t num, uint8_t num2)
{
	sub(cpu, num, num2);
}

void ggb::bitwiseAnd(CPUState* cpu, uint8_t& outNum, uint8_t num2)
{
	outNum &= num2;
	cpu->setCarryFlag(false);
	cpu->setHalfCarryFlag(true);
	cpu->setSubtractionFlag(false);
	cpu->setZeroFlag(outNum == 0);
}

void ggb::bitwiseOR(CPUState* cpu, uint8_t& outNum, uint8_t num2)
{
	outNum |= num2;
	cpu->setCarryFlag(false);
	cpu->setHalfCarryFlag(false);
	cpu->setSubtractionFlag(false);
	cpu->setZeroFlag(outNum == 0);
}

void ggb::bitwiseXOR(CPUState* cpu, uint8_t& outNum, uint8_t num2)
{
	outNum ^= num2;
	cpu->setCarryFlag(false);
	cpu->setHalfCarryFlag(false);
	cpu->setSubtractionFlag(false);
	cpu->setZeroFlag(outNum == 0);
}

void ggb::checkBit(CPUState* cpu, uint8_t num, int bit)
{
	const bool isSet = isBitSet(num, bit);
	cpu->setHalfCarryFlag(true);
	cpu->setSubtractionFlag(false);
	cpu->setZeroFlag(!isSet);
}

void ggb::swap(CPUState* cpu, uint8_t& out)
{
	const uint8_t upper = out >> 4;
	const uint8_t lower = out << 4;
	out = lower | upper;
	cpu->setZeroFlag(out == 0);
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(false);
	cpu->setCarryFlag(false);
}

void ggb::rotateLeft(CPUState* cpu, uint8_t& outNum)
{
	const bool carry = isBitSet(outNum, 7);
	cpu->setZeroFlag(false);
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(false);
	cpu->setCarryFlag(carry);
	outNum = outNum << 1;
	setBitToValue(outNum, 0, carry);
}

void ggb::rotateLeftThroughCarry(CPUState* cpu, uint8_t& out)
{
	const bool oldCarry = cpu->getCarryFlag();
	const bool carry = isBitSet(out, 7);
	cpu->setZeroFlag(false);
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(false);
	cpu->setCarryFlag(static_cast<bool>(carry));
	out = out << 1;
	setBitToValue(out, 0, oldCarry);
}

void ggb::rotateLeftSetZero(CPUState* cpu, uint8_t& out)
{
	rotateLeft(cpu, out);
	cpu->setZeroFlag(out == 0);
}

void ggb::rotateLeftThroughCarrySetZero(CPUState* cpu, uint8_t& out)
{
	rotateLeftThroughCarry(cpu, out);
	cpu->setZeroFlag(out == 0);
}

void ggb::shiftLeftArithmetically(CPUState* cpu, uint8_t& out)
{
	const bool carry = isBitSet(out, 7);
	out = out << 1;
	cpu->setZeroFlag(out == 0);
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(false);
	cpu->setCarryFlag(carry);
}

void ggb::rotateRight(CPUState* cpu, uint8_t& out)
{
	const bool carry = isBitSet(out, 0);
	cpu->setZeroFlag(false);
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(false);
	cpu->setCarryFlag(carry);
	out = out >> 1;
	setBitToValue(out, 7, carry);
}

void ggb::rotateRightThroughCarry(CPUState* cpu, uint8_t& out)
{
	const bool oldCarry = cpu->getCarryFlag();
	const bool carry = isBitSet(out, 0);
	cpu->setZeroFlag(false);
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(false);
	cpu->setCarryFlag(carry);
	out = out >> 1;
	setBitToValue(out, 7, oldCarry);
}

void ggb::rotateRightSetZero(CPUState* cpu, uint8_t& out)
{
	rotateRight(cpu, out);
	cpu->setZeroFlag(out == 0);
}

void ggb::rotateRightThroughCarrySetZero(CPUState* cpu, uint8_t& out)
{
	rotateRightThroughCarry(cpu, out);
	cpu->setZeroFlag(out == 0);
}

void ggb::shiftRightArithmetically(CPUState* cpu, uint8_t& out)
{
	const bool carry = isBitSet(out, 0);
	const bool upperBit = isBitSet(out, 7);

	out = out >> 1;
	cpu->setZeroFlag(out == 0);
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(false);
	cpu->setCarryFlag(carry);
	setBitToValue(out, 7, upperBit);
}

void ggb::shiftRightLogically(CPUState* cpu, uint8_t& out)
{
	const bool carry = isBitSet(out, 0);

	out = out >> 1;
	cpu->setZeroFlag(out == 0);
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(false);
	cpu->setCarryFlag(carry);
}
