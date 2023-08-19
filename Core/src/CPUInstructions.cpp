#include "CPUInstructions.hpp"

#include "CPU.hpp"

#include <cassert>
#include <exception>

using namespace ggb;

static uint16_t combineUpperAndLower(uint8_t upper, uint8_t lower)
{
	return (upper << 8) | lower;
}

static uint8_t read(CPUState* cpu, BUS* bus)
{
	auto result = bus->read(cpu->InstructionPointer());
	++cpu->InstructionPointer();
	return result;
}

static uint8_t readSigned(CPUState* cpu, BUS* bus)
{
	// TODO double check
	return static_cast<int8_t>(read(cpu, bus));
}

static uint16_t readTwoBytes(BUS* bus, uint16_t& address)
{
	uint8_t lower = bus->read(address++);
	uint8_t upper = bus->read(address++);

	return combineUpperAndLower(upper, lower);
}

static uint16_t readTwoBytes(CPUState* cpu, BUS* bus)
{
	return readTwoBytes(bus, cpu->InstructionPointer());
}

static void writeTwoBytes(BUS* bus, uint16_t address, uint16_t num)
{
	// TODO double check
	uint8_t lower = static_cast<uint8_t>(num);
	uint8_t upper = static_cast<uint8_t>(num >> 8);
	bus->write(address, lower);
	bus->write(address + 1, upper);
}

static void writeToStack(CPUState* cpu, BUS* bus, uint8_t num)
{
	assert(!"NOT implemented");
}

