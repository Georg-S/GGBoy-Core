#pragma once
#include <cstdint>

namespace ggb 
{
	struct CPUState
	{
		uint8_t& A();
		uint8_t& F();
		uint8_t& B();
		uint8_t& C();
		uint8_t& D();
		uint8_t& E();
		uint8_t& H();
		uint8_t& L();

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

	private:
		uint8_t F() const;

		union { uint16_t AF; uint8_t regs[2]; } afUnion = {};
		union { uint16_t BC; uint8_t regs[2]; } bcUnion = {};
		union { uint16_t DE; uint8_t regs[2]; } deUnion = {};
		union { uint16_t HL; uint8_t regs[2]; } hlUnion = {};
		uint16_t stackPointer = 0;
		uint16_t instructionPointer = 0;
	};
}
