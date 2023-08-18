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

__forceinline uint8_t readSigned(CPUState* cpu, BUS* bus)
{
	return static_cast<int8_t>(read(cpu, bus));
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

static void add(CPUState* cpu, uint16_t& outReg, uint16_t reg2) 
{
	const uint32_t res = uint32_t(outReg) + reg2;
	const bool halfCarry = (((outReg & 0xFFF) + (reg2 & 0xFFF)) & 0x1000) == 0x1000;
	const bool carry = (res & (0x1 << 16)) == (0x1 << 16);
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(halfCarry);
	cpu->setCarryFlag(carry);
	outReg += reg2;
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
	cpu->B() = read(cpu, bus);
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

void ggb::stop(CPUInstructionParameters)
{
	assert(!"Not implemented yet"); // Not really sure what to do here
}

void ggb::loadTwoBytesIntoLD(CPUInstructionParameters)
{
	cpu->DE() = readTwoBytes(cpu, bus);
}

void ggb::writeAToAddressDE(CPUInstructionParameters)
{
	bus->write(cpu->DE(), cpu->A());
}

void ggb::incrementDE(CPUInstructionParameters)
{
	++cpu->DE();
}

void ggb::incrementD(CPUInstructionParameters)
{
	increment(cpu, cpu->D());
}

void ggb::decrementD(CPUInstructionParameters)
{
	decrement(cpu, cpu->D());
}

void ggb::loadValueIntoD(CPUInstructionParameters)
{
	cpu->D() = read(cpu, bus);
}

void ggb::rotateALeftThroughCarry(CPUInstructionParameters)
{
	// TODO: Is this really the same as rotateALeft()????
	rotateALeft(cpu, bus);
}

void ggb::jumpRealativeToValue(CPUInstructionParameters)
{
	cpu->InstructionPointer() += readSigned(cpu, bus);
}

void ggb::addDEToHL(CPUInstructionParameters)
{
	add(cpu, cpu->HL(), cpu->DE());
}

void ggb::loadValuePointedByDEIntoA(CPUInstructionParameters)
{
	cpu->A() = bus->read(cpu->DE());
}

void ggb::decrementDE(CPUInstructionParameters)
{
	--cpu->DE();
}

void ggb::incrementE(CPUInstructionParameters)
{
	increment(cpu, cpu->E());
}

void ggb::decrementE(CPUInstructionParameters)
{
	decrement(cpu, cpu->E());
}

void ggb::loadValueIntoE(CPUInstructionParameters)
{
	cpu->E() = read(cpu, bus);
}

void ggb::rotateARightThroughCarry(CPUInstructionParameters)
{
	// TODO: is this really the same as a normal rotate right???
	rotateRight(cpu, cpu->A());
}

void ggb::jumpRelativeNotZeroToValue(CPUInstructionParameters)
{
	int8_t jumpDistance = cpu->getZeroFlag() ? 0 : readSigned(cpu, bus);
	cpu->InstructionPointer() += jumpDistance;
}

void ggb::loadTwoBytesIntoHL(CPUInstructionParameters)
{
	cpu->HL() = readTwoBytes(cpu, bus);
}

void ggb::loadAIntoHLAndInc(CPUInstructionParameters)
{
	bus->write(cpu->HL()++, cpu->A());
}

void ggb::incrementHL(CPUInstructionParameters)
{
	++cpu->HL();
}

void ggb::incrementH(CPUInstructionParameters)
{
	increment(cpu, cpu->H());
}

void ggb::decrementH(CPUInstructionParameters)
{
	decrement(cpu, cpu->H());
}

void ggb::loadValueIntoH(CPUInstructionParameters)
{
	cpu->H() = read(cpu, bus);
}

void ggb::decimalAdjustAccumulator(CPUInstructionParameters)
{
	notImplementedInstruction(cpu, bus);
}

void ggb::jumpRealativeZeroToValue(CPUInstructionParameters)
{
	int8_t jumpDistance = cpu->getZeroFlag() ? readSigned(cpu, bus) : 0;
	cpu->InstructionPointer() += jumpDistance;
}

void ggb::addHLToHL(CPUInstructionParameters)
{
	add(cpu, cpu->HL(), cpu->HL());
}

void ggb::loadValuePointedByHLIntoAIncrementHL(CPUInstructionParameters)
{
	cpu->A() = bus->read(cpu->HL()++);
}

void ggb::decrementHL(CPUInstructionParameters)
{
	--cpu->HL();
}

void ggb::incrementL(CPUInstructionParameters)
{
	increment(cpu, cpu->L());
}

void ggb::decrementL(CPUInstructionParameters)
{
	decrement(cpu, cpu->L());
}

void ggb::loadValueIntoL(CPUInstructionParameters)
{
	cpu->L() = read(cpu, bus);
}

void ggb::complementAccumulator(CPUInstructionParameters)
{
	// TODO check if this is the correct behavior
	cpu->A() = ~cpu->A();
	cpu->setSubtractionFlag(true);
	cpu->setHalfCarryFlag(true);
}

void ggb::jumpRelativeNotCarryToValue(CPUInstructionParameters)
{
	int8_t jumpDistance = cpu->getHalfCarryFlag() ? 0 : readSigned(cpu, bus);
	cpu->InstructionPointer() += jumpDistance;
}

void ggb::loadTwoBytesIntoStackPointer(CPUInstructionParameters)
{
	cpu->StackPointer() = readTwoBytes(cpu, bus);
}

void ggb::loadAIntoHLAndDec(CPUInstructionParameters)
{
	bus->write(cpu->HL()--, cpu->A());
}

void ggb::incrementSP(CPUInstructionParameters)
{
	++cpu->StackPointer();
}

void ggb::incrementAddressHL(CPUInstructionParameters)
{
	auto& val = bus->read(cpu->HL());
	increment(cpu, val);
}

void ggb::decrementAddressHL(CPUInstructionParameters)
{
	auto& val = bus->read(cpu->HL());
	decrement(cpu, val);
}

void ggb::loadValueIntoAddressHL(CPUInstructionParameters)
{
	bus->read(cpu->HL()) = read(cpu, bus);
}

void ggb::setCarryFlag(CPUInstructionParameters)
{
	cpu->setCarryFlag(true);
	cpu->setHalfCarryFlag(false);
	cpu->setSubtractionFlag(false);
}

void ggb::jumpRealativeCarryToValue(CPUInstructionParameters)
{
	int8_t jumpDistance = cpu->getHalfCarryFlag() ? readSigned(cpu, bus) : 0;
	cpu->InstructionPointer() += jumpDistance;
}

void ggb::addSPToHL(CPUInstructionParameters)
{
	add(cpu, cpu->HL(), cpu->StackPointer());
}

void ggb::loadValuePointedByHLIntoADecrementHL(CPUInstructionParameters)
{
	cpu->A() = bus->read(cpu->HL()--);
}

void ggb::decrementSP(CPUInstructionParameters)
{
	--cpu->StackPointer();
}

void ggb::incrementA(CPUInstructionParameters)
{
	increment(cpu, cpu->A());
}

void ggb::decrementA(CPUInstructionParameters)
{
	decrement(cpu, cpu->A());
}

void ggb::loadValueIntoA(CPUInstructionParameters)
{
	cpu->A() = read(cpu, bus);
}

void ggb::complementCarryFlag(CPUInstructionParameters)
{
	cpu->setCarryFlag(!cpu->getCarryFlag());
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(false);
}

void ggb::loadBIntoB(CPUInstructionParameters)
{
	cpu->B() = cpu->B();
}

void ggb::loadCIntoB(CPUInstructionParameters)
{
	cpu->B() = cpu->C();
}

void ggb::loadDIntoB(CPUInstructionParameters)
{
	cpu->B() = cpu->D();
}

void ggb::loadEIntoB(CPUInstructionParameters)
{
	cpu->B() = cpu->E();
}

void ggb::loadHIntoB(CPUInstructionParameters)
{
	cpu->B() = cpu->H();
}

void ggb::loadLIntoB(CPUInstructionParameters)
{
	cpu->B() = cpu->L();
}

void ggb::loadAddressHLIntoB(CPUInstructionParameters)
{
	cpu->B() = bus->read(cpu->HL());
}

void ggb::loadAIntoB(CPUInstructionParameters)
{
	cpu->B() = cpu->A();
}

void ggb::loadBIntoC(CPUInstructionParameters)
{
	cpu->C() = cpu->B();
}

void ggb::loadCIntoC(CPUInstructionParameters)
{
	cpu->C() = cpu->C();
}

void ggb::loadDIntoC(CPUInstructionParameters)
{
	cpu->C() = cpu->D();
}

void ggb::loadEIntoC(CPUInstructionParameters)
{
	cpu->C() = cpu->E();
}

void ggb::loadHIntoC(CPUInstructionParameters)
{
	cpu->C() = cpu->H();
}

void ggb::loadLIntoC(CPUInstructionParameters)
{
	cpu->C() = cpu->L();
}

void ggb::loadAddressHLIntoC(CPUInstructionParameters)
{
	cpu->C() = bus->read(cpu->HL());
}

void ggb::loadAIntoC(CPUInstructionParameters)
{
	cpu->C() = cpu->A();
}

void ggb::loadBIntoD(CPUInstructionParameters)
{
	cpu->D() = cpu->B();
}

void ggb::loadCIntoD(CPUInstructionParameters)
{
	cpu->D() = cpu->C();
}

void ggb::loadDIntoD(CPUInstructionParameters)
{
	cpu->D() = cpu->D();
}

void ggb::loadEIntoD(CPUInstructionParameters)
{
	cpu->D() = cpu->E();
}

void ggb::loadHIntoD(CPUInstructionParameters)
{
	cpu->D() = cpu->H();
}

void ggb::loadLIntoD(CPUInstructionParameters)
{
	cpu->D() = cpu->L();
}

void ggb::loadAddressHLIntoD(CPUInstructionParameters)
{
	cpu->D() = bus->read(cpu->HL());
}

void ggb::loadAIntoD(CPUInstructionParameters)
{
	cpu->D() = cpu->A();
}

void ggb::loadBIntoE(CPUInstructionParameters)
{
	cpu->E() = cpu->B();
}

void ggb::loadCIntoE(CPUInstructionParameters)
{
	cpu->E() = cpu->C();
}

void ggb::loadDIntoE(CPUInstructionParameters)
{
	cpu->E() = cpu->D();
}

void ggb::loadEIntoE(CPUInstructionParameters)
{
	cpu->E() = cpu->E();
}

void ggb::loadHIntoE(CPUInstructionParameters)
{
	cpu->E() = cpu->H();
}

void ggb::loadLIntoE(CPUInstructionParameters)
{
	cpu->E() = cpu->L();
}

void ggb::loadAddressHLIntoE(CPUInstructionParameters)
{
	cpu->E() = bus->read(cpu->HL());
}

void ggb::loadAIntoE(CPUInstructionParameters)
{
	cpu->E() = cpu->A();
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
