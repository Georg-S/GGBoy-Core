#include "Logging.hpp"

#include <iostream>
#include <bitset>

void ggb::logWarning(const std::string& message)
{
	std::cout << "WARNING: " << message << std::endl;
}

void ggb::logError(const std::string& message)
{
	std::cout << "ERROR: " << message << std::endl;
}

void ggb::logInfo(const std::string& message)
{
	std::cout << "INFO: " << message << "\n";
}

void ggb::logNumBinary(uint8_t num)
{
	std::bitset<8> abc(num);
	std::cout << abc << "\n";
}

void ggb::logNumBinary(uint16_t num)
{
	std::bitset<16> abc(num);
	std::cout << abc << "\n";
}
