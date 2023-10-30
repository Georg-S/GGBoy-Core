#pragma once

#include "MemoryBankController.hpp"

namespace ggb
{
	class MemoryBankControllerFive : public MemoryBankController
	{
	public:
		void write(uint16_t address, uint8_t value) override;
		uint8_t read(uint16_t address) const override;
		void executeOAMDMATransfer(uint16_t startAddress, uint8_t* oam) const override;
		virtual void serialization(Serialization* serialization) override;

	private:
		bool m_ramEnabled = false;
		int m_romBankNumber = 0;
	};
}