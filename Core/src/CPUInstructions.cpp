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

uint8_t readSigned(CPUState* cpu, BUS* bus)
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

static void add(CPUState* cpu, uint8_t& outReg, uint8_t reg2) 
{
	// TODO double check
	const uint16_t res = uint16_t(outReg) + reg2;
	const bool halfCarry = (((outReg & 0xF) + (reg2 & 0xF)) & 0x10) == 0x10;
	const bool carry = (res & (0x1 << 8)) == (0x1 << 8);
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(halfCarry);
	cpu->setCarryFlag(carry);
	outReg += reg2;
	cpu->setZeroFlag(outReg == 0);
}

static void add(CPUState* cpu, uint8_t& outReg, uint8_t reg2, uint8_t carryFlag)
{
	// TODO double check
	const uint16_t res = uint16_t(outReg) + reg2 + carryFlag;
	const bool halfCarry = (((outReg & 0xF) + (reg2 & 0xF) + carryFlag) & 0x10) == 0x10;
	const bool carry = (res & (0x1 << 8)) == (0x1 << 8);
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(halfCarry);
	cpu->setCarryFlag(carry);
	outReg += reg2 + carryFlag;
	cpu->setZeroFlag(outReg == 0);
}

static void sub(CPUState* cpu, uint8_t& outReg, uint8_t reg2)
{
	// TODO double check
	const bool halfCarry = ((int(outReg) & 0xF) - (reg2 & 0xF)) < 0;
	const bool carry = (int(outReg) - reg2) < 0;
	cpu->setSubtractionFlag(true);
	cpu->setHalfCarryFlag(halfCarry);
	cpu->setCarryFlag(carry);
	outReg -= reg2;
	cpu->setZeroFlag(outReg == 0);
}

static void sub(CPUState* cpu, uint8_t& outReg, uint8_t reg2, uint8_t carryFlag)
{
	// TODO double check
	const bool halfCarry = ((int(outReg) & 0xF) - (reg2 & 0xF) - carryFlag) < 0;
	const bool carry = (int(outReg) - reg2 - carryFlag) < 0;
	cpu->setSubtractionFlag(true);
	cpu->setHalfCarryFlag(halfCarry);
	cpu->setCarryFlag(carry);
	outReg = outReg - reg2 - carryFlag;
	cpu->setZeroFlag(outReg == 0);
}

static void bitwiseAnd(CPUState* cpu, uint8_t& outReg, uint8_t reg2) 
{
	outReg &= reg2;
	cpu->setCarryFlag(false);
	cpu->setHalfCarryFlag(true);
	cpu->setSubtractionFlag(false);
	cpu->setZeroFlag(outReg == 0);
}

static void bitwiseXOR(CPUState* cpu, uint8_t& outReg, uint8_t reg2) 
{
	outReg ^= reg2;
	cpu->setCarryFlag(false);
	cpu->setHalfCarryFlag(false);
	cpu->setSubtractionFlag(false);
	cpu->setZeroFlag(outReg == 0);
}

static void bitwiseOR(CPUState* cpu, uint8_t& outReg, uint8_t reg2) 
{
	outReg |= reg2;
	cpu->setCarryFlag(false);
	cpu->setHalfCarryFlag(false);
	cpu->setSubtractionFlag(false);
	cpu->setZeroFlag(outReg == 0);
}

static void compare(CPUState* cpu, uint8_t outReg, uint8_t reg2) 
{
	sub(cpu, outReg, reg2);
}

static void invalidInstruction(CPUInstructionParameters)
{
	assert(!"Tried to execute an invalid instruction");
	throw std::exception("Tried to execute an invalid instruction");
}

static void notImplementedInstruction(CPUInstructionParameters)
{
	assert(!"Tried to execute a not yet implemented instruction");
	throw std::exception("Tried to execute a not yet implemented instruction");
}

static void noop(CPUInstructionParameters)
{
}

static void loadBCValue(CPUInstructionParameters)
{
	cpu->BC() = readTwoBytes(cpu, bus);
}

static void writeAToAddressBC(CPUInstructionParameters)
{
	bus->write(cpu->BC(), cpu->A());
}

static void incrementBC(CPUInstructionParameters)
{
	cpu->BC()++; // No flags set on purpose
}

static void incrementB(CPUInstructionParameters)
{
	increment(cpu, cpu->B());
}

static void decrementB(CPUInstructionParameters)
{
	decrement(cpu, cpu->B());
}

static void loadNumberIntoB(CPUInstructionParameters)
{
	cpu->B() = read(cpu, bus);
}

static void rotateALeft(CPUInstructionParameters)
{
	rotateLeft(cpu, cpu->A());
}

static void loadStackPointerIntoAddress(CPUInstructionParameters)
{
	auto address = readTwoBytes(cpu, bus);
	bus->write(address, cpu->StackPointer());
}

static void addBCToHL(CPUInstructionParameters)
{
	add(cpu, cpu->HL(), cpu->BC());
}

static void loadValuePointedByBCIntoA(CPUInstructionParameters)
{
	cpu->A() = bus->read(cpu->BC());
}

static void decrementBC(CPUInstructionParameters)
{
	--cpu->BC();
}

static void incrementC(CPUInstructionParameters)
{
	increment(cpu, cpu->C());
}

