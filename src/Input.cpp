#include "Input.hpp"

#include "Utility.hpp"
#include "Constants.hpp"
#include "Logging.hpp"

static constexpr int ACTION_BUTTONS_BIT = ggb::BIT5;
static constexpr int DIRECTION_BUTTONS_BIT = ggb::BIT4;
static constexpr int DOWN_START_BIT = ggb::BIT3;
static constexpr int UP_SELECT_BIT = ggb::BIT2;
static constexpr int LEFT_B_BIT = ggb::BIT1;
static constexpr int RIGHT_A_BIT = ggb::BIT0;

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

		if ((actionSelectedAndPressed || directionSelectedAndPressed) && isBitSet(*m_inputRegister, bit))
			m_bus->requestInterrupt(INTERRUPT_JOYPAD_BIT);

		if (actionSelectedAndPressed || directionSelectedAndPressed)
			clearBit(*m_inputRegister, bit);
		else
			setBit(*m_inputRegister, bit);
	};

	const auto& buttons = m_currentState;
	setInputBitAndHandleInterrupt(buttons.isAPressed, buttons.isRightPressed, RIGHT_A_BIT);
	setInputBitAndHandleInterrupt(buttons.isBPressed, buttons.isLeftPressed, LEFT_B_BIT);
	setInputBitAndHandleInterrupt(buttons.isSelectPressed, buttons.isUpPressed, UP_SELECT_BIT);
	setInputBitAndHandleInterrupt(buttons.isStartPressed, buttons.isDownPressed, DOWN_START_BIT);
}

void ggb::Input::setButtonState(GameboyInput input)
{
	m_currentState = std::move(input);
}
