#include "CPUInstructions.hpp"

#include <cassert>
#include <exception>
#include <iostream>

#include "CPU.hpp"
#include "Logging.hpp"
#include "Utility.hpp"

using namespace ggb;

static uint8_t read(CPUState* cpu, BUS* bus)
{
	auto result = bus->read(cpu->InstructionPointer());
	++cpu->InstructionPointer();
	return result;
}

static int8_t readSigned(CPUState* cpu, BUS* bus)
{
	return static_cast<int8_t>(read(cpu, bus));
}

// Reads two bytes from the bus (instruction pointer) and returns them combined (first byte = upper, second byte = lower)
static uint16_t readTwoBytes(CPUState* cpu, BUS* bus)
{
	uint8_t lower = bus->read(cpu->InstructionPointer()++);
	uint8_t upper = bus->read(cpu->InstructionPointer()++);

	return combineUpperAndLower(upper, lower);
}

static void pushOnStack(CPUState* cpu, BUS* bus, uint8_t num)
{
	assert(!"NOT implemented");
}

static uint16_t popFromStack(CPUState* cpu, BUS* bus) 
{
	uint8_t lower = bus->read(cpu->StackPointer()++);
	uint8_t upper = bus->read(cpu->StackPointer()++);

	return combineUpperAndLower(upper, lower);
}

static void pushOnStack(CPUState* cpu, BUS* bus, uint16_t num)
{
	uint8_t lower = static_cast<uint8_t>(num);
	uint8_t upper = static_cast<uint8_t>(num >> 8);

	bus->write(--cpu->StackPointer(), upper);
	bus->write(--cpu->StackPointer(), lower);
}



static void invalidInstruction(CPUInstructionParameters)
{
	assert(!"Tried to execute an invalid instruction");
	throw std::exception("Tried to execute an invalid instruction");
}

static void noop(CPUInstructionParameters)
{
	// noop does nothing -> therefore empty on purpose
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
	uint8_t lower = static_cast<uint8_t>(cpu->StackPointer());
	uint8_t upper = static_cast<uint8_t>(cpu->StackPointer() >> 8);
	bus->write(address, lower);
	bus->write(address + 1, upper);
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
	// TODO what is done at stop ???
	//cpu->stop();
	bus->resetTimerDivider();
}

static void loadTwoBytesIntoDE(CPUInstructionParameters)
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
	auto num = readSigned(cpu, bus);
	if (!cpu->getZeroFlag()) 
	{
		cpu->InstructionPointer() += num;
		*branchTaken = true;
	}
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
	if (cpu->getSubtractionFlag()) 
	{
		if (cpu->getCarryFlag())
			cpu->A() -= 0x60;
		if (cpu->getHalfCarryFlag())
			cpu->A() -= 0x6;
	}
	else 
	{
		if (cpu->getCarryFlag() || cpu->A() > 0x99)
		{
			cpu->setCarryFlag(true);
			cpu->A() += 0x60;
		}

		if (cpu->getHalfCarryFlag() || ((cpu->A() & 0x0F) > 0x09))
			cpu->A() += 0x6;
	}
	cpu->setZeroFlag(cpu->A() == 0);
	cpu->setHalfCarryFlag(false);
}

static void jumpRealativeZeroToValue(CPUInstructionParameters)
{
	auto num = readSigned(cpu, bus);
	if (cpu->getZeroFlag()) 
	{
		cpu->InstructionPointer() += num;
		*branchTaken = true;
	}
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
	cpu->A() = ~cpu->A();
	cpu->setSubtractionFlag(true);
	cpu->setHalfCarryFlag(true);
}

static void jumpRelativeNotCarryToValue(CPUInstructionParameters)
{
	auto num = readSigned(cpu, bus);
	if (!cpu->getCarryFlag()) 
	{
		cpu->InstructionPointer() += num;
		*branchTaken = true;
	}
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
	auto val = bus->read(cpu->HL());
	increment(cpu, val);
	bus->write(cpu->HL(), val);
}

static void decrementAddressHL(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	decrement(cpu, val);
	bus->write(cpu->HL(), val);
}

static void loadValueIntoAddressHL(CPUInstructionParameters)
{
	auto val = read(cpu, bus);
	bus->write(cpu->HL(), val);
}

static void setCarryFlag(CPUInstructionParameters)
{
	cpu->setCarryFlag(true);
	cpu->setHalfCarryFlag(false);
	cpu->setSubtractionFlag(false);
}

static void jumpRealativeCarryToValue(CPUInstructionParameters)
{
	auto num = readSigned(cpu, bus);
	if (cpu->getCarryFlag()) 
	{
		cpu->InstructionPointer() += num;
		*branchTaken = true;
	}
}

static void addSPToHL(CPUInstructionParameters)
{
	add(cpu, cpu->HL(), cpu->StackPointer());
}

static void loadHLAddressIntoADecrementHL(CPUInstructionParameters)
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

static void loadHLAddressIntoC(CPUInstructionParameters)
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

static void loadHLAddressIntoD(CPUInstructionParameters)
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
	bus->write(cpu->HL(), cpu->B());
}

static void loadCIntoHLAddress(CPUInstructionParameters)
{
	bus->write(cpu->HL(), cpu->C());
}

static void loadDIntoHLAddress(CPUInstructionParameters)
{
	bus->write(cpu->HL(), cpu->D());
}

static void loadEIntoHLAddress(CPUInstructionParameters)
{
	bus->write(cpu->HL(), cpu->E());
}

static void loadHIntoHLAddress(CPUInstructionParameters)
{
	bus->write(cpu->HL(), cpu->H());
}

static void loadLIntoHLAddress(CPUInstructionParameters)
{
	bus->write(cpu->HL(), cpu->L());
}

static void halt(CPUInstructionParameters)
{
	// TODO handle halt bug
	cpu->stop();
}

static void loadAIntoHLAddress(CPUInstructionParameters)
{
	bus->write(cpu->HL(), cpu->A());
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

static void bitwiseORAAndB(CPUInstructionParameters)
{
	bitwiseOR(cpu, cpu->A(), cpu->B());
}

static void bitwiseORAAndC(CPUInstructionParameters)
{
	bitwiseOR(cpu, cpu->A(), cpu->C());
}

static void bitwiseORAAndD(CPUInstructionParameters)
{
	bitwiseOR(cpu, cpu->A(), cpu->D());
}

static void bitwiseORAAndE(CPUInstructionParameters)
{
	bitwiseOR(cpu, cpu->A(), cpu->E());
}

static void bitwiseORAAndH(CPUInstructionParameters)
{
	bitwiseOR(cpu, cpu->A(), cpu->H());
}

static void bitwiseORAAndL(CPUInstructionParameters)
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
	{
		cpu->InstructionPointer() = popFromStack(cpu, bus); 
		*branchTaken = true;
	}
}

static void popBC(CPUInstructionParameters)
{
	cpu->BC() = popFromStack(cpu, bus);
}

static void jumpNotZeroToNumber(CPUInstructionParameters)
{
	const uint16_t bytes = readTwoBytes(cpu, bus);
	if (!cpu->getZeroFlag()) 
	{
		*branchTaken = true;
		cpu->InstructionPointer() = bytes;
	}
}

static void jumpToNumber(CPUInstructionParameters)
{
	cpu->InstructionPointer() = readTwoBytes(cpu, bus);
}

static void call(CPUState* cpu, BUS* bus, bool shouldCall)
{
	const auto callAddr = readTwoBytes(cpu, bus);
	if (!shouldCall)
		return;

	callAddress(cpu, bus, callAddr);
}

static void callNotZeroNumber(CPUInstructionParameters)
{
	*branchTaken = !cpu->getZeroFlag();
	call(cpu, bus, *branchTaken);
}

static void pushBC(CPUInstructionParameters)
{
	pushOnStack(cpu, bus, cpu->BC());
}

static void addNumberToA(CPUInstructionParameters)
{
	auto num = read(cpu, bus);
	add(cpu, cpu->A(), num);
}

static void restart(CPUState* cpu, BUS* bus, uint16_t address)
{
	pushOnStack(cpu, bus, cpu->InstructionPointer());
	cpu->InstructionPointer() = address;
}

static void restart00(CPUInstructionParameters)
{
	restart(cpu, bus, 0x00);
}

static void returnZero(CPUInstructionParameters)
{
	if (cpu->getZeroFlag()) 
	{
		*branchTaken = true;
		cpu->InstructionPointer() = popFromStack(cpu, bus);
	}
}

static void returnInstr(CPUInstructionParameters)
{
	cpu->InstructionPointer() = popFromStack(cpu, bus);
}

static void jumpZeroToNumber(CPUInstructionParameters)
{
	const uint16_t bytes = readTwoBytes(cpu, bus);
	if (cpu->getZeroFlag()) 
	{
		*branchTaken = true;
		cpu->InstructionPointer() = bytes;
	}
}

static void prefixOPCode(CPUInstructionParameters)
{
	assert(!"Should not be reachable");
	// Will not be executed because we call the corresponding opcode of the extended opcodes
}

static void callZeroNumber(CPUInstructionParameters)
{
	*branchTaken = cpu->getZeroFlag();
	call(cpu, bus, *branchTaken);
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
	if (!cpu->getCarryFlag()) 
	{
		*branchTaken = true;
		cpu->InstructionPointer() = popFromStack(cpu, bus);
	}
}

static void popDE(CPUInstructionParameters)
{
	cpu->DE() = popFromStack(cpu, bus);
}

static void jumpNotCarryToNumber(CPUInstructionParameters)
{
	const uint16_t bytes = readTwoBytes(cpu, bus);
	if (!cpu->getCarryFlag()) 
	{
		*branchTaken = true;
		cpu->InstructionPointer() = bytes;
	}
}

