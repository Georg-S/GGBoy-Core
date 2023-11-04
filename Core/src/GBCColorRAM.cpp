#include "GBCColorRAM.hpp"

#include "Utility.hpp"

ggb::GBCColorRAM::GBCColorRAM(uint16_t specificationAddress)
	: m_specificationAddress(specificationAddress)
{
}

void ggb::GBCColorRAM::reset()
{
	m_colorRAM = {};
}

void ggb::GBCColorRAM::setBus(BUS* bus)
{
	m_paletteSpecification = bus->getPointerIntoMemory(m_specificationAddress);
}

void ggb::GBCColorRAM::write(uint8_t value)
{
	m_colorRAM[getRAMAddress()] = value;
	if (isBitSet(*m_paletteSpecification, 7))
		incrementAddress();
}

uint8_t ggb::GBCColorRAM::read() const
{
	return m_colorRAM[getRAMAddress()];
}

void ggb::GBCColorRAM::serialization(Serialization* serialization)
{
	serialization->read_write(m_colorRAM);
}

size_t ggb::GBCColorRAM::getRAMAddress() const
{
	return *m_paletteSpecification & 0b111111;
}

void ggb::GBCColorRAM::incrementAddress()
{
	size_t address = getRAMAddress() + 1;
	address %= GBC_COLOR_RAM_MEMORY_SIZE;
	*m_paletteSpecification = *m_paletteSpecification & ~0b111111;
	*m_paletteSpecification |= address;
}
