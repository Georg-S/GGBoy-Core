#include "Timer.hpp"

#include <cassert>

#include "Constants.hpp"
#include "Utility.hpp"

static uint32_t getTimerControlDivisor(uint8_t num) 
{
	assert(num < 4);
	if (num == 0)		return 1024;
	if (num == 0b01)	return 16;
	if (num == 0b10)	return 64;
	if (num == 0b11)	return 256;
	return 0;
}

ggb::Timer::Timer(BUS* bus)
	: m_bus(bus)
{
	m_dividerRegister = m_bus->getPointerIntoMemory(TIMER_DIVIDER_REGISTER_ADDRESS);
	m_timerCounter = m_bus->getPointerIntoMemory(TIMER_COUNTER_ADDRESS);
	m_timerModulo = m_bus->getPointerIntoMemory(TIMER_MODULO_ADDRESS);
	m_timerControl = m_bus->getPointerIntoMemory(TIMER_CONTROL_ADDRESS);
}

void ggb::Timer::step(int elapsedCycles)
{
	constexpr uint32_t cpu_clock = 4194304;
	updateTimerDivider(elapsedCycles);

	const bool enabled = isBitSet(*m_timerControl, 2);
	if (!enabled)
		return;

	const auto timerControlDividerBits = *m_timerControl & 0b11;
	const auto divisor = getTimerControlDivisor(timerControlDividerBits);
	const auto incrementer = cpu_clock / divisor;
	// TODO double check, probably wrong
	m_counterForTimerCounter += elapsedCycles;
	if (m_counterForTimerCounter >= incrementer)
	{
		m_counterForTimerCounter %= incrementer;
		++(*m_timerCounter);
		if (*m_timerCounter == 0) 
		{
			*m_timerCounter = *m_timerModulo;
			m_bus->requestInterrupt(INTERRUPT_TIMER_BIT);
		}
	}
}

void ggb::Timer::resetDividerRegister()
{
	*m_dividerRegister = 0x00;
}

void ggb::Timer::updateTimerDivider(int elapsedCycles)
{
	// TODO double check
	m_dividerCounter += elapsedCycles;
	if (m_dividerCounter >= TIMER_DIVIDER_REGISTER_INCREMENT_COUNT)
	{
		++(*m_dividerRegister);
		m_dividerCounter -= TIMER_DIVIDER_REGISTER_INCREMENT_COUNT;
	}
}