static void callNotCarryNumber(CPUInstructionParameters)
{
	*branchTaken = !cpu->getCarryFlag();
	call(cpu, bus, *branchTaken);
}

static void pushDE(CPUInstructionParameters)
{
	pushOnStack(cpu, bus, cpu->DE());
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
	{
		*branchTaken = true;
		cpu->InstructionPointer() = popFromStack(cpu, bus);
	}
}

static void enableInterrupts(CPUInstructionParameters)
{
	cpu->enableInterrupts();
}

static void returnFromInterruptHandler(CPUInstructionParameters)
{
	cpu->enableInterrupts();
	returnInstr(cpu, bus, branchTaken);
}

static void jumpCarryToNumber(CPUInstructionParameters)
{
	const uint16_t bytes = readTwoBytes(cpu, bus);
	if (cpu->getCarryFlag()) 
	{
		*branchTaken = true;
		cpu->InstructionPointer() = bytes;
	}
}

static void callCarryNumber(CPUInstructionParameters)
{
	*branchTaken = cpu->getCarryFlag();
	call(cpu, bus, *branchTaken);
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
	cpu->HL() = popFromStack(cpu, bus);
}

static void loadAIntoSpecialAddressPlusC(CPUInstructionParameters)
{
	bus->write(0xFF00 + cpu->C(), cpu->A());
}

static void pushHL(CPUInstructionParameters)
{
	pushOnStack(cpu, bus, cpu->HL());
}

static void bitwiseAndAAndNumber(CPUInstructionParameters)
{
	auto num = read(cpu, bus);
	bitwiseAnd(cpu, cpu->A(), num);
}

static void restart20(CPUInstructionParameters)
{
	restart(cpu, bus, 0x20);
}

static void addNumberToStackPointer(CPUInstructionParameters)
{
	add(cpu, cpu->StackPointer(), readSigned(cpu, bus));
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
	bitwiseXOR(cpu, cpu->A(), num);
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
	cpu->AF() = popFromStack(cpu, bus);
	cpu->F() &= 0xF0;
}

static void loadSpecialAddressPlusCIntoA(CPUInstructionParameters)
{
	cpu->A() = bus->read(0xFF00 + cpu->C());
}

static void disableInterrupts(CPUInstructionParameters)
{
	cpu->disableInterrupts();
}

static void pushAF(CPUInstructionParameters)
{
	pushOnStack(cpu, bus, cpu->AF());
}

static void bitwiseOrAAndNumber(CPUInstructionParameters)
{
	auto num = read(cpu, bus);
	bitwiseOR(cpu, cpu->A(), num);
}

static void restart30(CPUInstructionParameters)
{
	restart(cpu, bus, 0x30);
}

