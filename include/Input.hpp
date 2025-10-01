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

		void serialization(Serialization* serialization) 
		{
			serialization->read_write(isAPressed);
			serialization->read_write(isBPressed);
			serialization->read_write(isStartPressed);
			serialization->read_write(isSelectPressed);
			serialization->read_write(isUpPressed);
			serialization->read_write(isDownPressed);
			serialization->read_write(isLeftPressed);
			serialization->read_write(isRightPressed);
		}
	};

	class Input
	{
	public:
		Input() = default;
		void setBus(BUS* bus);
		void update();
		void setButtonState(GameboyInput input);
		void serialization(Serialization* serialization);

	private:
		BUS* m_bus = nullptr;
		uint8_t* m_inputRegister = nullptr;
		GameboyInput m_currentState = {};
	};
}