#pragma once

#include "MemoryBankController.hpp"

namespace ggb
{
	class MemoryBankControllerNone : public MemoryBankController
	{
	public:
		MemoryBankControllerNone(std::vector<uint8_t>&& cartridgeData);
		void write(uint16_t address, uint8_t value) override;
		uint8_t read(uint16_t address) const override;
		void executeOAMDMATransfer(uint16_t startAddress, uint8_t* oam) const override;
	};
}