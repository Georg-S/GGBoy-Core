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
	// TODO close SDL gamecontroller
}

bool InputHandler::isAPressed()
{
	return m_aButtonDown;
}

bool InputHandler::isBPressed() 
{
	return m_bButtonDown;
}

bool InputHandler::isStartPressed() 
{
	return m_startButtonDown;
}

bool InputHandler::isSelectPressed() 
{
	return m_selectButtonDown;
}

bool InputHandler::isUpPressed() 
{
	return m_upButtonDown;
}

bool InputHandler::isDownPressed() 
{
	return m_downButtonDown;
}

bool InputHandler::isLeftPressed() 
{
	return m_leftButtonDown;
}

bool InputHandler::isRightPressed() 
{
	return m_rightButtonDown;
}

void InputHandler::update(long long nanoSecondsPassed)
{
	m_aButtonDown = m_keyStates[SDL_SCANCODE_O] || controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A);
	m_bButtonDown = m_keyStates[SDL_SCANCODE_P] || controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B);
	m_startButtonDown = m_keyStates[SDL_SCANCODE_SPACE] || controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START);
	m_selectButtonDown = m_keyStates[SDL_SCANCODE_RETURN] || controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_BACK);
	m_selectButtonDown = m_keyStates[SDL_SCANCODE_RETURN] || controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_BACK);
	m_upButtonDown = m_keyStates[SDL_SCANCODE_W] || controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP);
	m_downButtonDown = m_keyStates[SDL_SCANCODE_S] || controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN);
	m_leftButtonDown = m_keyStates[SDL_SCANCODE_A] || controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT);
	m_rightButtonDown = m_keyStates[SDL_SCANCODE_D] || controllerButtonPressed(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
}

bool InputHandler::controllerButtonPressed(SDL_GameControllerButton button)
{
	if (!m_controller)
		return false;
	return SDL_GameControllerGetButton(m_controller, button);
}
