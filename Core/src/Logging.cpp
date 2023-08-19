#include "Logging.hpp"

#include <iostream>

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
	std::cout << "INFO: " << message << std::endl;
}
