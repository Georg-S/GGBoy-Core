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
		void serialization(Serialization* serialization);
		void updateAfterWrite();

	private:
		void updateTimerDivider(int elapsedCycles);

		bool m_enabled = false;
		uint32_t m_timerControlValue = 0;
		uint16_t m_dividerCounter = 0;
		uint32_t m_counterForTimerCounter = 0;
		BUS* m_bus;
		uint8_t* m_dividerRegister = nullptr;
		uint8_t* m_timerCounter = nullptr;
		uint8_t* m_timerModulo = nullptr;
		uint8_t* m_timerControl = nullptr;
	};
}