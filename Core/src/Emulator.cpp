#include "Emulator.hpp"
#include "Logging.hpp"
#include "Constants.hpp"

using namespace ggb;

ggb::Emulator::Emulator()
{
    m_cpu = std::make_unique<CPU>();
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
    reset();

    return true;
}

void ggb::Emulator::run()
{
    while (true)
        step();
}

void ggb::Emulator::step()
{
    int cycles = m_cpu->step();
    m_ppu->step(cycles);
    m_timer->step(cycles);
    m_input->update();
}

void ggb::Emulator::reset()
{
    m_bus->reset();
    m_cpu->reset();
    m_ppu->reset();
    m_timer->reset();
    rewire();
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

void ggb::Emulator::setInput(std::unique_ptr<Input> input)
{
    m_input = std::move(input);
    m_input->setBus(m_bus.get());
}

Dimensions ggb::Emulator::getTileDataDimensions() const
{
    return m_ppu->getTileDataDimensions();
}

Dimensions ggb::Emulator::getGameWindowDimensions() const
{
    return Dimensions{GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT};
}

void ggb::Emulator::rewire()
{
    m_bus->setCartridge(m_currentCartridge.get());
    m_bus->setTimer(m_timer.get());
    m_bus->setPixelProcessingUnit(m_ppu.get());
    m_ppu->setBus(m_bus.get());
    m_cpu->setBus(m_bus.get());
    m_timer->setBus(m_bus.get());
    if (m_input)
        m_input->setBus(m_bus.get());
}
