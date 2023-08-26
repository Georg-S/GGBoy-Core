#pragma once
#include <cstdint>

namespace ggb 
{
	class CPUState
	{
	public:
		// 8 Bit registers
		uint8_t& A();
		uint8_t& F();
		uint8_t& B();
		uint8_t& C();
		uint8_t& D();
		uint8_t& E();
		uint8_t& H();
		uint8_t& L();
		// 16 Bit registers
		uint16_t& AF();
		uint16_t& BC();
		uint16_t& DE();
		uint16_t& HL();
		uint16_t& StackPointer();
		uint16_t& InstructionPointer();

		void setZeroFlag(bool value);
		bool getZeroFlag() const;
		void setSubtractionFlag(bool value); // Also known as negative flag
		bool getSubtractionFlag() const;
		void setHalfCarryFlag(bool value);
		bool getHalfCarryFlag() const;
		void setCarryFlag(bool value);
		bool getCarryFlag() const;
		void disableInterrupts();
		void enableInterrupts();
		bool interruptsEnabled() const;
		bool stop();
		bool resume();
		bool isStopped() const;

	private:
		uint8_t F() const;

		union { uint16_t AF; uint8_t regs[2]; } afUnion = {};
		union { uint16_t BC; uint8_t regs[2]; } bcUnion = {};
		union { uint16_t DE; uint8_t regs[2]; } deUnion = {};
		union { uint16_t HL; uint8_t regs[2]; } hlUnion = {};
		uint16_t m_stackPointer = 0;
		uint16_t m_instructionPointer = 0;
		bool m_interruptsEnabled = true;
		bool m_stopped = false;
	};

	void increment(CPUState* cpu, uint8_t& toIncrement);
	void decrement(CPUState* cpu, uint8_t& toDecrement);
	void add(CPUState* cpu, uint8_t& outNum, uint8_t num2);
	void add(CPUState* cpu, uint8_t& outNum, uint8_t num2, uint8_t carryFlag);
	void add(CPUState* cpu, uint16_t& outNum, uint16_t num2);
	void add(CPUState* cpu, uint16_t& outNum, int8_t num2);
	void sub(CPUState* cpu, uint8_t& outNum, uint8_t num2);
	void sub(CPUState* cpu, uint8_t& outReg, uint8_t reg2, uint8_t carryFlag);
	void compare(CPUState* cpu, uint8_t num, uint8_t num2);
	void bitwiseAnd(CPUState* cpu, uint8_t& outReg, uint8_t reg2);
	void bitwiseOR(CPUState* cpu, uint8_t& outReg, uint8_t reg2);
	void bitwiseXOR(CPUState* cpu, uint8_t& outReg, uint8_t reg2);
	void checkBit(CPUState* cpu, uint8_t num, int bit);
	void swap(CPUState* cpu, uint8_t& out); // Swaps the two nibbles of the number
	void rotateLeft(CPUState* cpu, uint8_t& out);
	void rotateLeftThroughCarry(CPUState* cpu, uint8_t& out);
	void rotateLeftSetZero(CPUState* cpu, uint8_t& out);
	void rotateLeftThroughCarrySetZero(CPUState* cpu, uint8_t& out);
	void shiftLeftArithmetically(CPUState* cpu, uint8_t& out);
	void rotateRight(CPUState* cpu, uint8_t& out);
	void rotateRightThroughCarry(CPUState* cpu, uint8_t& out);
	void rotateRightSetZero(CPUState* cpu, uint8_t& out);
	void rotateRightThroughCarrySetZero(CPUState* cpu, uint8_t& out);
	void shiftRightArithmetically(CPUState* cpu, uint8_t& out);
	void shiftRightLogically(CPUState* cpu, uint8_t& out);
}