static void loadStackPointerPlusNumberIntoHL(CPUInstructionParameters)
{
	auto sp = cpu->StackPointer();
	const auto signedVal = readSigned(cpu, bus);
	add(cpu, sp, signedVal);
	cpu->HL() = sp;
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

static void compareAWithNumber(CPUInstructionParameters)
{
	auto num = read(cpu, bus);
	compare(cpu, cpu->A(), num);
}

static void restart38(CPUInstructionParameters)
{
	restart(cpu, bus, 0x38);
}

static void rotateBLeft(CPUInstructionParameters)
{
	rotateLeftSetZero(cpu, cpu->B());
}

static void rotateCLeft(CPUInstructionParameters)
{
	rotateLeftSetZero(cpu, cpu->C());
}

static void rotateDLeft(CPUInstructionParameters)
{
	rotateLeftSetZero(cpu, cpu->D());
}

static void rotateELeft(CPUInstructionParameters)
{
	rotateLeftSetZero(cpu, cpu->E());
}

static void rotateHLeft(CPUInstructionParameters)
{
	rotateLeftSetZero(cpu, cpu->H());
}

static void rotateLLeft(CPUInstructionParameters)
{
	rotateLeftSetZero(cpu, cpu->L());
}

static void rotateHLAddressLeft(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	rotateLeftSetZero(cpu, val);
	bus->write(cpu->HL(), val);
}

static void rotateALeftSetZero(CPUInstructionParameters)
{
	rotateLeftSetZero(cpu, cpu->A());
}

static void rotateBRight(CPUInstructionParameters)
{
	rotateRightSetZero(cpu, cpu->B());
}

static void rotateCRight(CPUInstructionParameters)
{
	rotateRightSetZero(cpu, cpu->C());
}

static void rotateDRight(CPUInstructionParameters)
{
	rotateRightSetZero(cpu, cpu->D());
}

static void rotateERight(CPUInstructionParameters)
{
	rotateRightSetZero(cpu, cpu->E());
}

static void rotateHRight(CPUInstructionParameters)
{
	rotateRightSetZero(cpu, cpu->H());
}

static void rotateLRight(CPUInstructionParameters)
{
	rotateRightSetZero(cpu, cpu->L());
}

static void rotateHLAddressRight(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	rotateRightSetZero(cpu, val);
	bus->write(cpu->HL(), val);
}

static void rotateARightSetZero(CPUInstructionParameters)
{
	rotateRightSetZero(cpu, cpu->A());
}

static void rotateBRightThroughCarry(CPUInstructionParameters)
{
	rotateRightThroughCarrySetZero(cpu, cpu->B());
}

static void rotateCRightThroughCarry(CPUInstructionParameters)
{
	rotateRightThroughCarrySetZero(cpu, cpu->C());
}

static void rotateDRightThroughCarry(CPUInstructionParameters)
{
	rotateRightThroughCarrySetZero(cpu, cpu->D());
}

static void rotateERightThroughCarry(CPUInstructionParameters)
{
	rotateRightThroughCarrySetZero(cpu, cpu->E());
}

static void rotateHRightThroughCarry(CPUInstructionParameters)
{
	rotateRightThroughCarrySetZero(cpu, cpu->H());
}

static void rotateLRightThroughCarry(CPUInstructionParameters)
{
	rotateRightThroughCarrySetZero(cpu, cpu->L());
}

static void rotateHLAddressRightThroughCarry(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	rotateRightThroughCarrySetZero(cpu, val);
	bus->write(cpu->HL(), val);
}

static void rotateARightThroughCarrySetZero(CPUInstructionParameters)
{
	rotateRightThroughCarrySetZero(cpu, cpu->A());
}

static void rotateBLeftThroughCarry(CPUInstructionParameters)
{
	rotateLeftThroughCarrySetZero(cpu, cpu->B());
}

static void rotateCLeftThroughCarry(CPUInstructionParameters)
{
	rotateLeftThroughCarrySetZero(cpu, cpu->C());
}

static void rotateDLeftThroughCarry(CPUInstructionParameters)
{
	rotateLeftThroughCarrySetZero(cpu, cpu->D());
}

static void rotateELeftThroughCarry(CPUInstructionParameters)
{
	rotateLeftThroughCarrySetZero(cpu, cpu->E());
}

static void rotateHLeftThroughCarry(CPUInstructionParameters)
{
	rotateLeftThroughCarrySetZero(cpu, cpu->H());
}

static void rotateLLeftThroughCarry(CPUInstructionParameters)
{
	rotateLeftThroughCarrySetZero(cpu, cpu->L());
}

static void rotateHLAddressLeftThroughCarry(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	rotateLeftThroughCarrySetZero(cpu, val);
	bus->write(cpu->HL(), val);
}

static void rotateALeftThroughCarrySetZero(CPUInstructionParameters)
{
	rotateLeftThroughCarrySetZero(cpu, cpu->A());
}

static void shiftBLeftArithmetically(CPUInstructionParameters)
{
	shiftLeftArithmetically(cpu, cpu->B());
}

static void shiftCLeftArithmetically(CPUInstructionParameters)
{
	shiftLeftArithmetically(cpu, cpu->C());
}

static void shiftDLeftArithmetically(CPUInstructionParameters)
{
	shiftLeftArithmetically(cpu, cpu->D());
}

static void shiftELeftArithmetically(CPUInstructionParameters)
{
	shiftLeftArithmetically(cpu, cpu->E());
}

static void shiftHLeftArithmetically(CPUInstructionParameters)
{
	shiftLeftArithmetically(cpu, cpu->H());
}

static void shiftLLeftArithmetically(CPUInstructionParameters)
{
	shiftLeftArithmetically(cpu, cpu->L());
}

static void shiftHLAddressLeftArithmetically(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	shiftLeftArithmetically(cpu, val);
	bus->write(cpu->HL(), val);
}

static void shiftALeftArithmetically(CPUInstructionParameters)
{
	shiftLeftArithmetically(cpu, cpu->A());
}

static void shiftBRightArithmetically(CPUInstructionParameters)
{
	shiftRightArithmetically(cpu, cpu->B());
}

static void shiftCRightArithmetically(CPUInstructionParameters)
{
	shiftRightArithmetically(cpu, cpu->C());
}

static void shiftDRightArithmetically(CPUInstructionParameters)
{
	shiftRightArithmetically(cpu, cpu->D());
}

static void shiftERightArithmetically(CPUInstructionParameters)
{
	shiftRightArithmetically(cpu, cpu->E());
}

static void shiftHRightArithmetically(CPUInstructionParameters)
{
	shiftRightArithmetically(cpu, cpu->H());
}

static void shiftLRightArithmetically(CPUInstructionParameters)
{
	shiftRightArithmetically(cpu, cpu->L());
}

static void shiftHLAddressRightArithmetically(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	shiftRightArithmetically(cpu, val);
	bus->write(cpu->HL(), val);
}

static void shiftARightArithmetically(CPUInstructionParameters)
{
	shiftRightArithmetically(cpu, cpu->A());
}




static void swapB(CPUInstructionParameters)
{
	swap(cpu, cpu->B());
}

static void swapC(CPUInstructionParameters)
{
	swap(cpu, cpu->C());
}

static void swapD(CPUInstructionParameters)
{
	swap(cpu, cpu->D());
}

static void swapE(CPUInstructionParameters)
{
	swap(cpu, cpu->E());
}

static void swapH(CPUInstructionParameters)
{
	swap(cpu, cpu->H());
}

static void swapL(CPUInstructionParameters)
{
	swap(cpu, cpu->L());
}

static void swapHLAddress(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	swap(cpu, val);
	bus->write(cpu->HL(), val);
}

static void swapA(CPUInstructionParameters)
{
	swap(cpu, cpu->A());
}

static void shiftBRightLogically(CPUInstructionParameters)
{
	shiftRightLogically(cpu, cpu->B());
}

static void shiftCRightLogically(CPUInstructionParameters)
{
	shiftRightLogically(cpu, cpu->C());
}

static void shiftDRightLogically(CPUInstructionParameters)
{
	shiftRightLogically(cpu, cpu->D());
}

static void shiftERightLogically(CPUInstructionParameters)
{
	shiftRightLogically(cpu, cpu->E());
}

static void shiftHRightLogically(CPUInstructionParameters)
{
	shiftRightLogically(cpu, cpu->H());
}

static void shiftLRightLogically(CPUInstructionParameters)
{
	shiftRightLogically(cpu, cpu->L());
}

static void shiftHLAddressRightLogically(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	shiftRightLogically(cpu, val);
	bus->write(cpu->HL(), val);
}

static void shiftARightLogically(CPUInstructionParameters)
{
	shiftRightLogically(cpu, cpu->A());
}

static void checkBit0A(CPUInstructionParameters)
{
	checkBit(cpu, cpu->A(), 0);
}

static void checkBit1A(CPUInstructionParameters)
{
	checkBit(cpu, cpu->A(), 1);
}

static void checkBit2A(CPUInstructionParameters)
{
	checkBit(cpu, cpu->A(), 2);
}

static void checkBit3A(CPUInstructionParameters)
{
	checkBit(cpu, cpu->A(), 3);
}

static void checkBit4A(CPUInstructionParameters)
{
	checkBit(cpu, cpu->A(), 4);
}

static void checkBit5A(CPUInstructionParameters)
{
	checkBit(cpu, cpu->A(), 5);
}

static void checkBit6A(CPUInstructionParameters)
{
	checkBit(cpu, cpu->A(), 6);
}

static void checkBit7A(CPUInstructionParameters)
{
	checkBit(cpu, cpu->A(), 7);
}

static void checkBit0B(CPUInstructionParameters)
{
	checkBit(cpu, cpu->B(), 0);
}

static void checkBit1B(CPUInstructionParameters)
{
	checkBit(cpu, cpu->B(), 1);
}

static void checkBit2B(CPUInstructionParameters)
{
	checkBit(cpu, cpu->B(), 2);
}

static void checkBit3B(CPUInstructionParameters)
{
	checkBit(cpu, cpu->B(), 3);
}

static void checkBit4B(CPUInstructionParameters)
{
	checkBit(cpu, cpu->B(), 4);
}

static void checkBit5B(CPUInstructionParameters)
{
	checkBit(cpu, cpu->B(), 5);
}

static void checkBit6B(CPUInstructionParameters)
{
	checkBit(cpu, cpu->B(), 6);
}

static void checkBit7B(CPUInstructionParameters)
{
	checkBit(cpu, cpu->B(), 7);
}

static void checkBit0C(CPUInstructionParameters)
{
	checkBit(cpu, cpu->C(), 0);
}

static void checkBit1C(CPUInstructionParameters)
{
	checkBit(cpu, cpu->C(), 1);
}

static void checkBit2C(CPUInstructionParameters)
{
	checkBit(cpu, cpu->C(), 2);
}

static void checkBit3C(CPUInstructionParameters)
{
	checkBit(cpu, cpu->C(), 3);
}

static void checkBit4C(CPUInstructionParameters)
{
	checkBit(cpu, cpu->C(), 4);
}

static void checkBit5C(CPUInstructionParameters)
{
	checkBit(cpu, cpu->C(), 5);
}

static void checkBit6C(CPUInstructionParameters)
{
	checkBit(cpu, cpu->C(), 6);
}

static void checkBit7C(CPUInstructionParameters)
{
	checkBit(cpu, cpu->C(), 7);
}


static void checkBit0D(CPUInstructionParameters)
{
	checkBit(cpu, cpu->D(), 0);
}

static void checkBit1D(CPUInstructionParameters)
{
	checkBit(cpu, cpu->D(), 1);
}

static void checkBit2D(CPUInstructionParameters)
{
	checkBit(cpu, cpu->D(), 2);
}

static void checkBit3D(CPUInstructionParameters)
{
	checkBit(cpu, cpu->D(), 3);
}

static void checkBit4D(CPUInstructionParameters)
{
	checkBit(cpu, cpu->D(), 4);
}

static void checkBit5D(CPUInstructionParameters)
{
	checkBit(cpu, cpu->D(), 5);
}

static void checkBit6D(CPUInstructionParameters)
{
	checkBit(cpu, cpu->D(), 6);
}

static void checkBit7D(CPUInstructionParameters)
{
	checkBit(cpu, cpu->D(), 7);
}

static void checkBit0E(CPUInstructionParameters)
{
	checkBit(cpu, cpu->E(), 0);
}

static void checkBit1E(CPUInstructionParameters)
{
	checkBit(cpu, cpu->E(), 1);
}

static void checkBit2E(CPUInstructionParameters)
{
	checkBit(cpu, cpu->E(), 2);
}

static void checkBit3E(CPUInstructionParameters)
{
	checkBit(cpu, cpu->E(), 3);
}

static void checkBit4E(CPUInstructionParameters)
{
	checkBit(cpu, cpu->E(), 4);
}

static void checkBit5E(CPUInstructionParameters)
{
	checkBit(cpu, cpu->E(), 5);
}

static void checkBit6E(CPUInstructionParameters)
{
	checkBit(cpu, cpu->E(), 6);
}

static void checkBit7E(CPUInstructionParameters)
{
	checkBit(cpu, cpu->E(), 7);
}

static void checkBit0H(CPUInstructionParameters)
{
	checkBit(cpu, cpu->H(), 0);
}

static void checkBit1H(CPUInstructionParameters)
{
	checkBit(cpu, cpu->H(), 1);
}

static void checkBit2H(CPUInstructionParameters)
{
	checkBit(cpu, cpu->H(), 2);
}

static void checkBit3H(CPUInstructionParameters)
{
	checkBit(cpu, cpu->H(), 3);
}

static void checkBit4H(CPUInstructionParameters)
{
	checkBit(cpu, cpu->H(), 4);
}

static void checkBit5H(CPUInstructionParameters)
{
	checkBit(cpu, cpu->H(), 5);
}

static void checkBit6H(CPUInstructionParameters)
{
	checkBit(cpu, cpu->H(), 6);
}

static void checkBit7H(CPUInstructionParameters)
{
	checkBit(cpu, cpu->H(), 7);
}

static void checkBit0L(CPUInstructionParameters)
{
	checkBit(cpu, cpu->L(), 0);
}

static void checkBit1L(CPUInstructionParameters)
{
	checkBit(cpu, cpu->L(), 1);
}

static void checkBit2L(CPUInstructionParameters)
{
	checkBit(cpu, cpu->L(), 2);
}

static void checkBit3L(CPUInstructionParameters)
{
	checkBit(cpu, cpu->L(), 3);
}

static void checkBit4L(CPUInstructionParameters)
{
	checkBit(cpu, cpu->L(), 4);
}

static void checkBit5L(CPUInstructionParameters)
{
	checkBit(cpu, cpu->L(), 5);
}

static void checkBit6L(CPUInstructionParameters)
{
	checkBit(cpu, cpu->L(), 6);
}

static void checkBit7L(CPUInstructionParameters)
{
	checkBit(cpu, cpu->L(), 7);
}

static void checkBit0HLAddress(CPUInstructionParameters)
{
	checkBit(cpu, bus->read(cpu->HL()), 0);
}

static void checkBit1HLAddress(CPUInstructionParameters)
{
	checkBit(cpu, bus->read(cpu->HL()), 1);
}

static void checkBit2HLAddress(CPUInstructionParameters)
{
	checkBit(cpu, bus->read(cpu->HL()), 2);
}

static void checkBit3HLAddress(CPUInstructionParameters)
{
	checkBit(cpu, bus->read(cpu->HL()), 3);
}

static void checkBit4HLAddress(CPUInstructionParameters)
{
	checkBit(cpu, bus->read(cpu->HL()), 4);
}

static void checkBit5HLAddress(CPUInstructionParameters)
{
	checkBit(cpu, bus->read(cpu->HL()), 5);
}

static void checkBit6HLAddress(CPUInstructionParameters)
{
	checkBit(cpu, bus->read(cpu->HL()), 6);
}

static void checkBit7HLAddress(CPUInstructionParameters)
{
	checkBit(cpu, bus->read(cpu->HL()), 7);
}

static void resetBit0A(CPUInstructionParameters)
{
	clearBit(cpu->A(), 0);
}

static void resetBit1A(CPUInstructionParameters)
{
	clearBit(cpu->A(), 1);
}

static void resetBit2A(CPUInstructionParameters)
{
	clearBit(cpu->A(), 2);
}

static void resetBit3A(CPUInstructionParameters)
{
	clearBit(cpu->A(), 3);
}

static void resetBit4A(CPUInstructionParameters)
{
	clearBit(cpu->A(), 4);
}

static void resetBit5A(CPUInstructionParameters)
{
	clearBit(cpu->A(), 5);
}

static void resetBit6A(CPUInstructionParameters)
{
	clearBit(cpu->A(), 6);
}

static void resetBit7A(CPUInstructionParameters)
{
	clearBit(cpu->A(), 7);
}

static void resetBit0B(CPUInstructionParameters)
{
	clearBit(cpu->B(), 0);
}

static void resetBit1B(CPUInstructionParameters)
{
	clearBit(cpu->B(), 1);
}

static void resetBit2B(CPUInstructionParameters)
{
	clearBit(cpu->B(), 2);
}

static void resetBit3B(CPUInstructionParameters)
{
	clearBit(cpu->B(), 3);
}

static void resetBit4B(CPUInstructionParameters)
{
	clearBit(cpu->B(), 4);
}

static void resetBit5B(CPUInstructionParameters)
{
	clearBit(cpu->B(), 5);
}

static void resetBit6B(CPUInstructionParameters)
{
	clearBit(cpu->B(), 6);
}

static void resetBit7B(CPUInstructionParameters)
{
	clearBit(cpu->B(), 7);
}

static void resetBit0C(CPUInstructionParameters)
{
	clearBit(cpu->C(), 0);
}

static void resetBit1C(CPUInstructionParameters)
{
	clearBit(cpu->C(), 1);
}

static void resetBit2C(CPUInstructionParameters)
{
	clearBit(cpu->C(), 2);
}

static void resetBit3C(CPUInstructionParameters)
{
	clearBit(cpu->C(), 3);
}

static void resetBit4C(CPUInstructionParameters)
{
	clearBit(cpu->C(), 4);
}

static void resetBit5C(CPUInstructionParameters)
{
	clearBit(cpu->C(), 5);
}

static void resetBit6C(CPUInstructionParameters)
{
	clearBit(cpu->C(), 6);
}

static void resetBit7C(CPUInstructionParameters)
{
	clearBit(cpu->C(), 7);
}


static void resetBit0D(CPUInstructionParameters)
{
	clearBit(cpu->D(), 0);
}

static void resetBit1D(CPUInstructionParameters)
{
	clearBit(cpu->D(), 1);
}

static void resetBit2D(CPUInstructionParameters)
{
	clearBit(cpu->D(), 2);
}

static void resetBit3D(CPUInstructionParameters)
{
	clearBit(cpu->D(), 3);
}

static void resetBit4D(CPUInstructionParameters)
{
	clearBit(cpu->D(), 4);
}

static void resetBit5D(CPUInstructionParameters)
{
	clearBit(cpu->D(), 5);
}

static void resetBit6D(CPUInstructionParameters)
{
	clearBit(cpu->D(), 6);
}

static void resetBit7D(CPUInstructionParameters)
{
	clearBit(cpu->D(), 7);
}

static void resetBit0E(CPUInstructionParameters)
{
	clearBit(cpu->E(), 0);
}

static void resetBit1E(CPUInstructionParameters)
{
	clearBit(cpu->E(), 1);
}

static void resetBit2E(CPUInstructionParameters)
{
	clearBit(cpu->E(), 2);
}

static void resetBit3E(CPUInstructionParameters)
{
	clearBit(cpu->E(), 3);
}

static void resetBit4E(CPUInstructionParameters)
{
	clearBit(cpu->E(), 4);
}

static void resetBit5E(CPUInstructionParameters)
{
	clearBit(cpu->E(), 5);
}

static void resetBit6E(CPUInstructionParameters)
{
	clearBit(cpu->E(), 6);
}

static void resetBit7E(CPUInstructionParameters)
{
	clearBit(cpu->E(), 7);
}

static void resetBit0H(CPUInstructionParameters)
{
	clearBit(cpu->H(), 0);
}

static void resetBit1H(CPUInstructionParameters)
{
	clearBit(cpu->H(), 1);
}

static void resetBit2H(CPUInstructionParameters)
{
	clearBit(cpu->H(), 2);
}

static void resetBit3H(CPUInstructionParameters)
{
	clearBit(cpu->H(), 3);
}

static void resetBit4H(CPUInstructionParameters)
{
	clearBit(cpu->H(), 4);
}

static void resetBit5H(CPUInstructionParameters)
{
	clearBit(cpu->H(), 5);
}

static void resetBit6H(CPUInstructionParameters)
{
	clearBit(cpu->H(), 6);
}

static void resetBit7H(CPUInstructionParameters)
{
	clearBit(cpu->H(), 7);
}

static void resetBit0L(CPUInstructionParameters)
{
	clearBit(cpu->L(), 0);
}

static void resetBit1L(CPUInstructionParameters)
{
	clearBit(cpu->L(), 1);
}

static void resetBit2L(CPUInstructionParameters)
{
	clearBit(cpu->L(), 2);
}

static void resetBit3L(CPUInstructionParameters)
{
	clearBit(cpu->L(), 3);
}

static void resetBit4L(CPUInstructionParameters)
{
	clearBit(cpu->L(), 4);
}

static void resetBit5L(CPUInstructionParameters)
{
	clearBit(cpu->L(), 5);
}

static void resetBit6L(CPUInstructionParameters)
{
	clearBit(cpu->L(), 6);
}

static void resetBit7L(CPUInstructionParameters)
{
	clearBit(cpu->L(), 7);
}

static void resetBit0HLAddress(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	clearBit(val, 0);
	bus->write(cpu->HL(), val);
}

static void resetBit1HLAddress(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	clearBit(val, 1);
	bus->write(cpu->HL(), val);
}

static void resetBit2HLAddress(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	clearBit(val, 2);
	bus->write(cpu->HL(), val);
}

static void resetBit3HLAddress(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	clearBit(val, 3);
	bus->write(cpu->HL(), val);
}

static void resetBit4HLAddress(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	clearBit(val, 4);
	bus->write(cpu->HL(), val);
}

static void resetBit5HLAddress(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	clearBit(val, 5);
	bus->write(cpu->HL(), val);
}

static void resetBit6HLAddress(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	clearBit(val, 6);
	bus->write(cpu->HL(), val);
}

static void resetBit7HLAddress(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	clearBit(val, 7);
	bus->write(cpu->HL(), val);
}

static void setBit0A(CPUInstructionParameters)
{
	setBit(cpu->A(), 0);
}

static void setBit1A(CPUInstructionParameters)
{
	setBit(cpu->A(), 1);
}

static void setBit2A(CPUInstructionParameters)
{
	setBit(cpu->A(), 2);
}

static void setBit3A(CPUInstructionParameters)
{
	setBit(cpu->A(), 3);
}

static void setBit4A(CPUInstructionParameters)
{
	setBit(cpu->A(), 4);
}

static void setBit5A(CPUInstructionParameters)
{
	setBit(cpu->A(), 5);
}

static void setBit6A(CPUInstructionParameters)
{
	setBit(cpu->A(), 6);
}

static void setBit7A(CPUInstructionParameters)
{
	setBit(cpu->A(), 7);
}

static void setBit0B(CPUInstructionParameters)
{
	setBit(cpu->B(), 0);
}

static void setBit1B(CPUInstructionParameters)
{
	setBit(cpu->B(), 1);
}

static void setBit2B(CPUInstructionParameters)
{
	setBit(cpu->B(), 2);
}

static void setBit3B(CPUInstructionParameters)
{
	setBit(cpu->B(), 3);
}

static void setBit4B(CPUInstructionParameters)
{
	setBit(cpu->B(), 4);
}

static void setBit5B(CPUInstructionParameters)
{
	setBit(cpu->B(), 5);
}

static void setBit6B(CPUInstructionParameters)
{
	setBit(cpu->B(), 6);
}

static void setBit7B(CPUInstructionParameters)
{
	setBit(cpu->B(), 7);
}

static void setBit0C(CPUInstructionParameters)
{
	setBit(cpu->C(), 0);
}

static void setBit1C(CPUInstructionParameters)
{
	setBit(cpu->C(), 1);
}

static void setBit2C(CPUInstructionParameters)
{
	setBit(cpu->C(), 2);
}

static void setBit3C(CPUInstructionParameters)
{
	setBit(cpu->C(), 3);
}

static void setBit4C(CPUInstructionParameters)
{
	setBit(cpu->C(), 4);
}

static void setBit5C(CPUInstructionParameters)
{
	setBit(cpu->C(), 5);
}

static void setBit6C(CPUInstructionParameters)
{
	setBit(cpu->C(), 6);
}

static void setBit7C(CPUInstructionParameters)
{
	setBit(cpu->C(), 7);
}


static void setBit0D(CPUInstructionParameters)
{
	setBit(cpu->D(), 0);
}

static void setBit1D(CPUInstructionParameters)
{
	setBit(cpu->D(), 1);
}

static void setBit2D(CPUInstructionParameters)
{
	setBit(cpu->D(), 2);
}

static void setBit3D(CPUInstructionParameters)
{
	setBit(cpu->D(), 3);
}

static void setBit4D(CPUInstructionParameters)
{
	setBit(cpu->D(), 4);
}

static void setBit5D(CPUInstructionParameters)
{
	setBit(cpu->D(), 5);
}

static void setBit6D(CPUInstructionParameters)
{
	setBit(cpu->D(), 6);
}

static void setBit7D(CPUInstructionParameters)
{
	setBit(cpu->D(), 7);
}

static void setBit0E(CPUInstructionParameters)
{
	setBit(cpu->E(), 0);
}

static void setBit1E(CPUInstructionParameters)
{
	setBit(cpu->E(), 1);
}

static void setBit2E(CPUInstructionParameters)
{
	setBit(cpu->E(), 2);
}

static void setBit3E(CPUInstructionParameters)
{
	setBit(cpu->E(), 3);
}

static void setBit4E(CPUInstructionParameters)
{
	setBit(cpu->E(), 4);
}

static void setBit5E(CPUInstructionParameters)
{
	setBit(cpu->E(), 5);
}

static void setBit6E(CPUInstructionParameters)
{
	setBit(cpu->E(), 6);
}

static void setBit7E(CPUInstructionParameters)
{
	setBit(cpu->E(), 7);
}

static void setBit0H(CPUInstructionParameters)
{
	setBit(cpu->H(), 0);
}

static void setBit1H(CPUInstructionParameters)
{
	setBit(cpu->H(), 1);
}

static void setBit2H(CPUInstructionParameters)
{
	setBit(cpu->H(), 2);
}

static void setBit3H(CPUInstructionParameters)
{
	setBit(cpu->H(), 3);
}

static void setBit4H(CPUInstructionParameters)
{
	setBit(cpu->H(), 4);
}

static void setBit5H(CPUInstructionParameters)
{
	setBit(cpu->H(), 5);
}

static void setBit6H(CPUInstructionParameters)
{
	setBit(cpu->H(), 6);
}

static void setBit7H(CPUInstructionParameters)
{
	setBit(cpu->H(), 7);
}

static void setBit0L(CPUInstructionParameters)
{
	setBit(cpu->L(), 0);
}

static void setBit1L(CPUInstructionParameters)
{
	setBit(cpu->L(), 1);
}

static void setBit2L(CPUInstructionParameters)
{
	setBit(cpu->L(), 2);
}

static void setBit3L(CPUInstructionParameters)
{
	setBit(cpu->L(), 3);
}

static void setBit4L(CPUInstructionParameters)
{
	setBit(cpu->L(), 4);
}

static void setBit5L(CPUInstructionParameters)
{
	setBit(cpu->L(), 5);
}

static void setBit6L(CPUInstructionParameters)
{
	setBit(cpu->L(), 6);
}

static void setBit7L(CPUInstructionParameters)
{
	setBit(cpu->L(), 7);
}

static void setBit0HLAddress(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	setBit(val, 0);
	bus->write(cpu->HL(), val);
}

static void setBit1HLAddress(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	setBit(val, 1);
	bus->write(cpu->HL(), val);
}

static void setBit2HLAddress(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	setBit(val, 2);
	bus->write(cpu->HL(), val);
}

static void setBit3HLAddress(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	setBit(val, 3);
	bus->write(cpu->HL(), val);
}

static void setBit4HLAddress(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	setBit(val, 4);
	bus->write(cpu->HL(), val);
}

static void setBit5HLAddress(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	setBit(val, 5);
	bus->write(cpu->HL(), val);
}

static void setBit6HLAddress(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	setBit(val, 6);
	bus->write(cpu->HL(), val);
}

static void setBit7HLAddress(CPUInstructionParameters)
{
	auto val = bus->read(cpu->HL());
	setBit(val, 7);
	bus->write(cpu->HL(), val);
}







void ggb::callAddress(CPUState* cpu, BUS* bus, uint16_t address)
{
	pushOnStack(cpu, bus, cpu->InstructionPointer());
	cpu->InstructionPointer() = address;
}

ggb::OPCodes::OPCodes()
{
	initOpcodesArray();
}

int OPCodes::execute(uint16_t opCode, ggb::CPUState* cpu, ggb::BUS* bus) const
{
	const OPCode* toExecute = nullptr;
	if (opCode == 0xCB) 
	{
		auto extendedOpcode = read(cpu, bus);
		toExecute = &m_extendedOpcodes[extendedOpcode];
	}
	else 
	{
		toExecute = &m_opcodes[opCode];
	}

	bool branchTaken = false;
	toExecute->func(cpu, bus, &branchTaken);

	if (branchTaken) 
	{
		assert(toExecute->branchCycleCount != 0);
		return toExecute->branchCycleCount;
	}
	return toExecute->baseCycleCount;
}

const std::string& OPCodes::getMnemonic(uint16_t opCode) const
{
	return m_opcodes[opCode].mnemonic;
}

void ggb::OPCodes::setOpcode(OPCode&& opcode)
{
	assert(opcode.id == m_counter);
	m_opcodes[m_counter] = std::move(opcode);
	++m_counter;
}

void ggb::OPCodes::setExtendedOpcode(OPCode&& opcode)
{
	assert(opcode.id == m_extendedOpcodeCounter);
	m_extendedOpcodes[m_extendedOpcodeCounter] = std::move(opcode);
	++m_extendedOpcodeCounter;
}

void ggb::OPCodes::initOpcodesArray()
{
	m_opcodes = std::vector<OPCode>(0xFF + 1);
	m_extendedOpcodes = std::vector<OPCode>(0xFF + 1);

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
	setOpcode({ 0x10,stop, 0, "STOP" });
	setOpcode({ 0x11,loadTwoBytesIntoDE, 12, "LD DE,u16" });
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
	setOpcode({ 0x20,jumpRelativeNotZeroToValue, 8, "JR NZ,i8", 12 });
	setOpcode({ 0x21,loadTwoBytesIntoHL, 12, "LD HL,u16" });
	setOpcode({ 0x22,loadAIntoHLAddressAndInc, 8, "LD (HL+),A" });
	setOpcode({ 0x23,incrementHL, 8, "INC HL" });
	setOpcode({ 0x24,incrementH, 4, "INC H" });
	setOpcode({ 0x25,decrementH, 4, "DEC H" });
	setOpcode({ 0x26,loadValueIntoH, 8, "LD H,u8" });
	setOpcode({ 0x27,decimalAdjustAccumulator, 4, "DAA" });
	setOpcode({ 0x28,jumpRealativeZeroToValue, 8, "JR Z,i8", 12 });
	setOpcode({ 0x29,addHLToHL, 8, "ADD HL,HL" });
	setOpcode({ 0x2A,loadHLAddressIntoAIncrementHL, 8, "LD A,(HL+)" });
	setOpcode({ 0x2B,decrementHL, 8, "DEC HL" });
	setOpcode({ 0x2C,incrementL, 4, "INC L" });
	setOpcode({ 0x2D,decrementL, 4, "DEC L" });
	setOpcode({ 0x2E,loadValueIntoL, 8, "LD L,u8" });
	setOpcode({ 0x2F,complementAccumulator, 4, "CPL" });
	setOpcode({ 0x30,jumpRelativeNotCarryToValue, 8, "JR NC,i8", 12 });
	setOpcode({ 0x31,loadTwoBytesIntoStackPointer, 12, "LD SP,u16" });
	setOpcode({ 0x32,loadAIntoHLAddressAndDec, 8, "LD (HL-),A" });
	setOpcode({ 0x33,incrementSP, 8, "INC SP" });
	setOpcode({ 0x34,incrementAddressHL, 12, "INC (HL)" });
	setOpcode({ 0x35,decrementAddressHL, 12, "DEC (HL)" });
	setOpcode({ 0x36,loadValueIntoAddressHL, 12, "LD (HL),u8" });
	setOpcode({ 0x37,setCarryFlag, 4, "SCF" });
	setOpcode({ 0x38,jumpRealativeCarryToValue, 8, "JR C,i8", 12 });
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
	setOpcode({ 0x45,loadLIntoB, 4, "LD B,L" });
	setOpcode({ 0x46,loadAddressHLIntoB, 8, "LD B,(HL)" });
	setOpcode({ 0x47,loadAIntoB, 4, "LD B,A" });
	setOpcode({ 0x48,loadBIntoC, 4, "LD C,B" });
	setOpcode({ 0x49,loadCIntoC, 4, "LD C,C" });
	setOpcode({ 0x4A,loadDIntoC, 4, "LD C,D" });
	setOpcode({ 0x4B,loadEIntoC, 4, "LD C,E" });
	setOpcode({ 0x4C,loadHIntoC, 4, "LD C,H" });
	setOpcode({ 0x4D,loadLIntoC, 4, "LD C,L" });
	setOpcode({ 0x4E,loadHLAddressIntoC, 8, "LD C,(HL)" });
	setOpcode({ 0x4F,loadAIntoC, 4, "LD C,A" });
	setOpcode({ 0x50,loadBIntoD, 4, "LD D,B" });
	setOpcode({ 0x51,loadCIntoD, 4, "LD D,C" });
	setOpcode({ 0x52,loadDIntoD, 4, "LD D,D" });
	setOpcode({ 0x53,loadEIntoD, 4, "LD D,E" });
	setOpcode({ 0x54,loadHIntoD, 4, "LD D,H" });
	setOpcode({ 0x55,loadLIntoD, 4, "LD D,L" });
	setOpcode({ 0x56,loadHLAddressIntoD, 8, "LD D,(HL)" });
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
	setOpcode({ 0x76,halt, 0, "HALT" });
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
	setOpcode({ 0xB0,bitwiseORAAndB, 4, "OR A,B" });
	setOpcode({ 0xB1,bitwiseORAAndC, 4, "OR A,C" });
	setOpcode({ 0xB2,bitwiseORAAndD, 4, "OR A,D" });
	setOpcode({ 0xB3,bitwiseORAAndE, 4, "OR A,E" });
	setOpcode({ 0xB4,bitwiseORAAndH, 4, "OR A,H" });
	setOpcode({ 0xB5,bitwiseORAAndL, 4, "OR A,L" });
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
	setOpcode({ 0xC0,returnNotZero, 8, "RET NZ", 20 });
	setOpcode({ 0xC1,popBC, 12, "POP BC" });
	setOpcode({ 0xC2,jumpNotZeroToNumber, 12, "JP NZ,u16", 16 });
	setOpcode({ 0xC3,jumpToNumber, 16, "JP u16" });
	setOpcode({ 0xC4,callNotZeroNumber, 12, "CALL NZ,u16", 24 });
	setOpcode({ 0xC5,pushBC, 16, "PUSH BC" });
	setOpcode({ 0xC6,addNumberToA, 8, "ADD A,u8" });
	setOpcode({ 0xC7,restart00, 16, "RST 00h" });
	setOpcode({ 0xC8,returnZero, 8, "RET Z", 20 });
	setOpcode({ 0xC9,returnInstr, 16, "RET" });
	setOpcode({ 0xCA,jumpZeroToNumber, 12, "JP Z,u16", 16 });
	setOpcode({ 0xCB,prefixOPCode, 0, "PREFIX CB" });
	setOpcode({ 0xCC,callZeroNumber, 12, "CALL Z,u16", 24 });
	setOpcode({ 0xCD,callInstr, 24, "CALL u16" });
	setOpcode({ 0xCE,addNumberAndCarryToA, 8, "ADC A,u8" });
	setOpcode({ 0xCF,restart08, 16, "RST 08h" });
	setOpcode({ 0xD0,returnNotCarry, 8, "RET NC", 20 });
	setOpcode({ 0xD1,popDE, 12, "POP DE" });
	setOpcode({ 0xD2,jumpNotCarryToNumber, 12, "JP NC,u16", 16 });
	setOpcode({ 0xD3,invalidInstruction, 0, "D3=INVALID" });
	setOpcode({ 0xD4,callNotCarryNumber, 12, "CALL NC,u16", 24 });
	setOpcode({ 0xD5,pushDE, 16, "PUSH DE" });
	setOpcode({ 0xD6,subNumberFromA, 8, "SUB A,u8" });
	setOpcode({ 0xD7,restart10, 16, "RST 10h" });
	setOpcode({ 0xD8,returnCarry, 8, "RET C", 20 });
	setOpcode({ 0xD9,returnFromInterruptHandler, 16, "RETI" });
	setOpcode({ 0xDA,jumpCarryToNumber, 12, "JP C,u16", 16 });
	setOpcode({ 0xDB,invalidInstruction, 0, "DB=INVALID" });
	setOpcode({ 0xDC,callCarryNumber, 12, "CALL C,u16", 24 });
	setOpcode({ 0xDD,invalidInstruction, 0, "DD=INVALID" });
	setOpcode({ 0xDE,subtractNumberAndCarryFromA, 8, "SBC A,u8" });
	setOpcode({ 0xDF,restart18, 16, "RST 18h" });
	setOpcode({ 0xE0,loadAIntoSpecialAddressPlusNumber, 12, "LD (FF00+u8),A" });
	setOpcode({ 0xE1,POPHL, 12, "POP HL" });
	setOpcode({ 0xE2,loadAIntoSpecialAddressPlusC, 8, "LD (FF00+C),A" });
	setOpcode({ 0xE3,invalidInstruction, 0, "E3=INVALID" });
	setOpcode({ 0xE4,invalidInstruction, 0, "E4=INVALID" });
	setOpcode({ 0xE5,pushHL, 16, "PUSH HL" });
	setOpcode({ 0xE6,bitwiseAndAAndNumber, 8, "AND A,u8" });
	setOpcode({ 0xE7,restart20, 16, "RST 20h" });
	setOpcode({ 0xE8,addNumberToStackPointer, 16, "ADD SP,i8" });
	setOpcode({ 0xE9,jumpToHL, 4, "JP HL" });
	setOpcode({ 0xEA,loadAIntoNumberAddress, 16, "LD (u16),A" });
	setOpcode({ 0xEB,invalidInstruction, 0, "EB=INVALID" });
	setOpcode({ 0xEC,invalidInstruction, 0, "EC=INVALID" });
	setOpcode({ 0xED,invalidInstruction, 0, "ED=INVALID" });
	setOpcode({ 0xEE,xorAAndNumber, 8, "XOR A,u8" });
	setOpcode({ 0xEF,restart28, 16, "RST 28h" });
	setOpcode({ 0xF0,loadSpecialAddressPlusNumberIntoA, 12, "LD A,(FF00+u8)" });
	setOpcode({ 0xF1,POPAF, 12, "POP AF" });
	setOpcode({ 0xF2,loadSpecialAddressPlusCIntoA, 8, "LD A,(FF00+C)" });
	setOpcode({ 0xF3,disableInterrupts, 4, "DI" });
	setOpcode({ 0xF4,invalidInstruction, 0, "F4=INVALID" });
	setOpcode({ 0xF5,pushAF, 16, "PUSH AF" });
	setOpcode({ 0xF6,bitwiseOrAAndNumber, 8, "OR A,u8" });
	setOpcode({ 0xF7,restart30, 16, "RST 30h" });
	setOpcode({ 0xF8,loadStackPointerPlusNumberIntoHL, 12, "LD HL,SP+i8" });
	setOpcode({ 0xF9,loadHLIntoStackPointer, 8, "LD SP,HL" });
	setOpcode({ 0xFA,loadAddressIntoA, 16, "LD A,(u16)" });
	setOpcode({ 0xFB,enableInterrupts, 4, "EI" });
	setOpcode({ 0xFC,invalidInstruction, 0, "FC=INVALID" });
	setOpcode({ 0xFD,invalidInstruction, 0, "FD=INVALID" });
	setOpcode({ 0xFE,compareAWithNumber, 8, "CP A,u8" });
	setOpcode({ 0xFF,restart38, 16, "RST 38h" });

	// Extended OPcodes start
	setExtendedOpcode({ 0x00,rotateBLeft, 8, "RLC B" });
	setExtendedOpcode({ 0x01,rotateCLeft, 8, "RLC C" });
	setExtendedOpcode({ 0x02,rotateDLeft, 8, "RLC D" });
	setExtendedOpcode({ 0x03,rotateELeft, 8, "RLC E" });
	setExtendedOpcode({ 0x04,rotateHLeft, 8, "RLC H" });
	setExtendedOpcode({ 0x05,rotateLLeft, 8, "RLC L" });
	setExtendedOpcode({ 0x06,rotateHLAddressLeft, 16, "RLC (HL)" });
	setExtendedOpcode({ 0x07,rotateALeftSetZero, 8, "RLC A" });
	setExtendedOpcode({ 0x08,rotateBRight, 8, "RRC B" });
	setExtendedOpcode({ 0x09,rotateCRight, 8, "RRC C" });
	setExtendedOpcode({ 0x0A,rotateDRight, 8, "RRC D" });
	setExtendedOpcode({ 0x0B,rotateERight, 8, "RRC E" });
	setExtendedOpcode({ 0x0C,rotateHRight, 8, "RRC H" });
	setExtendedOpcode({ 0x0D,rotateLRight, 8, "RRC L" });
	setExtendedOpcode({ 0x0E,rotateHLAddressRight, 16, "RRC (HL)" });
	setExtendedOpcode({ 0x0F,rotateARightSetZero, 8, "RRC A" });
	setExtendedOpcode({ 0x10,rotateBLeftThroughCarry, 8, "RL B" });
	setExtendedOpcode({ 0x11,rotateCLeftThroughCarry, 8, "RL C" });
	setExtendedOpcode({ 0x12,rotateDLeftThroughCarry, 8, "RL D" });
	setExtendedOpcode({ 0x13,rotateELeftThroughCarry, 8, "RL E" });
	setExtendedOpcode({ 0x14,rotateHLeftThroughCarry, 8, "RL H" });
	setExtendedOpcode({ 0x15,rotateLLeftThroughCarry, 8, "RL L" });
	setExtendedOpcode({ 0x16,rotateHLAddressLeftThroughCarry, 16, "RL (HL)" });
	setExtendedOpcode({ 0x17,rotateALeftThroughCarrySetZero, 8, "RL A" });
	setExtendedOpcode({ 0x18,rotateBRightThroughCarry, 8, "RR B" });
	setExtendedOpcode({ 0x19,rotateCRightThroughCarry, 8, "RR C" });
	setExtendedOpcode({ 0x1A,rotateDRightThroughCarry, 8, "RR D" });
	setExtendedOpcode({ 0x1B,rotateERightThroughCarry, 8, "RR E" });
	setExtendedOpcode({ 0x1C,rotateHRightThroughCarry, 8, "RR H" });
	setExtendedOpcode({ 0x1D,rotateLRightThroughCarry, 8, "RR L" });
	setExtendedOpcode({ 0x1E,rotateHLAddressRightThroughCarry, 16, "RR (HL)" });
	setExtendedOpcode({ 0x1F,rotateARightThroughCarrySetZero, 8, "RR A" });
	setExtendedOpcode({ 0x20,shiftBLeftArithmetically, 8, "SLA B" });
	setExtendedOpcode({ 0x21,shiftCLeftArithmetically, 8, "SLA C" });
	setExtendedOpcode({ 0x22,shiftDLeftArithmetically, 8, "SLA D" });
	setExtendedOpcode({ 0x23,shiftELeftArithmetically, 8, "SLA E" });
	setExtendedOpcode({ 0x24,shiftHLeftArithmetically, 8, "SLA H" });
	setExtendedOpcode({ 0x25,shiftLLeftArithmetically, 8, "SLA L" });
	setExtendedOpcode({ 0x26,shiftHLAddressLeftArithmetically, 16, "SLA (HL)" });
	setExtendedOpcode({ 0x27,shiftALeftArithmetically, 8, "SLA A" });
	setExtendedOpcode({ 0x28,shiftBRightArithmetically, 8, "SRA B" });
	setExtendedOpcode({ 0x29,shiftCRightArithmetically, 8, "SRA C" });
	setExtendedOpcode({ 0x2A,shiftDRightArithmetically, 8, "SRA D" });
	setExtendedOpcode({ 0x2B,shiftERightArithmetically, 8, "SRA E" });
	setExtendedOpcode({ 0x2C,shiftHRightArithmetically, 8, "SRA H" });
	setExtendedOpcode({ 0x2D,shiftLRightArithmetically, 8, "SRA L" });
	setExtendedOpcode({ 0x2E,shiftHLAddressRightArithmetically, 16, "SRA (HL)" });
	setExtendedOpcode({ 0x2F,shiftARightArithmetically, 8, "SRA A" });
	setExtendedOpcode({ 0x30,swapB, 8, "SWAP B" });
	setExtendedOpcode({ 0x31,swapC, 8, "SWAP C" });
	setExtendedOpcode({ 0x32,swapD, 8, "SWAP D" });
	setExtendedOpcode({ 0x33,swapE, 8, "SWAP E" });
	setExtendedOpcode({ 0x34,swapH, 8, "SWAP H" });
	setExtendedOpcode({ 0x35,swapL, 8, "SWAP L" });
	setExtendedOpcode({ 0x36,swapHLAddress, 16, "SWAP (HL)" });
	setExtendedOpcode({ 0x37,swapA, 8, "SWAP A" });
	setExtendedOpcode({ 0x38,shiftBRightLogically, 8, "SRL B" });
	setExtendedOpcode({ 0x39,shiftCRightLogically, 8, "SRL C" });
	setExtendedOpcode({ 0x3A,shiftDRightLogically, 8, "SRL D" });
	setExtendedOpcode({ 0x3B,shiftERightLogically, 8, "SRL E" });
	setExtendedOpcode({ 0x3C,shiftHRightLogically, 8, "SRL H" });
	setExtendedOpcode({ 0x3D,shiftLRightLogically, 8, "SRL L" });
	setExtendedOpcode({ 0x3E,shiftHLAddressRightLogically, 16, "SRL (HL)" });
	setExtendedOpcode({ 0x3F,shiftARightLogically, 8, "SRL A" });
	setExtendedOpcode({ 0x40,checkBit0B, 8, "BIT 0,B" });
	setExtendedOpcode({ 0x41,checkBit0C, 8, "BIT 0,C" });
	setExtendedOpcode({ 0x42,checkBit0D, 8, "BIT 0,D" });
	setExtendedOpcode({ 0x43,checkBit0E, 8, "BIT 0,E" });
	setExtendedOpcode({ 0x44,checkBit0H, 8, "BIT 0,H" });
	setExtendedOpcode({ 0x45,checkBit0L, 8, "BIT 0,L" });
	setExtendedOpcode({ 0x46,checkBit0HLAddress, 12, "BIT 0,(HL)" });
	setExtendedOpcode({ 0x47,checkBit0A, 8, "BIT 0,A" });
	setExtendedOpcode({ 0x48,checkBit1B, 8, "BIT 1,B" });
	setExtendedOpcode({ 0x49,checkBit1C, 8, "BIT 1,C" });
	setExtendedOpcode({ 0x4A,checkBit1D, 8, "BIT 1,D" });
	setExtendedOpcode({ 0x4B,checkBit1E, 8, "BIT 1,E" });
	setExtendedOpcode({ 0x4C,checkBit1H, 8, "BIT 1,H" });
	setExtendedOpcode({ 0x4D,checkBit1L, 8, "BIT 1,L" });
	setExtendedOpcode({ 0x4E,checkBit1HLAddress, 12, "BIT 1,(HL)" });
	setExtendedOpcode({ 0x4F,checkBit1A, 8, "BIT 1,A" });
	setExtendedOpcode({ 0x50,checkBit2B, 8, "BIT 2,B" });
	setExtendedOpcode({ 0x51,checkBit2C, 8, "BIT 2,C" });
	setExtendedOpcode({ 0x52,checkBit2D, 8, "BIT 2,D" });
	setExtendedOpcode({ 0x53,checkBit2E, 8, "BIT 2,E" });
	setExtendedOpcode({ 0x54,checkBit2H, 8, "BIT 2,H" });
	setExtendedOpcode({ 0x55,checkBit2L, 8, "BIT 2,L" });
	setExtendedOpcode({ 0x56,checkBit2HLAddress, 12, "BIT 2,(HL)" });
	setExtendedOpcode({ 0x57,checkBit2A, 8, "BIT 2,A" });
	setExtendedOpcode({ 0x58,checkBit3B, 8, "BIT 3,B" });
	setExtendedOpcode({ 0x59,checkBit3C, 8, "BIT 3,C" });
	setExtendedOpcode({ 0x5A,checkBit3D, 8, "BIT 3,D" });
	setExtendedOpcode({ 0x5B,checkBit3E, 8, "BIT 3,E" });
	setExtendedOpcode({ 0x5C,checkBit3H, 8, "BIT 3,H" });
	setExtendedOpcode({ 0x5D,checkBit3L, 8, "BIT 3,L" });
	setExtendedOpcode({ 0x5E,checkBit3HLAddress, 12, "BIT 3,(HL)" });
	setExtendedOpcode({ 0x5F,checkBit3A, 8, "BIT 3,A" });
	setExtendedOpcode({ 0x60,checkBit4B, 8, "BIT 4,B" });
	setExtendedOpcode({ 0x61,checkBit4C, 8, "BIT 4,C" });
	setExtendedOpcode({ 0x62,checkBit4D, 8, "BIT 4,D" });
	setExtendedOpcode({ 0x63,checkBit4E, 8, "BIT 4,E" });
	setExtendedOpcode({ 0x64,checkBit4H, 8, "BIT 4,H" });
	setExtendedOpcode({ 0x65,checkBit4L, 8, "BIT 4,L" });
	setExtendedOpcode({ 0x66,checkBit4HLAddress, 12, "BIT 4,(HL)" });
	setExtendedOpcode({ 0x67,checkBit4A, 8, "BIT 4,A" });
	setExtendedOpcode({ 0x68,checkBit5B, 8, "BIT 5,B" });
	setExtendedOpcode({ 0x69,checkBit5C, 8, "BIT 5,C" });
	setExtendedOpcode({ 0x6A,checkBit5D, 8, "BIT 5,D" });
	setExtendedOpcode({ 0x6B,checkBit5E, 8, "BIT 5,E" });
	setExtendedOpcode({ 0x6C,checkBit5H, 8, "BIT 5,H" });
	setExtendedOpcode({ 0x6D,checkBit5L, 8, "BIT 5,L" });
	setExtendedOpcode({ 0x6E,checkBit5HLAddress, 12, "BIT 5,(HL)" });
	setExtendedOpcode({ 0x6F,checkBit5A, 8, "BIT 5,A" });
	setExtendedOpcode({ 0x70,checkBit6B, 8, "BIT 6,B" });
	setExtendedOpcode({ 0x71,checkBit6C, 8, "BIT 6,C" });
	setExtendedOpcode({ 0x72,checkBit6D, 8, "BIT 6,D" });
	setExtendedOpcode({ 0x73,checkBit6E, 8, "BIT 6,E" });
	setExtendedOpcode({ 0x74,checkBit6H, 8, "BIT 6,H" });
	setExtendedOpcode({ 0x75,checkBit6L, 8, "BIT 6,L" });
	setExtendedOpcode({ 0x76,checkBit6HLAddress, 12, "BIT 6,(HL)" });
	setExtendedOpcode({ 0x77,checkBit6A, 8, "BIT 6,A" });
	setExtendedOpcode({ 0x78,checkBit7B, 8, "BIT 7,B" });
	setExtendedOpcode({ 0x79,checkBit7C, 8, "BIT 7,C" });
	setExtendedOpcode({ 0x7A,checkBit7D, 8, "BIT 7,D" });
	setExtendedOpcode({ 0x7B,checkBit7E, 8, "BIT 7,E" });
	setExtendedOpcode({ 0x7C,checkBit7H, 8, "BIT 7,H" });
	setExtendedOpcode({ 0x7D,checkBit7L, 8, "BIT 7,L" });
	setExtendedOpcode({ 0x7E,checkBit7HLAddress, 12, "BIT 7,(HL)" });
	setExtendedOpcode({ 0x7F,checkBit7A, 8, "BIT 7,A" });
	setExtendedOpcode({ 0x80,resetBit0B, 8, "RES 0,B" });
	setExtendedOpcode({ 0x81,resetBit0C, 8, "RES 0,C" });
	setExtendedOpcode({ 0x82,resetBit0D, 8, "RES 0,D" });
	setExtendedOpcode({ 0x83,resetBit0E, 8, "RES 0,E" });
	setExtendedOpcode({ 0x84,resetBit0H, 8, "RES 0,H" });
	setExtendedOpcode({ 0x85,resetBit0L, 8, "RES 0,L" });
	setExtendedOpcode({ 0x86,resetBit0HLAddress, 16, "RES 0,(HL)" });
	setExtendedOpcode({ 0x87,resetBit0A, 8, "RES 0,A" });
	setExtendedOpcode({ 0x88,resetBit1B, 8, "RES 1,B" });
	setExtendedOpcode({ 0x89,resetBit1C, 8, "RES 1,C" });
	setExtendedOpcode({ 0x8A,resetBit1D, 8, "RES 1,D" });
	setExtendedOpcode({ 0x8B,resetBit1E, 8, "RES 1,E" });
	setExtendedOpcode({ 0x8C,resetBit1H, 8, "RES 1,H" });
	setExtendedOpcode({ 0x8D,resetBit1L, 8, "RES 1,L" });
	setExtendedOpcode({ 0x8E,resetBit1HLAddress, 16, "RES 1,(HL)" });
	setExtendedOpcode({ 0x8F,resetBit1A, 8, "RES 1,A" });
	setExtendedOpcode({ 0x90,resetBit2B, 8, "RES 2,B" });
	setExtendedOpcode({ 0x91,resetBit2C, 8, "RES 2,C" });
	setExtendedOpcode({ 0x92,resetBit2D, 8, "RES 2,D" });
	setExtendedOpcode({ 0x93,resetBit2E, 8, "RES 2,E" });
	setExtendedOpcode({ 0x94,resetBit2H, 8, "RES 2,H" });
	setExtendedOpcode({ 0x95,resetBit2L, 8, "RES 2,L" });
	setExtendedOpcode({ 0x96,resetBit2HLAddress, 16, "RES 2,(HL)" });
	setExtendedOpcode({ 0x97,resetBit2A, 8, "RES 2,A" });
	setExtendedOpcode({ 0x98,resetBit3B, 8, "RES 3,B" });
	setExtendedOpcode({ 0x99,resetBit3C, 8, "RES 3,C" });
	setExtendedOpcode({ 0x9A,resetBit3D, 8, "RES 3,D" });
	setExtendedOpcode({ 0x9B,resetBit3E, 8, "RES 3,E" });
	setExtendedOpcode({ 0x9C,resetBit3H, 8, "RES 3,H" });
	setExtendedOpcode({ 0x9D,resetBit3L, 8, "RES 3,L" });
	setExtendedOpcode({ 0x9E,resetBit3HLAddress, 16, "RES 3,(HL)" });
	setExtendedOpcode({ 0x9F,resetBit3A, 8, "RES 3,A" });
	setExtendedOpcode({ 0xA0,resetBit4B, 8, "RES 4,B" });
	setExtendedOpcode({ 0xA1,resetBit4C, 8, "RES 4,C" });
	setExtendedOpcode({ 0xA2,resetBit4D, 8, "RES 4,D" });
	setExtendedOpcode({ 0xA3,resetBit4E, 8, "RES 4,E" });
	setExtendedOpcode({ 0xA4,resetBit4H, 8, "RES 4,H" });
	setExtendedOpcode({ 0xA5,resetBit4L, 8, "RES 4,L" });
	setExtendedOpcode({ 0xA6,resetBit4HLAddress, 16, "RES 4,(HL)" });
	setExtendedOpcode({ 0xA7,resetBit4A, 8, "RES 4,A" });
	setExtendedOpcode({ 0xA8,resetBit5B, 8, "RES 5,B" });
	setExtendedOpcode({ 0xA9,resetBit5C, 8, "RES 5,C" });
	setExtendedOpcode({ 0xAA,resetBit5D, 8, "RES 5,D" });
	setExtendedOpcode({ 0xAB,resetBit5E, 8, "RES 5,E" });
	setExtendedOpcode({ 0xAC,resetBit5H, 8, "RES 5,H" });
	setExtendedOpcode({ 0xAD,resetBit5L, 8, "RES 5,L" });
	setExtendedOpcode({ 0xAE,resetBit5HLAddress, 16, "RES 5,(HL)" });
	setExtendedOpcode({ 0xAF,resetBit5A, 8, "RES 5,A" });
	setExtendedOpcode({ 0xB0,resetBit6B, 8, "RES 6,B" });
	setExtendedOpcode({ 0xB1,resetBit6C, 8, "RES 6,C" });
	setExtendedOpcode({ 0xB2,resetBit6D, 8, "RES 6,D" });
	setExtendedOpcode({ 0xB3,resetBit6E, 8, "RES 6,E" });
	setExtendedOpcode({ 0xB4,resetBit6H, 8, "RES 6,H" });
	setExtendedOpcode({ 0xB5,resetBit6L, 8, "RES 6,L" });
	setExtendedOpcode({ 0xB6,resetBit6HLAddress, 16, "RES 6,(HL)" });
	setExtendedOpcode({ 0xB7,resetBit6A, 8, "RES 6,A" });
	setExtendedOpcode({ 0xB8,resetBit7B, 8, "RES 7,B" });
	setExtendedOpcode({ 0xB9,resetBit7C, 8, "RES 7,C" });
	setExtendedOpcode({ 0xBA,resetBit7D, 8, "RES 7,D" });
	setExtendedOpcode({ 0xBB,resetBit7E, 8, "RES 7,E" });
	setExtendedOpcode({ 0xBC,resetBit7H, 8, "RES 7,H" });
	setExtendedOpcode({ 0xBD,resetBit7L, 8, "RES 7,L" });
	setExtendedOpcode({ 0xBE,resetBit7HLAddress, 16, "RES 7,(HL)" });
	setExtendedOpcode({ 0xBF,resetBit7A, 8, "RES 7,A" });
	setExtendedOpcode({ 0xC0,setBit0B, 8, "SET 0,B" });
	setExtendedOpcode({ 0xC1,setBit0C, 8, "SET 0,C" });
	setExtendedOpcode({ 0xC2,setBit0D, 8, "SET 0,D" });
	setExtendedOpcode({ 0xC3,setBit0E, 8, "SET 0,E" });
	setExtendedOpcode({ 0xC4,setBit0H, 8, "SET 0,H" });
	setExtendedOpcode({ 0xC5,setBit0L, 8, "SET 0,L" });
	setExtendedOpcode({ 0xC6,setBit0HLAddress, 16, "SET 0,(HL)" });
	setExtendedOpcode({ 0xC7,setBit0A, 8, "SET 0,A" });
	setExtendedOpcode({ 0xC8,setBit1B, 8, "SET 1,B" });
	setExtendedOpcode({ 0xC9,setBit1C, 8, "SET 1,C" });
	setExtendedOpcode({ 0xCA,setBit1D, 8, "SET 1,D" });
	setExtendedOpcode({ 0xCB,setBit1E, 8, "SET 1,E" });
	setExtendedOpcode({ 0xCC,setBit1H, 8, "SET 1,H" });
	setExtendedOpcode({ 0xCD,setBit1L, 8, "SET 1,L" });
	setExtendedOpcode({ 0xCE,setBit1HLAddress, 16, "SET 1,(HL)" });
	setExtendedOpcode({ 0xCF,setBit1A, 8, "SET 1,A" });
	setExtendedOpcode({ 0xD0,setBit2B, 8, "SET 2,B" });
	setExtendedOpcode({ 0xD1,setBit2C, 8, "SET 2,C" });
	setExtendedOpcode({ 0xD2,setBit2D, 8, "SET 2,D" });
	setExtendedOpcode({ 0xD3,setBit2E, 8, "SET 2,E" });
	setExtendedOpcode({ 0xD4,setBit2H, 8, "SET 2,H" });
	setExtendedOpcode({ 0xD5,setBit2L, 8, "SET 2,L" });
	setExtendedOpcode({ 0xD6,setBit2HLAddress, 16, "SET 2,(HL)" });
	setExtendedOpcode({ 0xD7,setBit2A, 8, "SET 2,A" });
	setExtendedOpcode({ 0xD8,setBit3B, 8, "SET 3,B" });
	setExtendedOpcode({ 0xD9,setBit3C, 8, "SET 3,C" });
	setExtendedOpcode({ 0xDA,setBit3D, 8, "SET 3,D" });
	setExtendedOpcode({ 0xDB,setBit3E, 8, "SET 3,E" });
	setExtendedOpcode({ 0xDC,setBit3H, 8, "SET 3,H" });
	setExtendedOpcode({ 0xDD,setBit3L, 8, "SET 3,L" });
	setExtendedOpcode({ 0xDE,setBit3HLAddress, 16, "SET 3,(HL)" });
	setExtendedOpcode({ 0xDF,setBit3A, 8, "SET 3,A" });
	setExtendedOpcode({ 0xE0,setBit4B, 8, "SET 4,B" });
	setExtendedOpcode({ 0xE1,setBit4C, 8, "SET 4,C" });
	setExtendedOpcode({ 0xE2,setBit4D, 8, "SET 4,D" });
	setExtendedOpcode({ 0xE3,setBit4E, 8, "SET 4,E" });
	setExtendedOpcode({ 0xE4,setBit4H, 8, "SET 4,H" });
	setExtendedOpcode({ 0xE5,setBit4L, 8, "SET 4,L" });
	setExtendedOpcode({ 0xE6,setBit4HLAddress, 16, "SET 4,(HL)" });
	setExtendedOpcode({ 0xE7,setBit4A, 8, "SET 4,A" });
	setExtendedOpcode({ 0xE8,setBit5B, 8, "SET 5,B" });
	setExtendedOpcode({ 0xE9,setBit5C, 8, "SET 5,C" });
	setExtendedOpcode({ 0xEA,setBit5D, 8, "SET 5,D" });
	setExtendedOpcode({ 0xEB,setBit5E, 8, "SET 5,E" });
	setExtendedOpcode({ 0xEC,setBit5H, 8, "SET 5,H" });
	setExtendedOpcode({ 0xED,setBit5L, 8, "SET 5,L" });
	setExtendedOpcode({ 0xEE,setBit5HLAddress, 16, "SET 5,(HL)" });
	setExtendedOpcode({ 0xEF,setBit5A, 8, "SET 5,A" });
	setExtendedOpcode({ 0xF0,setBit6B, 8, "SET 6,B" });
	setExtendedOpcode({ 0xF1,setBit6C, 8, "SET 6,C" });
	setExtendedOpcode({ 0xF2,setBit6D, 8, "SET 6,D" });
	setExtendedOpcode({ 0xF3,setBit6E, 8, "SET 6,E" });
	setExtendedOpcode({ 0xF4,setBit6H, 8, "SET 6,H" });
	setExtendedOpcode({ 0xF5,setBit6L, 8, "SET 6,L" });
	setExtendedOpcode({ 0xF6,setBit6HLAddress, 16, "SET 6,(HL)" });
	setExtendedOpcode({ 0xF7,setBit6A, 8, "SET 6,A" });
	setExtendedOpcode({ 0xF8,setBit7B, 8, "SET 7,B" });
	setExtendedOpcode({ 0xF9,setBit7C, 8, "SET 7,C" });
	setExtendedOpcode({ 0xFA,setBit7D, 8, "SET 7,D" });
	setExtendedOpcode({ 0xFB,setBit7E, 8, "SET 7,E" });
	setExtendedOpcode({ 0xFC,setBit7H, 8, "SET 7,H" });
	setExtendedOpcode({ 0xFD,setBit7L, 8, "SET 7,L" });
	setExtendedOpcode({ 0xFE,setBit7HLAddress, 16, "SET 7,(HL)" });
	setExtendedOpcode({ 0xFF,setBit7A, 8, "SET 7,A" });
}
