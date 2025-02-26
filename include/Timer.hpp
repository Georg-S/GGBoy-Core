#pragma once
#include "BUS.hpp"
#include "Serialization.hpp"

namespace ggb
{
	class Timer
	{
	public:
		Timer(BUS* bus);
		void reset();
		void setBus(BUS* bus);
		void step(int elapsedCycles);
		void resetDividerRegister();
		void updateTimerDivider(int elapsedCycles);
		void serialization(Serialization* serialization);

	private:
		BUS* m_bus;
		uint8_t* m_dividerRegister = nullptr;
		uint8_t* m_timerCounter = nullptr;
		uint8_t* m_timerModulo = nullptr;
		uint8_t* m_timerControl = nullptr;
		uint16_t m_dividerCounter = 0;
		uint32_t m_counterForTimerCounter = 0;
	};
}