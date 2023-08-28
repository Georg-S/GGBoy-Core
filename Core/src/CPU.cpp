#include "CPU.hpp"

#include "CPUInstructions.hpp"
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
	m_cpuState.A() = 0x01;
	m_cpuState.B() = 0x00;
	m_cpuState.C() = 0x13;
	m_cpuState.D() = 0x00;
	m_cpuState.E() = 0xD8;
	m_cpuState.H() = 0x01;
	m_cpuState.L() = 0x4D;
	m_cpuState.InstructionPointer() = 0x100;
	m_cpuState.StackPointer() = 0xFFFE;

}

void ggb::CPU::setBus(BUS* bus)
{
	m_bus = bus;
}

bool ggb::CPU::handleInterrupts()
{
	if (m_cpuState.isStopped() && m_bus->read(INTERRUPT_REQUEST_ADDRESS) != 0)
		m_cpuState.resume(); // TODO: handle "halting bug" maybe

	if (!m_cpuState.interruptsEnabled())
		return false;

	// TODO maybe make a single lambda for the interrupts, however for now (debugging purposes) we leave it how it is
	if (m_bus->checkBit(INTERRUPT_REQUEST_ADDRESS, INTERRUPT_VBLANK_BIT) 
		&& m_bus->checkBit(ENABLED_INTERRUPT_ADDRESS, INTERRUPT_VBLANK_BIT))
	{
		m_cpuState.disableInterrupts();
		m_bus->resetBit(INTERRUPT_REQUEST_ADDRESS, INTERRUPT_VBLANK_BIT);
		callAddress(&m_cpuState, m_bus, VBLANK_INTERRUPT_ADDRESS);
		debugLog("VBLANK INTERRUPT");
		m_cpuState.resume();
		return true;
	}

	if (m_bus->checkBit(INTERRUPT_REQUEST_ADDRESS, INTERRUPT_LCD_STAT_BIT) 
		&& m_bus->checkBit(ENABLED_INTERRUPT_ADDRESS, INTERRUPT_LCD_STAT_BIT))
	{
		m_cpuState.disableInterrupts();
		m_bus->resetBit(INTERRUPT_REQUEST_ADDRESS, INTERRUPT_LCD_STAT_BIT);
		callAddress(&m_cpuState, m_bus, LCD_STAT_INTERRUPT_ADDRESS);
		debugLog("LCD STAT INTERRUPT");
		m_cpuState.resume();
		return true;
	}

	if (m_bus->checkBit(INTERRUPT_REQUEST_ADDRESS, INTERRUPT_TIMER_BIT) 
		&& m_bus->checkBit(ENABLED_INTERRUPT_ADDRESS, INTERRUPT_TIMER_BIT))
	{
		m_cpuState.disableInterrupts();
		m_bus->resetBit(INTERRUPT_REQUEST_ADDRESS, INTERRUPT_TIMER_BIT);
		callAddress(&m_cpuState, m_bus, TIMER_INTERRUPT_ADDRESS);
		debugLog("TIMER INTERRUPT");
		m_cpuState.resume();
		return true;
	}

	if (m_bus->checkBit(INTERRUPT_REQUEST_ADDRESS, INTERRUPT_SERIAL_BIT) 
		&& m_bus->checkBit(ENABLED_INTERRUPT_ADDRESS, INTERRUPT_SERIAL_BIT))
	{
		m_cpuState.disableInterrupts();
		m_bus->resetBit(INTERRUPT_REQUEST_ADDRESS, INTERRUPT_SERIAL_BIT);
		callAddress(&m_cpuState, m_bus, SERIAL_INTERRUPT_ADDRESS);
		debugLog("SERIAL INTERRUPT");
		m_cpuState.resume();
		return true;
	}

	if (m_bus->checkBit(INTERRUPT_REQUEST_ADDRESS, INTERRUPT_JOYPAD_BIT) 
		&& m_bus->checkBit(ENABLED_INTERRUPT_ADDRESS, INTERRUPT_JOYPAD_BIT))
	{
		m_cpuState.disableInterrupts();
		m_bus->resetBit(INTERRUPT_REQUEST_ADDRESS, INTERRUPT_JOYPAD_BIT);
		callAddress(&m_cpuState, m_bus, JOYPAD_INTERRUPT_ADDRESS);
		debugLog("JOYPAD INTERRUPT");
		m_cpuState.resume();
		return true;
	}

	return false;
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
	int duration = m_opcodes.execute(opCode, &m_cpuState, m_bus);
	++m_instructionCounter;

	auto serial = m_bus->read(0xff02);
	if (serial == 0x81) 
	{
		char c = m_bus->read(0xff01);
		printf("%c", c);
		m_bus->write(0xFF02, uint8_t(0x0));

	}

	return duration;
}