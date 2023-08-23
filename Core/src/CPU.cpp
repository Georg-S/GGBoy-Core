#include "CPU.hpp"

#include "CPUInstructions.hpp"
#include "Logging.hpp"

static constexpr uint16_t VBLANK_INTERRUPT_ADDRESS = 0x40;
static constexpr uint16_t LCD_STAT_INTERRUPT_ADDRESS = 0x48;
static constexpr uint16_t TIMER_INTERRUPT_ADDRESS = 0x50;
static constexpr uint16_t SERIAL_INTERRUPT_ADDRESS = 0x58;
static constexpr uint16_t JOYPAD_INTERRUPT_ADDRESS = 0x60;

static constexpr int VBLANK_BIT = 0;
static constexpr int LCD_STAT_BIT = 1;
static constexpr int TIMER_BIT = 2;
static constexpr int SERIAL_BIT = 3;
static constexpr int JOYPAD_BIT = 4;

static void debugLog(const std::string& message) 
{
	bool debugLogEnabled = true;

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
	// TODO maybe make a single lambda for the interrupts, however for now (debugging purposes) we leave it how it is
	if (m_bus->checkBit(interruptRequestAddress, VBLANK_BIT) && m_bus->checkBit(enabledInterruptAddress, VBLANK_BIT))
	{
		m_cpuState.disableInterrupts();
		m_bus->resetBit(interruptRequestAddress, VBLANK_BIT);
		callAddress(&m_cpuState, m_bus, VBLANK_INTERRUPT_ADDRESS);
		debugLog("VBLANK INTERRUPT");
		return true;
	}

	if (m_bus->checkBit(interruptRequestAddress, LCD_STAT_BIT) && m_bus->checkBit(enabledInterruptAddress, LCD_STAT_BIT))
	{
		m_cpuState.disableInterrupts();
		m_bus->resetBit(interruptRequestAddress, LCD_STAT_BIT);
		callAddress(&m_cpuState, m_bus, LCD_STAT_INTERRUPT_ADDRESS);
		debugLog("LCD STAT INTERRUPT");
		return true;
	}

	if (m_bus->checkBit(interruptRequestAddress, TIMER_BIT) && m_bus->checkBit(enabledInterruptAddress, TIMER_BIT))
	{
		m_cpuState.disableInterrupts();
		m_bus->resetBit(interruptRequestAddress, TIMER_BIT);
		callAddress(&m_cpuState, m_bus, TIMER_INTERRUPT_ADDRESS);
		debugLog("TIMER INTERRUPT");
		return true;
	}

	if (m_bus->checkBit(interruptRequestAddress, SERIAL_BIT) && m_bus->checkBit(enabledInterruptAddress, SERIAL_BIT))
	{
		m_cpuState.disableInterrupts();
		m_bus->resetBit(interruptRequestAddress, SERIAL_BIT);
		callAddress(&m_cpuState, m_bus, SERIAL_INTERRUPT_ADDRESS);
		debugLog("SERIAL INTERRUPT");
		return true;
	}

	if (m_bus->checkBit(interruptRequestAddress, JOYPAD_BIT) && m_bus->checkBit(enabledInterruptAddress, JOYPAD_BIT))
	{
		m_cpuState.disableInterrupts();
		m_bus->resetBit(interruptRequestAddress, JOYPAD_BIT);
		callAddress(&m_cpuState, m_bus, JOYPAD_INTERRUPT_ADDRESS);
		debugLog("JOYPAD INTERRUPT");
		return true;
	}

	return false;
}

int ggb::CPU::step()
{
	if (m_cpuState.interruptsEnabled()) 
	{
		if (handleInterrupts())
			return 5;
	}

	const int instructionPointer = m_cpuState.InstructionPointer();
	auto opCode = m_bus->read(instructionPointer);
	//debugLog(m_opcodes.getMnemonic(opCode) + "\t \t" + std::to_string(m_cpuState.B()));
	++m_cpuState.InstructionPointer();
	int duration = m_opcodes.execute(opCode, &m_cpuState, m_bus);
	++m_instructionCounter;

	auto test = m_bus->read(0xff02);
	if (test == 0x81) 
	{
		char c = m_bus->read(0xff01);
		printf("%c", c);
		m_bus->write(0xFF02, uint8_t(0x0));

	}

	if (false)
		m_bus->printVRAM();

	return duration;
}