static void decrementC(CPUInstructionParameters)
{
	decrement(cpu, cpu->C());
}

static void loadValueIntoC(CPUInstructionParameters)
{
	cpu->C() = read(cpu, bus);
}

static void rotateARight(CPUInstructionParameters)
{
	rotateRight(cpu, cpu->A());
}

static void stop(CPUInstructionParameters)
{
	assert(!"Not implemented yet"); // Not really sure what to do here
}

static void loadTwoBytesIntoLD(CPUInstructionParameters)
{
	cpu->DE() = readTwoBytes(cpu, bus);
}

static void writeAToAddressDE(CPUInstructionParameters)
{
	bus->write(cpu->DE(), cpu->A());
}

static void incrementDE(CPUInstructionParameters)
{
	++cpu->DE();
}

static void incrementD(CPUInstructionParameters)
{
	increment(cpu, cpu->D());
}

static void decrementD(CPUInstructionParameters)
{
	decrement(cpu, cpu->D());
}

static void loadValueIntoD(CPUInstructionParameters)
{
	cpu->D() = read(cpu, bus);
}

static void rotateALeftThroughCarry(CPUInstructionParameters)
{
	// TODO: Is this really the same as rotateALeft()????
	rotateALeft(cpu, bus);
}

static void jumpRealativeToValue(CPUInstructionParameters)
{
	cpu->InstructionPointer() += readSigned(cpu, bus);
}

static void addDEToHL(CPUInstructionParameters)
{
	add(cpu, cpu->HL(), cpu->DE());
}

static void loadValuePointedByDEIntoA(CPUInstructionParameters)
{
	cpu->A() = bus->read(cpu->DE());
}

static void decrementDE(CPUInstructionParameters)
{
	--cpu->DE();
}

static void incrementE(CPUInstructionParameters)
{
	increment(cpu, cpu->E());
}

static void decrementE(CPUInstructionParameters)
{
	decrement(cpu, cpu->E());
}

static void loadValueIntoE(CPUInstructionParameters)
{
	cpu->E() = read(cpu, bus);
}

static void rotateARightThroughCarry(CPUInstructionParameters)
{
	// TODO: is this really the same as a normal rotate right???
	rotateRight(cpu, cpu->A());
}

static void jumpRelativeNotZeroToValue(CPUInstructionParameters)
{
	int8_t jumpDistance = cpu->getZeroFlag() ? 0 : readSigned(cpu, bus);
	cpu->InstructionPointer() += jumpDistance;
}

static void loadTwoBytesIntoHL(CPUInstructionParameters)
{
	cpu->HL() = readTwoBytes(cpu, bus);
}

static void loadAIntoHLAndInc(CPUInstructionParameters)
{
	bus->write(cpu->HL()++, cpu->A());
}

static void incrementHL(CPUInstructionParameters)
{
	++cpu->HL();
}

static void incrementH(CPUInstructionParameters)
{
	increment(cpu, cpu->H());
}

static void decrementH(CPUInstructionParameters)
{
	decrement(cpu, cpu->H());
}

static void loadValueIntoH(CPUInstructionParameters)
{
	cpu->H() = read(cpu, bus);
}

static void decimalAdjustAccumulator(CPUInstructionParameters)
{
	notImplementedInstruction(cpu, bus);
}

static void jumpRealativeZeroToValue(CPUInstructionParameters)
{
	int8_t jumpDistance = cpu->getZeroFlag() ? readSigned(cpu, bus) : 0;
	cpu->InstructionPointer() += jumpDistance;
}

static void addHLToHL(CPUInstructionParameters)
{
	add(cpu, cpu->HL(), cpu->HL());
}

static void loadValuePointedByHLIntoAIncrementHL(CPUInstructionParameters)
{
	cpu->A() = bus->read(cpu->HL()++);
}

static void decrementHL(CPUInstructionParameters)
{
	--cpu->HL();
}

static void incrementL(CPUInstructionParameters)
{
	increment(cpu, cpu->L());
}

static void decrementL(CPUInstructionParameters)
{
	decrement(cpu, cpu->L());
}

static void loadValueIntoL(CPUInstructionParameters)
{
	cpu->L() = read(cpu, bus);
}

static void complementAccumulator(CPUInstructionParameters)
{
	// TODO check if this is the correct behavior
	cpu->A() = ~cpu->A();
	cpu->setSubtractionFlag(true);
	cpu->setHalfCarryFlag(true);
}

static void jumpRelativeNotCarryToValue(CPUInstructionParameters)
{
	int8_t jumpDistance = cpu->getHalfCarryFlag() ? 0 : readSigned(cpu, bus);
	cpu->InstructionPointer() += jumpDistance;
}

static void loadTwoBytesIntoStackPointer(CPUInstructionParameters)
{
	cpu->StackPointer() = readTwoBytes(cpu, bus);
}

static void loadAIntoHLAndDec(CPUInstructionParameters)
{
	bus->write(cpu->HL()--, cpu->A());
}

static void incrementSP(CPUInstructionParameters)
{
	++cpu->StackPointer();
}

static void incrementAddressHL(CPUInstructionParameters)
{
	auto& val = bus->read(cpu->HL());
	increment(cpu, val);
}

