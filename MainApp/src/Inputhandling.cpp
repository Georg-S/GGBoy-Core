#include "Inputhandling.hpp"

#include <iostream>

InputHandler::InputHandler()
{
	if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0)
	{
		fprintf(stderr, "Error initializing controller SDL_Error: %s\n", SDL_GetError());
		return;
	}

	for (int i = 0; i < SDL_NumJoysticks(); i++)
	{
		if (SDL_IsGameController(i))
		{
			m_controller = SDL_GameControllerOpen(i);
			return;
		}
	}
}

InputHandler::~InputHandler()
{
	SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

bool InputHandler::isAPressed()
{
	return m_inputState.aButtonDown;
}

bool InputHandler::isBPressed()
{
	return m_inputState.bButtonDown;
}

bool InputHandler::isStartPressed()
{
	return m_inputState.startButtonDown;
}

bool InputHandler::isSelectPressed()
{
	return m_inputState.selectButtonDown;
}

bool InputHandler::isUpPressed()
{
	return m_inputState.upButtonDown;
}

bool InputHandler::isDownPressed()
{
	return m_inputState.downButtonDown;
}

bool InputHandler::isLeftPressed()
{
	return m_inputState.leftButtonDown;
}

bool InputHandler::isRightPressed()
{
	return m_inputState.rightButtonDown;
}

void InputHandler::update(long long nanoSecondsPassed)
{
	m_inputState = {};
	updateKeyboardInput();
	updateControllerInput();
}

void InputHandler::updateControllerInput()
{
	constexpr int joyStickThreshold = 20000;
	auto xValue = SDL_GameControllerGetAxis(m_controller, SDL_CONTROLLER_AXIS_LEFTX);
	auto yValue = SDL_GameControllerGetAxis(m_controller, SDL_CONTROLLER_AXIS_LEFTY);
	const bool joyStickLeft = xValue < -joyStickThreshold;
	const bool joyStickRight = xValue > joyStickThreshold;
	const bool joyStickDown = yValue > joyStickThreshold;
	const bool joyStickUp = yValue < -joyStickThreshold;

	m_inputState.aButtonDown |= controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A);
	m_inputState.bButtonDown |= controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B);
	m_inputState.startButtonDown |= controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START);
	m_inputState.selectButtonDown |= controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_BACK);
	m_inputState.selectButtonDown |= controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_BACK);
	m_inputState.upButtonDown |= controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP) || joyStickUp;
	m_inputState.downButtonDown |= controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN) || joyStickDown;
	m_inputState.leftButtonDown |= controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT) || joyStickLeft;
	m_inputState.rightButtonDown |= controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT) || joyStickRight;
}

void InputHandler::updateKeyboardInput()
{
	m_inputState.aButtonDown |= static_cast<bool>(m_keyStates[SDL_SCANCODE_O]);
	m_inputState.bButtonDown |= static_cast<bool>(m_keyStates[SDL_SCANCODE_P]);
	m_inputState.startButtonDown |= static_cast<bool>(m_keyStates[SDL_SCANCODE_SPACE]);
	m_inputState.selectButtonDown |= static_cast<bool>(m_keyStates[SDL_SCANCODE_RETURN]);
	m_inputState.selectButtonDown |= static_cast<bool>(m_keyStates[SDL_SCANCODE_RETURN]);
	m_inputState.upButtonDown |= static_cast<bool>(m_keyStates[SDL_SCANCODE_W]);
	m_inputState.downButtonDown |= static_cast<bool>(m_keyStates[SDL_SCANCODE_S]);
	m_inputState.leftButtonDown |= static_cast<bool>(m_keyStates[SDL_SCANCODE_A]);
	m_inputState.rightButtonDown |= static_cast<bool>(m_keyStates[SDL_SCANCODE_D]);
}

bool InputHandler::controllerButtonPressed(SDL_GameControllerButton button)
{
	if (!m_controller)
		return false;
	return SDL_GameControllerGetButton(m_controller, button);
}
