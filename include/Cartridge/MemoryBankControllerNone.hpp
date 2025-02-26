#pragma once

#include "MemoryBankController.hpp"

namespace ggb
{
	class MemoryBankControllerNone : public MemoryBankController
	{
	public:
		void write(uint16_t address, uint8_t value) override;
		uint8_t read(uint16_t address) const override;
	};
}