static void writeToStack(CPUState* cpu, BUS* bus, uint16_t num)
{
	uint8_t lower = static_cast<uint8_t>(num);
	uint8_t upper = static_cast<uint8_t>(num >> 8);

	bus->write(--cpu->StackPointer(), upper);
	bus->write(--cpu->StackPointer(), lower);
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

static void rotateLeftThroughCarry(CPUState* cpu, uint8_t& out)
{
	constexpr uint8_t lastBit = uint8_t(1) << uint8_t(7);

	// TODO double check
	const uint8_t oldCarry = cpu->getCarryFlag();
	const uint8_t carry = (out & lastBit) == lastBit;
	cpu->setZeroFlag(false);
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(false);
	cpu->setCarryFlag(static_cast<bool>(carry));
	out = out << 1;
	out |= oldCarry;
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

static void rotateRightThroughCarry(CPUState* cpu, uint8_t& out)
{
	constexpr uint8_t firstbit = uint8_t(1);

	// TODO double check
	const uint8_t oldCarry = cpu->getCarryFlag();
	const uint8_t carry = (out & firstbit) == firstbit;
	cpu->setZeroFlag(false);
	cpu->setSubtractionFlag(false);
	cpu->setHalfCarryFlag(false);
	cpu->setCarryFlag(static_cast<bool>(carry));
	out = out >> 1;
	out |= (oldCarry << 7);
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
	writeTwoBytes(bus, address, cpu->StackPointer());
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
	rotateLeftThroughCarry(cpu, cpu->A());
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
	rotateRightThroughCarry(cpu, cpu->A());
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

static void loadAIntoHLAddressAndInc(CPUInstructionParameters)
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

static void loadHLAddressIntoAIncrementHL(CPUInstructionParameters)
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

static void loadAIntoHLAddressAndDec(CPUInstructionParameters)
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

static void loadHLAddressIntoADecrementHL(CPUInstructionParameters)
{
	// TODO double check
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

static void returnNotZero(CPUInstructionParameters)
{
	if (!cpu->getZeroFlag())
		cpu->InstructionPointer() = readTwoBytes(bus, cpu->StackPointer());
}

static void popBC(CPUInstructionParameters)
{
	cpu->BC() = readTwoBytes(bus, cpu->StackPointer());
}

static void jumpNotZeroToNumber(CPUInstructionParameters)
{
	const uint16_t bytes = readTwoBytes(cpu, bus);
	if (!cpu->getZeroFlag())
		cpu->InstructionPointer() = bytes;
}

static void jumpToNumber(CPUInstructionParameters)
{
	cpu->InstructionPointer() = readTwoBytes(cpu, bus);
}

static void call(CPUInstructionParameters, bool call)
{
	// TODO check
	const auto number = readTwoBytes(cpu, bus);
	if (!call)
		return;

	writeToStack(cpu, bus, number);
	cpu->StackPointer() = number;
}

static void callNotZeroNumber(CPUInstructionParameters)
{
	call(cpu, bus, !cpu->getZeroFlag());
}

static void pushBC(CPUInstructionParameters)
{
	writeToStack(cpu, bus, cpu->BC());
}

static void addNumberToA(CPUInstructionParameters)
{
	auto num = read(cpu, bus);
	add(cpu, cpu->A(), num);
}

static void restart(CPUInstructionParameters, uint16_t address)
{
	writeToStack(cpu, bus, cpu->InstructionPointer());
	cpu->InstructionPointer() = address;
}

static void restart00(CPUInstructionParameters) 
{
	restart(cpu, bus, 0x00);
}

static void returnZero(CPUInstructionParameters) 
{
	if (cpu->getZeroFlag())
		cpu->InstructionPointer() = readTwoBytes(bus, cpu->StackPointer());
}

static void returnInstr(CPUInstructionParameters) 
{
	cpu->InstructionPointer() = readTwoBytes(bus, cpu->StackPointer());
}

static void jumpZeroToNumber(CPUInstructionParameters) 
{
	const uint16_t bytes = readTwoBytes(cpu, bus);
	if (cpu->getZeroFlag())
		cpu->InstructionPointer() = bytes;
}

static void prefixOPCode(CPUInstructionParameters) 
{
	notImplementedInstruction(cpu, bus);
}

static void callZeroNumber(CPUInstructionParameters) 
{
	call(cpu, bus, cpu->getZeroFlag());
}

static void callInstr(CPUInstructionParameters)
{
	call(cpu, bus, true);
}

static void addNumberAndCarryToA(CPUInstructionParameters) 
{
	add(cpu, cpu->A(), read(cpu, bus), cpu->getCarryFlag());
}

static void restart08(CPUInstructionParameters)
{
	restart(cpu, bus, 0x08);
}

static void returnNotCarry(CPUInstructionParameters) 
{
	if (cpu->getCarryFlag())
		cpu->InstructionPointer() = readTwoBytes(bus, cpu->StackPointer());
}

static void popDE(CPUInstructionParameters)
{
	cpu->DE() = readTwoBytes(bus, cpu->StackPointer());
}

static void jumpNotCarryToNumber(CPUInstructionParameters)
{
	const uint16_t bytes = readTwoBytes(cpu, bus);
	if (!cpu->getCarryFlag())
		cpu->InstructionPointer() = bytes;
}

static void callNotCarryNumber(CPUInstructionParameters)
{
	call(cpu, bus, !cpu->getCarryFlag());
}

static void pushDE(CPUInstructionParameters)
{
	writeToStack(cpu, bus, cpu->DE());
}

static void subNumberFromA(CPUInstructionParameters)
{
	auto num = read(cpu, bus);
	sub(cpu, cpu->A(), num);
}

static void restart10(CPUInstructionParameters)
{
	restart(cpu, bus, 0x10);
}

static void returnCarry(CPUInstructionParameters)
{
	if (cpu->getCarryFlag())
		cpu->InstructionPointer() = readTwoBytes(bus, cpu->StackPointer());
}

static void returnFromInterruptHandler(CPUInstructionParameters) 
{
	notImplementedInstruction(cpu, bus);
}

static void jumpCarryToNumber(CPUInstructionParameters)
{
	const uint16_t bytes = readTwoBytes(cpu, bus);
	if (cpu->getCarryFlag())
		cpu->InstructionPointer() = bytes;
}

static void callCarryNumber(CPUInstructionParameters)
{
	call(cpu, bus, cpu->getCarryFlag());
}

static void subtractNumberAndCarryFromA(CPUInstructionParameters)
{
	sub(cpu, cpu->A(), read(cpu, bus), cpu->getCarryFlag());
}

static void restart18(CPUInstructionParameters)
{
	restart(cpu, bus, 0x18);
}

static void loadAIntoSpecialAddressPlusNumber(CPUInstructionParameters) 
{
	auto num = read(cpu, bus);
	bus->write(0xFF00 + num, cpu->A());
}

static void POPHL(CPUInstructionParameters) 
{
	cpu->HL() = readTwoBytes(bus, cpu->StackPointer());
}

static void loadAIntoSpecialAddressPlusCarry(CPUInstructionParameters)
{
	// TODO double check
	bus->write(0xFF00 + cpu->getCarryFlag(), cpu->A());
}

static void pushHL(CPUInstructionParameters)
{
	writeToStack(cpu, bus, cpu->HL());
}

static void bitwiseAndAAndNumber(CPUInstructionParameters) 
{
	auto num = read(cpu, bus);
	cpu->A() = cpu->A() & num;
}

static void restart20(CPUInstructionParameters)
{
	restart(cpu, bus, 0x20);
}

static void addNumberToStackPointer(CPUInstructionParameters)
{
	// TODO implement
	notImplementedInstruction(cpu, bus);
}

static void jumpToHL(CPUInstructionParameters) 
{
	cpu->InstructionPointer() = cpu->HL();
}

static void loadAIntoNumberAddress(CPUInstructionParameters)
{
	auto address = readTwoBytes(cpu, bus);
	bus->write(address, cpu->A());
}

static void xorAAndNumber(CPUInstructionParameters)
{
	auto num = read(cpu, bus);
	cpu->A() = cpu->A() ^ num;
}

static void restart28(CPUInstructionParameters)
{
	restart(cpu, bus, 0x28);
}

static void loadSpecialAddressPlusNumberIntoA(CPUInstructionParameters) 
{
	auto num = read(cpu, bus);
	cpu->A() = bus->read(0xFF00 + num);
}

static void POPAF(CPUInstructionParameters)
{
	cpu->AF() = readTwoBytes(bus, cpu->StackPointer());
}

static void loadSpecialAddressPlusCarryIntoA(CPUInstructionParameters)
{
	cpu->A() = bus->read(0xFF00 + cpu->getCarryFlag());
}

static void disableInterrupts(CPUInstructionParameters) 
{
	notImplementedInstruction(cpu, bus);
}

static void pushAF(CPUInstructionParameters)
{
	writeToStack(cpu, bus, cpu->AF());
}

static void bitwiseOrAAndNumber(CPUInstructionParameters) 
{
	auto num = read(cpu, bus);
	cpu->A() |= num;
}

static void restart30(CPUInstructionParameters)
{
	restart(cpu, bus, 0x30);
}

static void loadStackPointerPlusNumberIntoHL(CPUInstructionParameters)
{
	notImplementedInstruction(cpu,bus);
}

static void loadHLIntoStackPointer(CPUInstructionParameters)
{
	cpu->StackPointer() = cpu->HL();
}

static void loadAddressIntoA(CPUInstructionParameters)
{
	auto address = readTwoBytes(cpu, bus);
	cpu->A() = bus->read(address);
}

static void enableInterrupts(CPUInstructionParameters)
{
	notImplementedInstruction(cpu, bus);
}

static void compareAWithNumber(CPUInstructionParameters)
{
	auto num = read(cpu, bus);
	compare(cpu, cpu->A(), num);
}

static void restart38(CPUInstructionParameters)
{
	restart(cpu, bus, 0x38);
}








ggb::OPCodes::OPCodes()
{
	initOpcodesArray();
}

void OPCodes::execute(uint16_t opCode, ggb::CPUState* cpu, ggb::BUS* bus)
{
	m_opcodes[opCode].func(cpu, bus);
}

std::string OPCodes::getMnemonic(uint16_t opCode) const
{
	return m_opcodes[opCode].mnemonic;
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

	setOpcode({ 0x00, noop, 4, "NOP" });
	setOpcode({ 0x01,loadBCValue, 12, "LD BC,u16" });
	setOpcode({ 0x02,writeAToAddressBC, 8, "LD (BC),A" });
	setOpcode({ 0x03,incrementBC, 8, "INC BC" });
	setOpcode({ 0x04,incrementB, 4, "INC B" });
	setOpcode({ 0x05,decrementB, 4, "DEC B" });
	setOpcode({ 0x06,loadNumberIntoB, 8, "LD B,u8" });
	setOpcode({ 0x07,rotateALeft, 4, "RLCA" });
	setOpcode({ 0x08,loadStackPointerIntoAddress, 20 ,"LD (u16),SP" });
	setOpcode({ 0x09,addBCToHL, 8 ,"ADD HL,BC" });
	setOpcode({ 0x0A,loadValuePointedByBCIntoA, 8 ,"LD A,(BC)" });
	setOpcode({ 0x0B,decrementBC, 8, "DEC BC" });
	setOpcode({ 0x0C,incrementC, 4, "INC C" });
	setOpcode({ 0x0D,decrementC, 4, "DEC C" });
	setOpcode({ 0x0E,loadValueIntoC, 8, "LD C,u8" });
	setOpcode({ 0x0F,rotateARight, 4, "RRCA" });
	setOpcode({ 0x10,stop, 4, "STOP" });
	setOpcode({ 0x11,loadTwoBytesIntoLD, 12, "LD DE,u16" });
	setOpcode({ 0x12,writeAToAddressDE, 8, "LD (DE),A" });
	setOpcode({ 0x13,incrementDE, 8, "INC DE" });
	setOpcode({ 0x14,incrementD, 4, "INC D" });
	setOpcode({ 0x15,decrementD, 4, "DEC D" });
	setOpcode({ 0x16,loadValueIntoD, 8, "LD D,u8" });
	setOpcode({ 0x17,rotateALeftThroughCarry, 4, "RLA" });
	setOpcode({ 0x18,jumpRealativeToValue, 12, "JR i8" });
	setOpcode({ 0x19,addDEToHL, 8, "ADD HL,DE" });
	setOpcode({ 0x1A,loadValuePointedByDEIntoA, 8, "LD A,(DE)" });
	setOpcode({ 0x1B,decrementDE, 8, "DEC DE" });
	setOpcode({ 0x1C,incrementE, 4, "INC E" });
	setOpcode({ 0x1D,decrementE, 4, "DEC E" });
	setOpcode({ 0x1E,loadValueIntoE, 8, "LD E,u8" });
	setOpcode({ 0x1F,rotateARightThroughCarry, 4, "RRA" });
	setOpcode({ 0x20,jumpRelativeNotZeroToValue, 8, "JR NZ,i8" }); // TODO can have different cycle counts, maybe this should be handled
	setOpcode({ 0x21,loadTwoBytesIntoHL, 12, "LD HL,u16" });
	setOpcode({ 0x22,loadAIntoHLAddressAndInc, 8, "LD (HL+),A" });
	setOpcode({ 0x23,incrementHL, 8, "INC HL" });
	setOpcode({ 0x24,incrementH, 4, "INC H" });
	setOpcode({ 0x25,decrementH, 4, "DEC H" });
	setOpcode({ 0x26,loadValueIntoH, 8, "LD H,u8" });
	setOpcode({ 0x27,decimalAdjustAccumulator, 4, "DAA" });
	setOpcode({ 0x28,jumpRealativeZeroToValue, 8, "JR Z,i8" }); // TODO can have different cycle counts, maybe this should be handled
	setOpcode({ 0x29,addHLToHL, 8, "ADD HL,HL" });
	setOpcode({ 0x2A,loadHLAddressIntoAIncrementHL, 8, "LD A,(HL+)" });
	setOpcode({ 0x2B,decrementHL, 8, "DEC HL" });
	setOpcode({ 0x2C,incrementL, 4, "INC L" });
	setOpcode({ 0x2D,decrementL, 4, "DEC L" });
	setOpcode({ 0x2E,loadValueIntoL, 8, "LD L,u8" });
	setOpcode({ 0x2F,complementAccumulator, 4, "CPL" });
	setOpcode({ 0x30,jumpRelativeNotCarryToValue, 8, "JR NC,i8" });
	setOpcode({ 0x31,loadTwoBytesIntoStackPointer, 12, "LD SP,u16" });
	setOpcode({ 0x32,loadAIntoHLAddressAndDec, 8, "LD (HL-),A" });
	setOpcode({ 0x33,incrementSP, 8, "INC SP" });
	setOpcode({ 0x34,incrementAddressHL, 12, "INC (HL)" });
	setOpcode({ 0x35,decrementAddressHL, 12, "DEC (HL)" });
	setOpcode({ 0x36,loadValueIntoAddressHL, 12, "LD (HL),u8" });
	setOpcode({ 0x37,setCarryFlag, 4, "SCF" });
	setOpcode({ 0x38,jumpRealativeCarryToValue, 8, "JR C,i8" }); // TODO can have different cycle counts, maybe this should be handled
	setOpcode({ 0x39,addSPToHL, 8, "ADD HL,SP" });
	setOpcode({ 0x3A,loadHLAddressIntoADecrementHL, 8, "LD A,(HL-)" });
	setOpcode({ 0x3B,decrementSP, 8, "DEC SP" });
	setOpcode({ 0x3C,incrementA, 4, "INC A" });
	setOpcode({ 0x3D,decrementA, 4, "DEC A" });
	setOpcode({ 0x3E,loadValueIntoA, 8, "LD A,u8" });
	setOpcode({ 0x3F,complementCarryFlag, 4, "CCF" });
	setOpcode({ 0x40,loadBIntoB, 4, "LD B,B" });
	setOpcode({ 0x41,loadCIntoB, 4, "LD B,C" });
	setOpcode({ 0x42,loadDIntoB, 4, "LD B,D" });
	setOpcode({ 0x43,loadEIntoB, 4, "LD B,E" });
	setOpcode({ 0x44,loadHIntoB, 4, "LD B,H" });
	setOpcode({ 0x45,loadLIntoB, 0, "LD B,L" });
	setOpcode({ 0x46,loadAddressHLIntoB, 8, "LD B,(HL)" });
	setOpcode({ 0x47,loadAIntoB, 4, "LD B,A" });
	setOpcode({ 0x48,loadBIntoC, 4, "LD C,B" });
	setOpcode({ 0x49,loadCIntoC, 4, "LD C,C" });
	setOpcode({ 0x4A,loadDIntoC, 4, "LD C,D" });
	setOpcode({ 0x4B,loadEIntoC, 4, "LD C,E" });
	setOpcode({ 0x4C,loadHIntoC, 4, "LD C,H" });
	setOpcode({ 0x4D,loadLIntoC, 4, "LD C,L" });
	setOpcode({ 0x4E,loadAddressHLIntoC, 8, "LD C,(HL)" });
	setOpcode({ 0x4F,loadAIntoC, 4, "LD C,A" });
	setOpcode({ 0x50,loadBIntoD, 4, "LD D,B" });
	setOpcode({ 0x51,loadCIntoD, 4, "LD D,C" });
	setOpcode({ 0x52,loadDIntoD, 4, "LD D,D" });
	setOpcode({ 0x53,loadEIntoD, 4, "LD D,E" });
	setOpcode({ 0x54,loadHIntoD, 4, "LD D,H" });
	setOpcode({ 0x55,loadLIntoD, 4, "LD D,L" });
	setOpcode({ 0x56,loadAddressHLIntoD, 8, "LD D,(HL)" });
	setOpcode({ 0x57,loadAIntoD, 4, "LD D,A" });
	setOpcode({ 0x58,loadBIntoE, 4, "LD E,B" });
	setOpcode({ 0x59,loadCIntoE, 4, "LD E,C" });
	setOpcode({ 0x5A,loadDIntoE, 4, "LD E,D" });
	setOpcode({ 0x5B,loadEIntoE, 4, "LD E,E" });
	setOpcode({ 0x5C,loadHIntoE, 4, "LD E,H" });
	setOpcode({ 0x5D,loadLIntoE, 4, "LD E,L" });
	setOpcode({ 0x5E,loadAddressHLIntoE, 8, "LD E,(HL)" });
	setOpcode({ 0x5F,loadAIntoE, 4, "LD E,A" });
	setOpcode({ 0x60,loadBIntoH, 4, "LD H,B" });
	setOpcode({ 0x61,loadCIntoH, 4, "LD H,C" });
	setOpcode({ 0x62,loadDIntoH, 4, "LD H,D" });
	setOpcode({ 0x63,loadEIntoH, 4, "LD H,E" });
	setOpcode({ 0x64,loadHIntoH, 4, "LD H,H" });
	setOpcode({ 0x65,loadLIntoH, 4, "LD H,L" });
	setOpcode({ 0x66,loadAddressHLIntoH, 8, "LD H,(HL)" });
	setOpcode({ 0x67,loadAIntoH, 4, "LD H,A" });
	setOpcode({ 0x68,loadBIntoL, 4, "LD L,B" });
	setOpcode({ 0x69,loadCIntoL, 4, "LD L,C" });
	setOpcode({ 0x6A,loadDIntoL, 4, "LD L,D" });
	setOpcode({ 0x6B,loadEIntoL, 4, "LD L,E" });
	setOpcode({ 0x6C,loadHIntoL, 4, "LD L,H" });
	setOpcode({ 0x6D,loadLIntoL, 4, "LD L,L" });
	setOpcode({ 0x6E,loadAddressHLIntoL, 8, "LD L,(HL)" });
	setOpcode({ 0x6F,loadAIntoL, 4, "LD L,A" });
	setOpcode({ 0x70,loadBIntoHLAddress, 8, "LD (HL),B" });
	setOpcode({ 0x71,loadCIntoHLAddress, 8, "LD (HL),C" });
	setOpcode({ 0x72,loadDIntoHLAddress, 8, "LD (HL),D" });
	setOpcode({ 0x73,loadEIntoHLAddress, 8, "LD (HL),E" });
	setOpcode({ 0x74,loadHIntoHLAddress, 8, "LD (HL),H" });
	setOpcode({ 0x75,loadLIntoHLAddress, 8, "LD (HL),L" });
	setOpcode({ 0x76,halt, 4, "HALT" });
	setOpcode({ 0x77,loadAIntoHLAddress, 8, "LD (HL),A" });
	setOpcode({ 0x78,loadBIntoA, 4, "LD A,B" });
	setOpcode({ 0x79,loadCIntoA, 4, "LD A,C" });
	setOpcode({ 0x7A,loadDIntoA, 4, "LD A,D" });
	setOpcode({ 0x7B,loadEIntoA, 4, "LD A,E" });
	setOpcode({ 0x7C,loadHIntoA, 4, "LD A,H" });
	setOpcode({ 0x7D,loadLIntoA, 4, "LD A,L" });
	setOpcode({ 0x7E,loadAddressHLIntoA, 8, "LD A,(HL)" });
	setOpcode({ 0x7F,loadAIntoA, 4, "LD A,A" });
	setOpcode({ 0x80,addBToA, 4, "ADD A,B" });
	setOpcode({ 0x81,addCToA, 4, "ADD A,C" });
	setOpcode({ 0x82,addDToA, 4, "ADD A,D" });
	setOpcode({ 0x83,addEToA, 4, "ADD A,E" });
	setOpcode({ 0x84,addHToA, 4, "ADD A,H" });
	setOpcode({ 0x85,addLToA, 4, "ADD A,L" });
	setOpcode({ 0x86,addHLAddressToA, 8, "ADD A,(HL)" });
	setOpcode({ 0x87,addAToA, 4, "ADD A,A" });
	setOpcode({ 0x88,addBAndCarryToA, 4, "ADC A,B" });
	setOpcode({ 0x89,addCAndCarryToA, 4, "ADC A,C" });
	setOpcode({ 0x8A,addDAndCarryToA, 4, "ADC A,D" });
	setOpcode({ 0x8B,addEAndCarryToA, 4, "ADC A,E" });
	setOpcode({ 0x8C,addHAndCarryToA, 4, "ADC A,H" });
	setOpcode({ 0x8D,addLAndCarryToA, 4, "ADC A,L" });
	setOpcode({ 0x8E,addHLAddressAndCarryToA, 8, "ADC A,(HL)" });
	setOpcode({ 0x8F,addAAndCarryToA, 4, "ADC A,A" });
	setOpcode({ 0x90,subBFromA, 4, "SUB A,B" });
	setOpcode({ 0x91,subCFromA, 4, "SUB A,C" });
	setOpcode({ 0x92,subDFromA, 4, "SUB A,D" });
	setOpcode({ 0x93,subEFromA, 4, "SUB A,E" });
	setOpcode({ 0x94,subHFromA, 4, "SUB A,H" });
	setOpcode({ 0x95,subLFromA, 4, "SUB A,L" });
	setOpcode({ 0x96,subHLAddressFromA, 8, "SUB A,(HL)" });
	setOpcode({ 0x97,subAFromA, 4, "SUB A,A" });
	setOpcode({ 0x98,subBAndCarryFromA, 4, "SBC A,B" });
	setOpcode({ 0x99,subCAndCarryFromA, 4, "SBC A,c" });
	setOpcode({ 0x9A,subDAndCarryFromA, 4, "SBC A,D" });
	setOpcode({ 0x9B,subEAndCarryFromA, 4, "SBC A,E" });
	setOpcode({ 0x9C,subHAndCarryFromA, 4, "SBC A,H" });
	setOpcode({ 0x9D,subLAndCarryFromA, 4, "SBC A,L" });
	setOpcode({ 0x9E,subHLAddressAndCarryFromA, 8, "SBC A,(HL)" });
	setOpcode({ 0x9F,subAAndCarryFromA, 4, "SBC A,A" });
	setOpcode({ 0xA0,bitwiseAndAAndB, 4, "AND A,B" });
	setOpcode({ 0xA1,bitwiseAndAAndC, 4, "AND A,C" });
	setOpcode({ 0xA2,bitwiseAndAAndD, 4, "AND A,D" });
	setOpcode({ 0xA3,bitwiseAndAAndE, 4, "AND A,E" });
	setOpcode({ 0xA4,bitwiseAndAAndH, 4, "AND A,H" });
	setOpcode({ 0xA5,bitwiseAndAAndL, 4, "AND A,L" });
	setOpcode({ 0xA6,bitwiseAndAAndHLAddress, 8, "AND A,(HL)" });
	setOpcode({ 0xA7,bitwiseAndAAndA, 4, "AND A,A" });
	setOpcode({ 0xA8,bitwiseXORAAndB, 4, "XOR A,B" });
	setOpcode({ 0xA9,bitwiseXORAAndC, 4, "XOR A,C" });
	setOpcode({ 0xAA,bitwiseXORAAndD, 4, "XOR A,D" });
	setOpcode({ 0xAB,bitwiseXORAAndE, 4, "XOR A,E" });
	setOpcode({ 0xAC,bitwiseXORAAndH, 4, "XOR A,H" });
	setOpcode({ 0xAD,bitwiseXORAAndL, 4, "XOR A,L" });
	setOpcode({ 0xAE,bitwiseXORAAndHLAddress, 8, "XOR A,(HL)" });
	setOpcode({ 0xAF,bitwiseXORAAndA, 4, "XOR A,A" });
	setOpcode({ 0xB0,bitwiseORAndB, 4, "OR A,B" });
	setOpcode({ 0xB1,bitwiseORAndC, 4, "OR A,C" });
	setOpcode({ 0xB2,bitwiseORAndD, 4, "OR A,D" });
	setOpcode({ 0xB3,bitwiseORAndE, 4, "OR A,E" });
	setOpcode({ 0xB4,bitwiseORAndH, 4, "OR A,H" });
	setOpcode({ 0xB5,bitwiseORAndL, 4, "OR A,L" });
	setOpcode({ 0xB6,bitwiseORAAndHLAddress, 8, "OR A,(HL)" });
	setOpcode({ 0xB7,bitwiseORAAndA, 4, "OR A,A" });
	setOpcode({ 0xB8,compareAAndB, 4, "CP A,B" });
	setOpcode({ 0xB9,compareAAndC, 4, "CP A,C" });
	setOpcode({ 0xBA,compareAAndD, 4, "CP A,D" });
	setOpcode({ 0xBB,compareAAndE, 4, "CP A,E" });
	setOpcode({ 0xBC,compareAAndH, 4, "CP A,H" });
	setOpcode({ 0xBD,compareAAndL, 4, "CP A,L" });
	setOpcode({ 0xBE,compareAAndHLAddress, 8, "CP A,(HL)" });
	setOpcode({ 0xBF,compareAAndA, 4, "CP A,A" });
	setOpcode({ 0xC0,returnNotZero, 8, "RET NZ" }); // TODO can have different cycle counts, maybe this should be handled
	setOpcode({ 0xC1,popBC, 12, "POP BC" });
	setOpcode({ 0xC2,jumpNotZeroToNumber, 12, "JP NZ,u16" }); // TODO can have different cycle counts, maybe this should be handled
	setOpcode({ 0xC3,jumpToNumber, 16, "JP u16" });
	setOpcode({ 0xC4,callNotZeroNumber, 12, "CALL NZ,u16" }); // TODO can have different cycle counts
	setOpcode({ 0xC5,pushBC, 16, "PUSH BC" });
	setOpcode({ 0xC6,addNumberToA, 8, "ADD A,u8" });
	setOpcode({ 0xC7,restart00, 16, "RST 00h" });
	setOpcode({ 0xC8,returnZero, 8, "RET Z" });// TODO can have different cycle counts
	setOpcode({ 0xC9,returnInstr, 16, "RET" });
	setOpcode({ 0xCA,jumpZeroToNumber, 12, "RET" });// TODO can have different cycle counts
	setOpcode({ 0xCB,prefixOPCode, 4, "PREFIX CB" });
	setOpcode({ 0xCC,callZeroNumber, 12, "CALL Z,u16" });// TODO can have different cycle counts
	setOpcode({ 0xCD,callInstr, 24, "CALL u16" });
	setOpcode({ 0xCE,addNumberAndCarryToA, 8, "ADC A,u8" });
	setOpcode({ 0xCF,restart08, 16, "RST 08h" });
	setOpcode({ 0xD0,returnNotCarry, 8, "RET NC" });// TODO can have different cycle counts
	setOpcode({ 0xD1,popDE, 12, "POP DE" });
	setOpcode({ 0xD2,jumpNotCarryToNumber, 12, "JP NC,u16" });// TODO can have different cycle counts
	setOpcode({ 0xD3,invalidInstruction, 0, "D3=INVALID"});
	setOpcode({ 0xD4,callNotCarryNumber, 12, "CALL NC,u16" });// TODO can have different cycle counts
	setOpcode({ 0xD5,pushDE, 16, "PUSH DE" });
	setOpcode({ 0xD6,subNumberFromA, 8, "SUB A,u8" });
	setOpcode({ 0xD7,restart10, 16, "RST 10h" });
	setOpcode({ 0xD8,returnCarry, 8, "RET C" }); // TODO can have different cycle counts
	setOpcode({ 0xD9,returnFromInterruptHandler, 16, "RETI" });
	setOpcode({ 0xDA,jumpCarryToNumber, 12, "JP C,u16" });// TODO can have different cycle counts
	setOpcode({ 0xDB,invalidInstruction, 0, "DB=INVALID" });
	setOpcode({ 0xDC,callCarryNumber, 12, "CALL C,u16" });// TODO can have different cycle counts
	setOpcode({ 0xDD,invalidInstruction, 0, "DD=INVALID" });
	setOpcode({ 0xDE,subtractNumberAndCarryFromA, 8, "SBC A,u8"});
	setOpcode({ 0xDF,restart18, 16, "RST 18h" });
	setOpcode({ 0xE0,loadAIntoSpecialAddressPlusNumber, 12, "LD (FF00+u8),A" });
	setOpcode({ 0xE1,POPHL, 12, "POP HL" });
	setOpcode({ 0xE2,loadAIntoSpecialAddressPlusCarry, 8, "LD (FF00+C),A" });
	setOpcode({ 0xE3,invalidInstruction, 0, "E3=INVALID" });
	setOpcode({ 0xE4,invalidInstruction, 0, "E4=INVALID" });
	setOpcode({ 0xE5,pushHL, 16, "PUSH HL" });
	setOpcode({ 0xE6,bitwiseAndAAndNumber, 8, "AND A,u8" });
	setOpcode({ 0xE7,restart20, 16, "RST 20h" });
	setOpcode({ 0xE8,addNumberToStackPointer, 16, "ADD SP,i8" }); 
	setOpcode({ 0xE9,jumpToHL, 4, "JP HL" });
	setOpcode({ 0xEA,loadAIntoNumberAddress, 16, "LD (u16),A" });
	setOpcode({ 0xEB,invalidInstruction, 0, "EB=INVALID" });
	setOpcode({ 0xEC,invalidInstruction, 0, "EC=INVALID" });// TODO can have different cycle counts
	setOpcode({ 0xED,invalidInstruction, 0, "ED=INVALID" });
	setOpcode({ 0xEE,xorAAndNumber, 8, "XOR A,u8" });
	setOpcode({ 0xEF,restart28, 16, "RST 28h" });
	setOpcode({ 0xF0,loadSpecialAddressPlusNumberIntoA, 12, "LD A,(FF00+u8)" });
	setOpcode({ 0xF1,POPAF, 12, "POP AF" });
	setOpcode({ 0xF2,loadSpecialAddressPlusCarryIntoA, 8, "LD A,(FF00+C)" });
	setOpcode({ 0xF3,disableInterrupts, 4, "DI" });
	setOpcode({ 0xF4,invalidInstruction, 0, "F4=INVALID" });
	setOpcode({ 0xF5,pushAF, 16, "PUSH AF" });
	setOpcode({ 0xF6,bitwiseOrAAndNumber, 8, "OR A,u8" });
	setOpcode({ 0xF7,restart30, 16, "RST 30h" });
	setOpcode({ 0xF8,loadStackPointerPlusNumberIntoHL, 12, "LD HL,SP+i8" });
	setOpcode({ 0xF9,loadHLIntoStackPointer, 8, "LD SP,HL" });
	setOpcode({ 0xFA,loadAddressIntoA, 16, "LD A,(u16)" });
	setOpcode({ 0xFB,enableInterrupts, 0, "EI" });
	setOpcode({ 0xFC,invalidInstruction, 0, "FC=INVALID" });// TODO can have different cycle counts
	setOpcode({ 0xFD,invalidInstruction, 0, "FD=INVALID" });
	setOpcode({ 0xFE,compareAWithNumber, 8, "CP A,u8" });
	setOpcode({ 0xFF,restart38, 16, "RST 38h" });

}