static void decrementAddressHL(CPUInstructionParameters)
{
	auto& val = bus->read(cpu->HL());
	decrement(cpu, val);
}

static void loadValueIntoAddressHL(CPUInstructionParameters)
{
	bus->read(cpu->HL()) = read(cpu, bus);
}

static void setCarryFlag(CPUInstructionParameters)
{
	cpu->setCarryFlag(true);
	cpu->setHalfCarryFlag(false);
	cpu->setSubtractionFlag(false);
}

static void jumpRealativeCarryToValue(CPUInstructionParameters)
{
	int8_t jumpDistance = cpu->getHalfCarryFlag() ? readSigned(cpu, bus) : 0;
	cpu->InstructionPointer() += jumpDistance;
}

static void addSPToHL(CPUInstructionParameters)
{
	add(cpu, cpu->HL(), cpu->StackPointer());
}

static void loadValuePointedByHLIntoADecrementHL(CPUInstructionParameters)
{
	cpu->A() = bus->read(cpu->HL()--);
}

static void decrementSP(CPUInstructionParameters)
{
	--cpu->StackPointer();
}

static void incrementA(CPUInstructionParameters)
{
	increment(cpu, cpu->A());
}

static void decrementA(CPUInstructionParameters)
{
	decrement(cpu, cpu->A());
}

static void loadValueIntoA(CPUInstructionParameters)
{
	cpu->A() = read(cpu, bus);
}

static void complementCarryFlag(CPUInstructionParameters)
{
	cpu->setCarryFlag(!cpu->getCarryFlag());
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(false);
}

static void loadBIntoB(CPUInstructionParameters)
{
	cpu->B() = cpu->B();
}

static void loadCIntoB(CPUInstructionParameters)
{
	cpu->B() = cpu->C();
}

static void loadDIntoB(CPUInstructionParameters)
{
	cpu->B() = cpu->D();
}

static void loadEIntoB(CPUInstructionParameters)
{
	cpu->B() = cpu->E();
}

static void loadHIntoB(CPUInstructionParameters)
{
	cpu->B() = cpu->H();
}

static void loadLIntoB(CPUInstructionParameters)
{
	cpu->B() = cpu->L();
}

static void loadAddressHLIntoB(CPUInstructionParameters)
{
	cpu->B() = bus->read(cpu->HL());
}

static void loadAIntoB(CPUInstructionParameters)
{
	cpu->B() = cpu->A();
}

static void loadBIntoC(CPUInstructionParameters)
{
	cpu->C() = cpu->B();
}

static void loadCIntoC(CPUInstructionParameters)
{
	cpu->C() = cpu->C();
}

static void loadDIntoC(CPUInstructionParameters)
{
	cpu->C() = cpu->D();
}

static void loadEIntoC(CPUInstructionParameters)
{
	cpu->C() = cpu->E();
}

static void loadHIntoC(CPUInstructionParameters)
{
	cpu->C() = cpu->H();
}

static void loadLIntoC(CPUInstructionParameters)
{
	cpu->C() = cpu->L();
}

static void loadAddressHLIntoC(CPUInstructionParameters)
{
	cpu->C() = bus->read(cpu->HL());
}

static void loadAIntoC(CPUInstructionParameters)
{
	cpu->C() = cpu->A();
}

static void loadBIntoD(CPUInstructionParameters)
{
	cpu->D() = cpu->B();
}

static void loadCIntoD(CPUInstructionParameters)
{
	cpu->D() = cpu->C();
}

static void loadDIntoD(CPUInstructionParameters)
{
	cpu->D() = cpu->D();
}

static void loadEIntoD(CPUInstructionParameters)
{
	cpu->D() = cpu->E();
}

static void loadHIntoD(CPUInstructionParameters)
{
	cpu->D() = cpu->H();
}

static void loadLIntoD(CPUInstructionParameters)
{
	cpu->D() = cpu->L();
}

static void loadAddressHLIntoD(CPUInstructionParameters)
{
	cpu->D() = bus->read(cpu->HL());
}

static void loadAIntoD(CPUInstructionParameters)
{
	cpu->D() = cpu->A();
}

static void loadBIntoE(CPUInstructionParameters)
{
	cpu->E() = cpu->B();
}

static void loadCIntoE(CPUInstructionParameters)
{
	cpu->E() = cpu->C();
}

static void loadDIntoE(CPUInstructionParameters)
{
	cpu->E() = cpu->D();
}

static void loadEIntoE(CPUInstructionParameters)
{
	cpu->E() = cpu->E();
}

static void loadHIntoE(CPUInstructionParameters)
{
	cpu->E() = cpu->H();
}

static void loadLIntoE(CPUInstructionParameters)
{
	cpu->E() = cpu->L();
}

static void loadAddressHLIntoE(CPUInstructionParameters)
{
	cpu->E() = bus->read(cpu->HL());
}

static void loadAIntoE(CPUInstructionParameters)
{
	cpu->E() = cpu->A();
}

static void loadBIntoH(CPUInstructionParameters)
{
	cpu->H() = cpu->B();
}

static void loadCIntoH(CPUInstructionParameters)
{
	cpu->H() = cpu->C();
}

static void loadDIntoH(CPUInstructionParameters)
{
	cpu->H() = cpu->D();
}

