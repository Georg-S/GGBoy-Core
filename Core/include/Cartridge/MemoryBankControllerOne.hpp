#pragma once

#include "MemoryBankController.hpp"

namespace ggb
{
	class MemoryBankControllerOne : public MemoryBankController
	{
	public:
		MemoryBankControllerOne(std::vector<uint8_t>&& cartridgeData);
		void write(uint16_t address, uint8_t value) override;
		uint8_t read(uint16_t address) const override;
		void executeOAMDMATransfer(uint16_t startAddress, uint8_t* oam) const override;
	private:
		bool m_ramEnabled = false;
		int m_romBankNumber = 0;
	};
}