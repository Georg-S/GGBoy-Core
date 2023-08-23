#pragma once
#include <string>

namespace ggb
{
	void logWarning(const std::string& message);
	void logError(const std::string& message);
	void logInfo(const std::string& message);
	void logNumBinary(uint8_t num);
	void logNumBinary(uint16_t num);
}