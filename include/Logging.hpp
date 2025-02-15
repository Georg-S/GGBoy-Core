#pragma once
#include <string_view>

namespace ggb
{
	void logWarning(std::string_view message);
	void logError(std::string_view message);
	void logInfo(std::string_view message);
	void logNumBinary(uint8_t num);
	void logNumBinary(uint16_t num);
}