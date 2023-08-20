#include "Emulator.hpp"
#include "Logging.hpp"

using namespace ggb;

ggb::Emulator::Emulator()
{
    m_CPU = CPU();
    m_bus = std::make_unique<BUS>();
}

bool ggb::Emulator::loadCartridge(const std::filesystem::path& path)
{
    m_currentCartridge = ggb::loadCartridge(path);
    if (!m_currentCartridge) 
    {
        logError("Was not able to read ROM!");
        return false;
    }

    m_bus->setCartridge(m_currentCartridge.get());
    m_CPU.setBus(m_bus.get());
    m_CPU.reset();
    return true;
}

void ggb::Emulator::run()
{
    while (true) 
    {
        m_CPU.step();
    }
}
