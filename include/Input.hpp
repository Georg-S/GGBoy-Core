#pragma once
#include <functional>
#include "BUS.hpp"

namespace ggb
{
	class Input
	{
	public:
		Input() = default;
		virtual ~Input() = default;
		virtual void setBus(BUS* bus) final;
		virtual void update() final;
		virtual bool isAPressed() = 0;
		virtual bool isBPressed() = 0;
		virtual bool isStartPressed() = 0;
		virtual bool isSelectPressed() = 0;
		virtual bool isUpPressed() = 0;
		virtual bool isDownPressed() = 0;
		virtual bool isLeftPressed() = 0;
		virtual bool isRightPressed() = 0;

	private:
		BUS* m_bus = nullptr;
		uint8_t* m_inputRegister = nullptr;
	};
}