static void loadEIntoH(CPUInstructionParameters)
{
	cpu->H() = cpu->E();
}

static void loadHIntoH(CPUInstructionParameters)
{
	cpu->H() = cpu->H();
}

static void loadLIntoH(CPUInstructionParameters)
{
	cpu->H() = cpu->L();
}

static void loadAddressHLIntoH(CPUInstructionParameters)
{
	cpu->H() = bus->read(cpu->HL());
}

static void loadAIntoH(CPUInstructionParameters)
{
	cpu->H() = cpu->A();
}

static void loadBIntoL(CPUInstructionParameters)
{
	cpu->L() = cpu->B();
}

static void loadCIntoL(CPUInstructionParameters)
{
	cpu->L() = cpu->C();
}

static void loadDIntoL(CPUInstructionParameters)
{
	cpu->L() = cpu->D();
}

static void loadEIntoL(CPUInstructionParameters)
{
	cpu->L() = cpu->E();
}

static void loadHIntoL(CPUInstructionParameters)
{
	cpu->L() = cpu->H();
}

static void loadLIntoL(CPUInstructionParameters)
{
	cpu->L() = cpu->L();
}

static void loadAddressHLIntoL(CPUInstructionParameters)
{
	cpu->L() = bus->read(cpu->HL());
}

static void loadAIntoL(CPUInstructionParameters)
{
	cpu->L() = cpu->A();
}

static void loadBIntoHLAddress(CPUInstructionParameters)
{
	bus->read(cpu->HL()) = cpu->B();
}

static void loadCIntoHLAddress(CPUInstructionParameters)
{
	bus->read(cpu->HL()) = cpu->C();
}

static void loadDIntoHLAddress(CPUInstructionParameters)
{
	bus->read(cpu->HL()) = cpu->D();
}

static void loadEIntoHLAddress(CPUInstructionParameters)
{
	bus->read(cpu->HL()) = cpu->E();
}

static void loadHIntoHLAddress(CPUInstructionParameters)
{
	bus->read(cpu->HL()) = cpu->H();
}

static void loadLIntoHLAddress(CPUInstructionParameters)
{
	bus->read(cpu->HL()) = cpu->L();
}

static void halt(CPUInstructionParameters)
{
	notImplementedInstruction(cpu, bus);
}

static void loadAIntoHLAddress(CPUInstructionParameters)
{
	bus->read(cpu->HL()) = cpu->A();
}

static void loadBIntoA(CPUInstructionParameters)
{
	cpu->A() = cpu->B();
}

static void loadCIntoA(CPUInstructionParameters)
{
	cpu->A() = cpu->C();
}

static void loadDIntoA(CPUInstructionParameters)
{
	cpu->A() = cpu->D();
}

static void loadEIntoA(CPUInstructionParameters)
{
	cpu->A() = cpu->E();
}

static void loadHIntoA(CPUInstructionParameters)
{
	cpu->A() = cpu->H();
}

static void loadLIntoA(CPUInstructionParameters)
{
	cpu->A() = cpu->L();
}

static void loadAddressHLIntoA(CPUInstructionParameters)
{
	cpu->A() = bus->read(cpu->HL());
}

static void loadAIntoA(CPUInstructionParameters)
{
	cpu->A() = cpu->A();
}

static void addBToA(CPUInstructionParameters) 
{
	add(cpu, cpu->A(), cpu->B());
}

static void addCToA(CPUInstructionParameters)
{
	add(cpu, cpu->A(), cpu->C());
}

static void addDToA(CPUInstructionParameters)
{
	add(cpu, cpu->A(), cpu->D());
}

static void addEToA(CPUInstructionParameters)
{
	add(cpu, cpu->A(), cpu->E());
}

static void addHToA(CPUInstructionParameters)
{
	add(cpu, cpu->A(), cpu->H());
}

static void addLToA(CPUInstructionParameters)
{
	add(cpu, cpu->A(), cpu->L());
}

static void addHLAddressToA(CPUInstructionParameters)
{
	add(cpu, cpu->A(), bus->read(cpu->HL()));
}

static void addAToA(CPUInstructionParameters)
{
	add(cpu, cpu->A(), cpu->A());
}

static void addBAndCarryToA(CPUInstructionParameters)
{
	add(cpu, cpu->A(), cpu->B(), cpu->getCarryFlag());
}

static void addCAndCarryToA(CPUInstructionParameters)
{
	add(cpu, cpu->A(), cpu->C(), cpu->getCarryFlag());
}

static void addDAndCarryToA(CPUInstructionParameters)
{
	add(cpu, cpu->A(), cpu->D(), cpu->getCarryFlag());
}

static void addEAndCarryToA(CPUInstructionParameters)
{
	add(cpu, cpu->A(), cpu->E(), cpu->getCarryFlag());
}

static void addHAndCarryToA(CPUInstructionParameters)
{
	add(cpu, cpu->A(), cpu->H(), cpu->getCarryFlag());
}

static void addLAndCarryToA(CPUInstructionParameters)
{
	add(cpu, cpu->A(), cpu->L(), cpu->getCarryFlag());
}

static void addHLAddressAndCarryToA(CPUInstructionParameters)
{
	add(cpu, cpu->A(), bus->read(cpu->HL()), cpu->getCarryFlag());
}

