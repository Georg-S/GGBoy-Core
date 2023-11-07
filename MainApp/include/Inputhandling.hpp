#pragma once
#include <SDL.h>
#include <Input.hpp>
#include <Emulator.hpp>


class InputHandler : public ggb::Input
{
public:
	InputHandler();
	~InputHandler();
	bool isAPressed() override;
	bool isBPressed() override;
	bool isStartPressed() override;
	bool isSelectPressed() override;
	bool isUpPressed() override;
	bool isDownPressed() override;
	bool isLeftPressed() override;
	bool isRightPressed() override;
	void update(long long nanoSecondsPassed);

private:
	bool controllerButtonPressed(SDL_GameControllerButton button);
	bool m_aButtonDown = false;
	bool m_bButtonDown = false;
	bool m_startButtonDown = false;
	bool m_selectButtonDown = false;
	bool m_upButtonDown = false;
	bool m_downButtonDown = false;
	bool m_leftButtonDown = false;
	bool m_rightButtonDown = false;

	const Uint8* m_keyStates = SDL_GetKeyboardState(nullptr);
	SDL_GameController* m_controller = nullptr;
};