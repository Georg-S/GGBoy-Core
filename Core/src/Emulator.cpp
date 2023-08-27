#include "Emulator.hpp"
#include "Logging.hpp"
#include "Constants.hpp"

using namespace ggb;

ggb::Emulator::Emulator()
{
    m_CPU = CPU();
    m_bus = std::make_unique<BUS>();
    m_ppu = std::make_unique<PixelProcessingUnit>(m_bus.get());
    m_timer = std::make_unique<Timer>(m_bus.get());
}

bool ggb::Emulator::loadCartridge(const std::filesystem::path& path)
{
    m_currentCartridge = ggb::loadCartridge(path);
    if (!m_currentCartridge) 
    {
        logError("Was not able to read ROM!");
        return false;
    }

    m_ppu = std::make_unique<PixelProcessingUnit>(m_bus.get()); // TODO make a reset function instead of just recreating it
    m_bus->setCartridge(m_currentCartridge.get());
    m_bus->setTimer(m_timer.get());
    m_bus->setPixelProcessingUnit(m_ppu.get());
    m_CPU.setBus(m_bus.get());
    m_CPU.reset();
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
    m_timer->step(cycles);
    
}

void ggb::Emulator::setTileDataRenderer(std::unique_ptr<ggb::Renderer> renderer)
{
    m_ppu->setTileDataRenderer(std::move(renderer));
    m_ppu->setDrawTileData(true);
}

void ggb::Emulator::setGameRenderer(std::unique_ptr<ggb::Renderer> renderer)
{
    m_ppu->setGameRenderer(std::move(renderer));
}

Dimensions ggb::Emulator::getTileDataDimensions() const
{
    return m_ppu->getTileDataDimensions();
}

Dimensions ggb::Emulator::getGameWindowDimensions() const
{
    return Dimensions{GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT};
}
