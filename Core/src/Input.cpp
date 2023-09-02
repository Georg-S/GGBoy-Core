#include "Input.hpp"

#include "Utility.hpp"
#include "Constants.hpp"
#include "Logging.hpp"

static constexpr int ACTION_BUTTONS_BIT = 5;
static constexpr int DIRECTION_BUTTONS_BIT = 4;
static constexpr int DOWN_START_BIT = 3;
static constexpr int UP_SELECT_BIT = 2;
static constexpr int LEFT_B_BIT = 1;
static constexpr int RIGHT_A_BIT = 0;

void ggb::Input::setBus(BUS* bus)
{
	m_bus = bus;
	m_inputRegister = m_bus->getPointerIntoMemory(INPUT_REGISTER_ADDRESS);
}

void ggb::Input::update()
{
	const bool actionSelected = !isBitSet(*m_inputRegister, ACTION_BUTTONS_BIT);
	const bool directionSelected = !isBitSet(*m_inputRegister, DIRECTION_BUTTONS_BIT);

	auto setInputBitAndHandleInterrupt = [this, actionSelected, directionSelected](bool actionPressed, bool directionPressed, int bit) 
	{
		const bool actionSelectedAndPressed = actionSelected && actionPressed;
		const bool directionSelectedAndPressed = directionSelected && directionPressed;

		if (isBitSet(*m_inputRegister, bit) && (actionSelectedAndPressed || directionSelectedAndPressed))
			m_bus->requestInterrupt(INTERRUPT_JOYPAD_BIT);

		if (actionSelectedAndPressed || directionSelectedAndPressed)
			clearBit(*m_inputRegister, bit);
		else
			setBit(*m_inputRegister, bit);
	};

	setInputBitAndHandleInterrupt(isAPressed(), isRightPressed(), RIGHT_A_BIT);
	setInputBitAndHandleInterrupt(isBPressed(), isLeftPressed(), LEFT_B_BIT);
	setInputBitAndHandleInterrupt(isSelectPressed(), isUpPressed(), UP_SELECT_BIT);
	setInputBitAndHandleInterrupt(isStartPressed(), isDownPressed(), DOWN_START_BIT);
}


