#pragma once
#include <functional>
#include "BUS.hpp"

namespace ggb
{
	struct GameboyInput 
	{
		bool isAPressed = false;
		bool isBPressed = false;
		bool isStartPressed = false;
		bool isSelectPressed = false;
		bool isUpPressed = false;
		bool isDownPressed = false;
		bool isLeftPressed = false;
		bool isRightPressed = false;
	};

	class Input
	{
	public:
		Input() = default;
		void setBus(BUS* bus);
		void update();
		void setButtonState(GameboyInput input);

	private:
		BUS* m_bus = nullptr;
		uint8_t* m_inputRegister = nullptr;
		GameboyInput m_currentState = {};
	};
}