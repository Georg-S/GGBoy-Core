#include "Emulator.hpp"
#include "Logging.hpp"

using namespace ggb;

ggb::Emulator::Emulator()
{
    m_CPU = CPU();
    m_bus = std::make_unique<BUS>();
    m_ppu = std::make_unique<PixelProcessingUnit>(m_bus.get());
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
    m_ppu = std::make_unique<PixelProcessingUnit>(m_bus.get()); // TODO make a reset function instead of just recreating it
    return true;
}

void ggb::Emulator::run()
{
    while (true)
        step();
}

void ggb::Emulator::step()
{
    int cycles = m_CPU.step();
    m_ppu->step(cycles);
}

void ggb::Emulator::setTileDataRenderer(std::unique_ptr<ggb::Renderer> renderer)
{
    m_ppu->setTileDataRenderer(std::move(renderer));
    m_ppu->setDrawTileData(true);
}

Dimensions ggb::Emulator::getTileDataDimensions() const
{
    return m_ppu->getTileDataDimensions();
}
