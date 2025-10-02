#include "Timer.hpp"

#include <cassert>

#include "Constants.hpp"
#include "Utility.hpp"

static uint32_t getTimerControlDivisor(uint8_t num) 
{
	assert(num < 4);
	if (num == 0b00)	return 1024;
	if (num == 0b01)	return 16;
	if (num == 0b10)	return 64;
	if (num == 0b11)	return 256;
	return 0;
}

ggb::Timer::Timer(BUS* bus)
{
	setBus(bus);
	updateAfterWrite();
}

void ggb::Timer::reset()
{
	m_dividerCounter = 0;
	m_counterForTimerCounter = 0;
}

void ggb::Timer::setBus(BUS* bus)
{
	m_bus = bus;
	m_dividerRegister = m_bus->getPointerIntoMemory(TIMER_DIVIDER_REGISTER_ADDRESS);
	m_timerCounter = m_bus->getPointerIntoMemory(TIMER_COUNTER_ADDRESS);
	m_timerModulo = m_bus->getPointerIntoMemory(TIMER_MODULO_ADDRESS);
	m_timerControl = m_bus->getPointerIntoMemory(TIMER_CONTROL_ADDRESS);
}

void ggb::Timer::step(int elapsedCycles)
{
	updateTimerDivider(elapsedCycles);

	if (!m_enabled)
		return;

	m_counterForTimerCounter += elapsedCycles;
	while (m_counterForTimerCounter >= m_timerControlValue)
	{
		m_counterForTimerCounter -= m_timerControlValue;
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

void ggb::Timer::serialization(Serialization* serialization)
{
	serialization->read_write(m_dividerCounter);
	serialization->read_write(m_counterForTimerCounter);
	serialization->read_write(m_enabled);
	serialization->read_write(m_timerControlValue);
}

void ggb::Timer::updateAfterWrite()
{
	m_enabled = isBitSet<2>(*m_timerControl);

	const auto timerControlDividerBits = *m_timerControl & 0b11;
	m_timerControlValue = getTimerControlDivisor(timerControlDividerBits);
}

inline void ggb::Timer::updateTimerDivider(int elapsedCycles)
{
	m_dividerCounter += elapsedCycles;
	if (m_dividerCounter >= TIMER_DIVIDER_REGISTER_INCREMENT_COUNT)
	{
		++(*m_dividerRegister);
		m_dividerCounter -= TIMER_DIVIDER_REGISTER_INCREMENT_COUNT;
	}
}
