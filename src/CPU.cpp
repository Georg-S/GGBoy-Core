#include "CPU.hpp"

#include "CPUInstructions.hpp"
#include "Utility.hpp"
#include "Logging.hpp"
#include "Constants.hpp"

static void debugLog(const std::string& message) 
{
	bool debugLogEnabled = false;

	if (debugLogEnabled)
		ggb::logInfo(message);
};

void ggb::CPU::reset()
{
	m_cpuState.setZeroFlag(true);
	m_cpuState.setSubtractionFlag(false);
	m_cpuState.setHalfCarryFlag(false);
	m_cpuState.setCarryFlag(false);
	m_cpuState.A() = 0x11;
	m_cpuState.B() = 0x00;
	m_cpuState.C() = 0x00;
	m_cpuState.D() = 0xFF;
	m_cpuState.E() = 0x56;
	m_cpuState.H() = 0x00;
	m_cpuState.L() = 0x0D;
	m_cpuState.InstructionPointer() = 0x100;
	m_cpuState.StackPointer() = 0xFFFE;
	m_cpuState.disableInterrupts();
	m_cpuState.resume();
}

void ggb::CPU::setBus(BUS* bus)
{
	m_bus = bus;

	m_requestedInterrupts = m_bus->getPointerIntoMemory(INTERRUPT_REQUEST_ADDRESS);
	m_enabledInterrupts = m_bus->getPointerIntoMemory(ENABLED_INTERRUPT_ADDRESS);
}

bool ggb::CPU::handleInterrupts()
{
	if (m_cpuState.isStopped() && (*m_requestedInterrupts & *m_enabledInterrupts)) 
		m_cpuState.resume();

	if (!m_cpuState.interruptsEnabled())
		return false;

	if (handleInterrupt(INTERRUPT_VBLANK_BIT, VBLANK_INTERRUPT_ADDRESS, "VBLANK INTERRUPT"))
		return true;
	if (handleInterrupt(INTERRUPT_LCD_STAT_BIT, LCD_STAT_INTERRUPT_ADDRESS, "LCD STAT INTERRUPT"))
		return true;
	if (handleInterrupt(INTERRUPT_TIMER_BIT, TIMER_INTERRUPT_ADDRESS, "TIMER INTERRUPT"))
		return true;
	if (handleInterrupt(INTERRUPT_SERIAL_BIT, SERIAL_INTERRUPT_ADDRESS, "SERIAL INTERRUPT"))
		return true;
	if (handleInterrupt(INTERRUPT_JOYPAD_BIT, JOYPAD_INTERRUPT_ADDRESS, "JOYPAD INTERRUPT"))
		return true;
	return false;
}

bool ggb::CPU::handleInterrupt(int interruptBit, uint16_t interruptHandlerAddress, const char* interruptString)
{
	if (!isBitSet(*m_requestedInterrupts, interruptBit) || !isBitSet(*m_enabledInterrupts, interruptBit))
		return false;

	m_cpuState.disableInterrupts();
	clearBit(*m_requestedInterrupts, interruptBit);
	callAddress(&m_cpuState, m_bus, interruptHandlerAddress);
	debugLog(interruptString);
	return true;
}

int ggb::CPU::step()
{
	if (handleInterrupts())
		return 20; // 5 Machine cycles

	if (m_cpuState.isStopped())
		return 4; // For now we just say 4 clocks have gone by (one machine cycle)

	const int instructionPointer = m_cpuState.InstructionPointer();
	auto opCode = m_bus->read(instructionPointer);
	debugLog(m_opcodes.getMnemonic(opCode));
	++m_cpuState.InstructionPointer();
	const int duration = m_opcodes.execute(opCode, &m_cpuState, m_bus);

	auto serial = m_bus->read(0xff02);
	if (serial == 0x81) 
	{
		char c = m_bus->read(0xff01);
		printf("%c", c);
		m_bus->write(0xFF02, uint8_t(0x0));
	}

	return duration;
}

void ggb::CPU::serialization(Serialization* serialization)
{
	m_cpuState.serialization(serialization);
}
