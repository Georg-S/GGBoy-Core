#include "Cartridge.hpp"

#include "Logging.hpp"

#include <fstream>

using namespace ggb;

bool ggb::Cartridge::load(const std::filesystem::path& romPath)
{
    if (!std::filesystem::exists(romPath)) 
    {
        logError("File not found: " + romPath.string());
    }

    std::ifstream stream(romPath, std::ios::in | std::ios::binary);
    auto bufData = std::vector<char>(std::istreambuf_iterator<char>(stream), {});
    m_cartridgeData.clear();
    m_cartridgeData.reserve(bufData.size());
    for (auto& data : bufData)
        m_cartridgeData.emplace_back(static_cast<std::byte>(std::move(data)));

    return true;
}

uint8_t ggb::Cartridge::read(uint16_t address)
{
    return static_cast<uint8_t>(m_cartridgeData[address]);
}

std::unique_ptr<Cartridge> ggb::loadCartridge(const std::filesystem::path& path)
{
    auto res = std::make_unique<Cartridge>();

    if (res->load(path))
        return res;

    return nullptr;
}