static void addAAndCarryToA(CPUInstructionParameters)
{
	add(cpu, cpu->A(), cpu->A(), cpu->getCarryFlag());
}

static void subBFromA(CPUInstructionParameters)
{
	sub(cpu, cpu->A(), cpu->B());
}

static void subCFromA(CPUInstructionParameters)
{
	sub(cpu, cpu->A(), cpu->C());
}

static void subDFromA(CPUInstructionParameters)
{
	sub(cpu, cpu->A(), cpu->D());
}

static void subEFromA(CPUInstructionParameters)
{
	sub(cpu, cpu->A(), cpu->E());
}

static void subHFromA(CPUInstructionParameters)
{
	sub(cpu, cpu->A(), cpu->H());
}

static void subLFromA(CPUInstructionParameters)
{
	sub(cpu, cpu->A(), cpu->L());
}

static void subHLAddressFromA(CPUInstructionParameters)
{
	sub(cpu, cpu->A(), bus->read(cpu->HL()));
}

static void subAFromA(CPUInstructionParameters)
{
	sub(cpu, cpu->A(), cpu->A());
}

static void subBAndCarryFromA(CPUInstructionParameters)
{
	sub(cpu, cpu->A(), cpu->B(), cpu->getCarryFlag());
}

static void subCAndCarryFromA(CPUInstructionParameters)
{
	sub(cpu, cpu->A(), cpu->C(), cpu->getCarryFlag());
}

static void subDAndCarryFromA(CPUInstructionParameters)
{
	sub(cpu, cpu->A(), cpu->D(), cpu->getCarryFlag());
}

static void subEAndCarryFromA(CPUInstructionParameters)
{
	sub(cpu, cpu->A(), cpu->E(), cpu->getCarryFlag());
}

static void subHAndCarryFromA(CPUInstructionParameters)
{
	sub(cpu, cpu->A(), cpu->H(), cpu->getCarryFlag());
}

static void subLAndCarryFromA(CPUInstructionParameters)
{
	sub(cpu, cpu->A(), cpu->L(), cpu->getCarryFlag());
}

static void subHLAddressAndCarryFromA(CPUInstructionParameters)
{
	sub(cpu, cpu->A(), bus->read(cpu->HL()), cpu->getCarryFlag());
}

static void subAAndCarryFromA(CPUInstructionParameters)
{
	sub(cpu, cpu->A(), cpu->A(), cpu->getCarryFlag());
}

static void bitwiseAndAAndB(CPUInstructionParameters)
{
	bitwiseAnd(cpu, cpu->A(), cpu->B());
}

static void bitwiseAndAAndC(CPUInstructionParameters)
{
	bitwiseAnd(cpu, cpu->A(), cpu->C());
}

static void bitwiseAndAAndD(CPUInstructionParameters)
{
	bitwiseAnd(cpu, cpu->A(), cpu->D());
}

static void bitwiseAndAAndE(CPUInstructionParameters)
{
	bitwiseAnd(cpu, cpu->A(), cpu->E());
}

static void bitwiseAndAAndH(CPUInstructionParameters)
{
	bitwiseAnd(cpu, cpu->A(), cpu->H());
}

static void bitwiseAndAAndL(CPUInstructionParameters)
{
	bitwiseAnd(cpu, cpu->A(), cpu->L());
}

static void bitwiseAndAAndHLAddress(CPUInstructionParameters)
{
	bitwiseAnd(cpu, cpu->A(), bus->read(cpu->HL()));
}

static void bitwiseAndAAndA(CPUInstructionParameters)
{
	bitwiseAnd(cpu, cpu->A(), cpu->A());
}

static void bitwiseXORAAndB(CPUInstructionParameters)
{
	bitwiseXOR(cpu, cpu->A(), cpu->B());
}

static void bitwiseXORAAndC(CPUInstructionParameters)
{
	bitwiseXOR(cpu, cpu->A(), cpu->C());
}

static void bitwiseXORAAndD(CPUInstructionParameters)
{
	bitwiseXOR(cpu, cpu->A(), cpu->D());
}

static void bitwiseXORAAndE(CPUInstructionParameters)
{
	bitwiseXOR(cpu, cpu->A(), cpu->E());
}

static void bitwiseXORAAndH(CPUInstructionParameters)
{
	bitwiseXOR(cpu, cpu->A(), cpu->H());
}

static void bitwiseXORAAndL(CPUInstructionParameters)
{
	bitwiseXOR(cpu, cpu->A(), cpu->L());
}

static void bitwiseXORAAndHLAddress(CPUInstructionParameters)
{
	bitwiseXOR(cpu, cpu->A(), bus->read(cpu->HL()));
}

static void bitwiseXORAAndA(CPUInstructionParameters)
{
	bitwiseXOR(cpu, cpu->A(), cpu->A());
}

static void bitwiseORAndB(CPUInstructionParameters)
{
	bitwiseOR(cpu, cpu->A(), cpu->B());
}

static void bitwiseORAndC(CPUInstructionParameters)
{
	bitwiseOR(cpu, cpu->A(), cpu->C());
}

static void bitwiseORAndD(CPUInstructionParameters)
{
	bitwiseOR(cpu, cpu->A(), cpu->D());
}

