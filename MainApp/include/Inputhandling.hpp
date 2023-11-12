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
	void updateControllerInput();
	void updateKeyboardInput();
	bool controllerButtonPressed(SDL_GameControllerButton button);
	struct InputState
	{
		bool aButtonDown = false;
		bool bButtonDown = false;
		bool startButtonDown = false;
		bool selectButtonDown = false;
		bool upButtonDown = false;
		bool downButtonDown = false;
		bool leftButtonDown = false;
		bool rightButtonDown = false;
	};

	InputState m_inputState = {};
	const Uint8* m_keyStates = SDL_GetKeyboardState(nullptr);
	SDL_GameController* m_controller = nullptr;
};