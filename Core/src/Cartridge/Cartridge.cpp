#include "Cartridge/Cartridge.hpp"

#include <fstream>
#include <cassert>

#include "Logging.hpp"
#include "Constants.hpp"
#include "Cartridge/MemoryBankControllerNone.hpp"
#include "Cartridge/MemoryBankControllerOne.hpp"
#include "Cartridge/MemoryBankControllerFive.hpp"

using namespace ggb;

bool ggb::Cartridge::load(const std::filesystem::path& romPath)
{
	if (!std::filesystem::exists(romPath))
	{
		logError("File not found: " + romPath.string());
	}

	std::ifstream stream(romPath, std::ios::in | std::ios::binary);
	auto bufData = std::vector<char>(std::istreambuf_iterator<char>(stream), {});
	std::vector<uint8_t> cartridgeData;
	cartridgeData.reserve(bufData.size());
	for (auto& data : bufData)
		cartridgeData.emplace_back(static_cast<uint8_t>(std::move(data)));

	m_mbcType = getMBCType(cartridgeData);
	m_memoryBankController = createMemoryBankController(m_mbcType);
	m_memoryBankController->initialize(std::move(cartridgeData));

	return true;
}

void ggb::Cartridge::write(uint16_t address, uint8_t value)
{
	m_memoryBankController->write(address, value);
}

void ggb::Cartridge::executeDMATransfer(uint16_t startAddress, uint8_t* out, size_t sizeInBytes)
{
	m_memoryBankController->executeDMATransfer(startAddress, out, sizeInBytes);
}

uint8_t ggb::Cartridge::read(uint16_t address) const
{
	return m_memoryBankController->read(address);
}

void ggb::Cartridge::serialize(Serialization* serialize)
{
	serialization(serialize);
	m_memoryBankController->serialization(serialize);
}

void ggb::Cartridge::deserialize(Serialization* deserialize)
{
	serialization(deserialize);
	m_memoryBankController = createMemoryBankController(m_mbcType);
	m_memoryBankController->serialization(deserialize);
}

void ggb::Cartridge::saveRAM(const std::filesystem::path& outputPath)
{
	m_memoryBankController->saveRAM(outputPath);
}

void ggb::Cartridge::loadRAM(const std::filesystem::path& inputPath)
{
	m_memoryBankController->loadRAM(inputPath);
}

void ggb::Cartridge::serialization(Serialization* serialize)
{
	serialize->read_write(m_mbcType);
}

std::unique_ptr<MemoryBankController> ggb::Cartridge::createMemoryBankController(MBCTYPE mbcType) const
{
	switch (mbcType)
	{
	case ggb::NO_MBC:
		return std::make_unique<MemoryBankControllerNone>();
	case ggb::MBC1:
		return std::make_unique<MemoryBankControllerOne>();
	case ggb::MBC1_RAM:
		return std::make_unique<MemoryBankControllerOne>();
		break;
	case ggb::MBC1_RAM_BATTERY:
		return std::make_unique<MemoryBankControllerOne>();
	case ggb::MC5_RAM_BATTERY:
		return std::make_unique<MemoryBankControllerFive>();
	default:
		break;
	}
	assert(!"Not implemented");
	return nullptr;
}

std::unique_ptr<Cartridge> ggb::loadCartridge(const std::filesystem::path& path)
{
	auto res = std::make_unique<Cartridge>();

	if (res->load(path))
		return res;

	return nullptr;
}