static void bitwiseORAndE(CPUInstructionParameters)
{
	bitwiseOR(cpu, cpu->A(), cpu->E());
}

static void bitwiseORAndH(CPUInstructionParameters)
{
	bitwiseOR(cpu, cpu->A(), cpu->H());
}

static void bitwiseORAndL(CPUInstructionParameters)
{
	bitwiseOR(cpu, cpu->A(), cpu->L());
}

static void bitwiseORAAndHLAddress(CPUInstructionParameters)
{
	bitwiseOR(cpu, cpu->A(), bus->read(cpu->HL()));
}

static void bitwiseORAAndA(CPUInstructionParameters)
{
	bitwiseOR(cpu, cpu->A(), cpu->A());
}

static void compareAAndB(CPUInstructionParameters)
{
	compare(cpu, cpu->A(), cpu->B());
}

static void compareAAndC(CPUInstructionParameters)
{
	compare(cpu, cpu->A(), cpu->C());
}

static void compareAAndD(CPUInstructionParameters)
{
	compare(cpu, cpu->A(), cpu->D());
}

static void compareAAndE(CPUInstructionParameters)
{
	compare(cpu, cpu->A(), cpu->E());
}

static void compareAAndH(CPUInstructionParameters)
{
	compare(cpu, cpu->A(), cpu->H());
}

static void compareAAndL(CPUInstructionParameters)
{
	compare(cpu, cpu->A(), cpu->L());
}

static void compareAAndHLAddress(CPUInstructionParameters)
{
	compare(cpu, cpu->A(), bus->read(cpu->HL()));
}

static void compareAAndA(CPUInstructionParameters)
{
	compare(cpu, cpu->A(), cpu->A());
}







ggb::OPCodes::OPCodes()
{
	initOpcodesArray();
}

void OPCodes::execute(uint16_t opCode, ggb::CPUState* cpu, ggb::BUS* bus)
{
	m_opcodes[opCode].func(cpu, bus);
}

void ggb::OPCodes::setOpcode(OPCode&& opcode)
{
	assert(opcode.id == m_counter);
	m_opcodes[m_counter] = std::move(opcode);
	++m_counter;
}

void ggb::OPCodes::initOpcodesArray()
{
	for (auto& opcode : m_opcodes)
		opcode = OPCode({ -1, notImplementedInstruction, -1 });

	setOpcode({ 0x00, noop, 4 });
	setOpcode({ 0x01,loadBCValue, 12 });
	setOpcode({ 0x02,writeAToAddressBC, 8 });
	setOpcode({ 0x03,incrementBC, 8 });
	setOpcode({ 0x04,incrementB, 4 });
	setOpcode({ 0x05,decrementB, 4 });
	setOpcode({ 0x06,loadNumberIntoB, 8 });
	setOpcode({ 0x07,rotateALeft, 4 });
	setOpcode({ 0x08,loadStackPointerIntoAddress, 20 });
	setOpcode({ 0x09,addBCToHL, 8 });
	setOpcode({ 0x0A,loadValuePointedByBCIntoA, 8 });
	setOpcode({ 0x0B,decrementBC, 8 });
	setOpcode({ 0x0C,incrementC, 4 });
	setOpcode({ 0x0D,decrementC, 4 });
	setOpcode({ 0x0E,loadValueIntoC, 8 });
	setOpcode({ 0x0F,rotateARight, 4 });
	setOpcode({ 0x10,stop, 4 });
	setOpcode({ 0x11,loadTwoBytesIntoLD, 12 });
	setOpcode({ 0x12,writeAToAddressDE, 8 });
	setOpcode({ 0x13,incrementDE, 8 });
	setOpcode({ 0x14,incrementD, 4 });
	setOpcode({ 0x15,decrementD, 4 });
	setOpcode({ 0x16,loadValueIntoD, 8 });
	setOpcode({ 0x17,rotateALeftThroughCarry, 4 });
	setOpcode({ 0x18,jumpRealativeToValue, 12 });
	setOpcode({ 0x19,addDEToHL, 8 });
	setOpcode({ 0x1A,loadValuePointedByDEIntoA, 8 });
	setOpcode({ 0x1B,decrementDE, 8 });
	setOpcode({ 0x1C,incrementE, 4 });
	setOpcode({ 0x1D,decrementE, 4 });
	setOpcode({ 0x1E,loadValueIntoE, 8 });
	setOpcode({ 0x1F,rotateARightThroughCarry, 4 });
	setOpcode({ 0x20,jumpRelativeNotZeroToValue, 8 }); // TODO can have different cycle counts, maybe this should be handled
	setOpcode({ 0x21,loadTwoBytesIntoHL, 12 });
	setOpcode({ 0x22,loadAIntoHLAndInc, 8 });
	setOpcode({ 0x23,incrementHL, 8 });
	setOpcode({ 0x24,incrementH, 4 });
	setOpcode({ 0x25,decrementH, 4 });
	setOpcode({ 0x26,loadValueIntoH, 8 });
	setOpcode({ 0x27,decimalAdjustAccumulator, 4 });
	setOpcode({ 0x28,jumpRealativeZeroToValue, 8 }); // TODO can have different cycle counts, maybe this should be handled
	setOpcode({ 0x29,addHLToHL, 8 });
	setOpcode({ 0x2A,loadValuePointedByHLIntoAIncrementHL, 8 });
	setOpcode({ 0x2B,decrementHL, 8 });
	setOpcode({ 0x2C,incrementL, 4 });
	setOpcode({ 0x2D,decrementL, 4 });
	setOpcode({ 0x2E,loadValueIntoL, 8 });
	setOpcode({ 0x2F,complementAccumulator, 4 });
	setOpcode({ 0x30,jumpRelativeNotCarryToValue, 8 });
	setOpcode({ 0x31,loadTwoBytesIntoStackPointer, 12 });
	setOpcode({ 0x32,loadAIntoHLAndDec, 8 });
	setOpcode({ 0x33,incrementSP, 8 });
	setOpcode({ 0x34,incrementAddressHL, 12 });
	setOpcode({ 0x35,decrementAddressHL, 12 });
	setOpcode({ 0x36,loadValueIntoAddressHL, 12 });
	setOpcode({ 0x37,setCarryFlag, 4 });
	setOpcode({ 0x38,jumpRealativeCarryToValue, 8 }); // TODO can have different cycle counts, maybe this should be handled
	setOpcode({ 0x39,addSPToHL, 8 });
	setOpcode({ 0x3A,loadValuePointedByHLIntoADecrementHL, 8 });
	setOpcode({ 0x3B,decrementSP, 8 });
	setOpcode({ 0x3C,incrementA, 4 });
	setOpcode({ 0x3D,decrementA, 4 });
	setOpcode({ 0x3E,loadValueIntoA, 8 });
	setOpcode({ 0x3F,complementCarryFlag, 4 });
	setOpcode({ 0x40,loadBIntoB, 4 });
	setOpcode({ 0x41,loadCIntoB, 4 });
	setOpcode({ 0x42,loadDIntoB, 4 });
	setOpcode({ 0x43,loadEIntoB, 4 });
	setOpcode({ 0x44,loadHIntoB, 4 });
	setOpcode({ 0x45,loadLIntoB, 0 });
	setOpcode({ 0x46,loadAddressHLIntoB, 8 });
	setOpcode({ 0x47,loadAIntoB, 4 });
	setOpcode({ 0x48,loadBIntoC, 4 });
	setOpcode({ 0x49,loadCIntoC, 4 });
	setOpcode({ 0x4A,loadDIntoC, 4 });
	setOpcode({ 0x4B,loadEIntoC, 4 });
	setOpcode({ 0x4C,loadHIntoC, 4 });
	setOpcode({ 0x4D,loadLIntoC, 4 });
	setOpcode({ 0x4E,loadAddressHLIntoC, 8 });
	setOpcode({ 0x4F,loadAIntoC, 4 });
	setOpcode({ 0x50,loadBIntoD, 4 });
	setOpcode({ 0x51,loadCIntoD, 4 });
	setOpcode({ 0x52,loadDIntoD, 4 });
	setOpcode({ 0x53,loadEIntoD, 4 });
	setOpcode({ 0x54,loadHIntoD, 4 });
	setOpcode({ 0x55,loadLIntoD, 4 });
	setOpcode({ 0x56,loadAddressHLIntoD, 8 });
	setOpcode({ 0x57,loadAIntoD, 4 });
	setOpcode({ 0x58,loadBIntoE, 4 });
	setOpcode({ 0x59,loadCIntoE, 4 });
	setOpcode({ 0x5A,loadDIntoE, 4 });
	setOpcode({ 0x5B,loadEIntoE, 4 });
	setOpcode({ 0x5C,loadHIntoE, 4 });
	setOpcode({ 0x5D,loadLIntoE, 4 });
	setOpcode({ 0x5E,loadAddressHLIntoE, 8 });
	setOpcode({ 0x5F,loadAIntoE, 4 });
	setOpcode({ 0x60,loadBIntoH, 4 });
	setOpcode({ 0x61,loadCIntoH, 4 });
	setOpcode({ 0x62,loadDIntoH, 4 });
	setOpcode({ 0x63,loadEIntoH, 4 });
	setOpcode({ 0x64,loadHIntoH, 4 });
	setOpcode({ 0x65,loadLIntoH, 4 });
	setOpcode({ 0x66,loadAddressHLIntoH, 8 });
	setOpcode({ 0x67,loadAIntoH, 4 });
	setOpcode({ 0x68,loadBIntoL, 4 });
	setOpcode({ 0x69,loadCIntoL, 4 });
	setOpcode({ 0x6A,loadDIntoL, 4 });
	setOpcode({ 0x6B,loadEIntoL, 4 });
	setOpcode({ 0x6C,loadHIntoL, 4 });
	setOpcode({ 0x6D,loadLIntoL, 4 });
	setOpcode({ 0x6E,loadAddressHLIntoL, 8 });
	setOpcode({ 0x6F,loadAIntoL, 4 });
	setOpcode({ 0x70,loadBIntoHLAddress, 8 });
	setOpcode({ 0x71,loadCIntoHLAddress, 8 });
	setOpcode({ 0x72,loadDIntoHLAddress, 8 });
	setOpcode({ 0x73,loadEIntoHLAddress, 8 });
	setOpcode({ 0x74,loadHIntoHLAddress, 8 });
	setOpcode({ 0x75,loadLIntoHLAddress, 8 });
	setOpcode({ 0x76,halt, 4 });
	setOpcode({ 0x77,loadAIntoHLAddress, 8 });
	setOpcode({ 0x78,loadBIntoA, 4 });
	setOpcode({ 0x79,loadCIntoA, 4 });
	setOpcode({ 0x7A,loadDIntoA, 4 });
	setOpcode({ 0x7B,loadEIntoA, 4 });
	setOpcode({ 0x7C,loadHIntoA, 4 });
	setOpcode({ 0x7D,loadLIntoA, 4 });
	setOpcode({ 0x7E,loadAddressHLIntoA, 8 });
	setOpcode({ 0x7F,loadAIntoA, 4 });
	setOpcode({ 0x80,addBToA, 4 });
	setOpcode({ 0x81,addCToA, 4 });
	setOpcode({ 0x82,addDToA, 4 });
	setOpcode({ 0x83,addEToA, 4 });
	setOpcode({ 0x84,addHToA, 4 });
	setOpcode({ 0x85,addLToA, 4 });
	setOpcode({ 0x86,addHLAddressToA, 8 });
	setOpcode({ 0x87,addAToA, 4 });
	setOpcode({ 0x88,addBAndCarryToA, 4 });
	setOpcode({ 0x89,addCAndCarryToA, 4 });
	setOpcode({ 0x8A,addDAndCarryToA, 4 });
	setOpcode({ 0x8B,addEAndCarryToA, 4 });
	setOpcode({ 0x8C,addHAndCarryToA, 4 });
	setOpcode({ 0x8D,addLAndCarryToA, 4 });
	setOpcode({ 0x8E,addHLAddressAndCarryToA, 8 });
	setOpcode({ 0x8F,addAAndCarryToA, 4 });
	setOpcode({ 0x90,subBFromA, 4 });
	setOpcode({ 0x91,subCFromA, 4 });
	setOpcode({ 0x92,subDFromA, 4 });
	setOpcode({ 0x93,subEFromA, 4 });
	setOpcode({ 0x94,subHFromA, 4 });
	setOpcode({ 0x95,subLFromA, 4 });
	setOpcode({ 0x96,subHLAddressFromA, 8 });
	setOpcode({ 0x97,subAFromA, 4 });
	setOpcode({ 0x98,subBAndCarryFromA, 4 });
	setOpcode({ 0x99,subCAndCarryFromA, 4 });
	setOpcode({ 0x9A,subDAndCarryFromA, 4 });
	setOpcode({ 0x9B,subEAndCarryFromA, 4 });
	setOpcode({ 0x9C,subHAndCarryFromA, 4 });
	setOpcode({ 0x9D,subLAndCarryFromA, 4 });
	setOpcode({ 0x9E,subHLAddressAndCarryFromA, 8 });
	setOpcode({ 0x9F,subAAndCarryFromA, 4 });
	setOpcode({ 0xA0,bitwiseAndAAndB, 4 });
	setOpcode({ 0xA1,bitwiseAndAAndC, 4 });
	setOpcode({ 0xA2,bitwiseAndAAndD, 4 });
	setOpcode({ 0xA3,bitwiseAndAAndE, 4 });
	setOpcode({ 0xA4,bitwiseAndAAndH, 4 });
	setOpcode({ 0xA5,bitwiseAndAAndL, 4 });
	setOpcode({ 0xA6,bitwiseAndAAndHLAddress, 8 });
	setOpcode({ 0xA7,bitwiseAndAAndA, 4 });
	setOpcode({ 0xA8,bitwiseXORAAndB, 4 });
	setOpcode({ 0xA9,bitwiseXORAAndC, 4 });
	setOpcode({ 0xAA,bitwiseXORAAndD, 4 });
	setOpcode({ 0xAB,bitwiseXORAAndE, 4 });
	setOpcode({ 0xAC,bitwiseXORAAndH, 4 });
	setOpcode({ 0xAD,bitwiseXORAAndL, 4 });
	setOpcode({ 0xAE,bitwiseXORAAndHLAddress, 8 });
	setOpcode({ 0xAF,bitwiseXORAAndA, 4 });
	setOpcode({ 0xB0,bitwiseORAndB, 4 });
	setOpcode({ 0xB1,bitwiseORAndC, 4 });
	setOpcode({ 0xB2,bitwiseORAndD, 4 });
	setOpcode({ 0xB3,bitwiseORAndE, 4 });
	setOpcode({ 0xB4,bitwiseORAndH, 4 });
	setOpcode({ 0xB5,bitwiseORAndL, 4 });
	setOpcode({ 0xB6,bitwiseORAAndHLAddress, 8 });
	setOpcode({ 0xB7,bitwiseORAAndA, 4 });
	setOpcode({ 0xB8,compareAAndB, 4 });
	setOpcode({ 0xB9,compareAAndC, 4 });
	setOpcode({ 0xBA,compareAAndD, 4 });
	setOpcode({ 0xBB,compareAAndE, 4 });
	setOpcode({ 0xBC,compareAAndH, 4 });
	setOpcode({ 0xBD,compareAAndL, 4 });
	setOpcode({ 0xBE,compareAAndHLAddress, 8 });
	setOpcode({ 0xBF,compareAAndA, 4 });
}
