#include "Cartridge/MemoryBankControllerThree.hpp"

#include <cassert>

const uint8_t& ggb::MemoryBankControllerThree::RealTimeClock::getRegister(Register selectedRegister) const
{
	if (selectedRegister == Register::SECONDS)
		return m_seconds;
	if (selectedRegister == Register::MINUTES)
		return m_minutes;
	if (selectedRegister == Register::HOURS)
		return m_hours;
	if (selectedRegister == Register::DAYSLOWER)
		return m_daysLower;
	if (selectedRegister == Register::DAYSUPPERFLAGS)
		return m_daysUpperAndFlags;
	assert(!"Invalid Register");
	return m_daysUpperAndFlags;
}

uint8_t& ggb::MemoryBankControllerThree::RealTimeClock::getRegister(Register selectedRegister)
{
	return const_cast<uint8_t&>(const_cast<const MemoryBankControllerThree::RealTimeClock*>(this)->getRegister(selectedRegister));
}

ggb::MemoryBankControllerThree::RealTimeClock::Register ggb::MemoryBankControllerThree::RealTimeClock::getRegisterFromValue(uint8_t value)
{
	if (value == 0x08)
		return Register::SECONDS;
	if (value == 0x09)
		return Register::MINUTES;
	if (value == 0x0A)
		return Register::HOURS;
	if (value == 0x0B)
		return Register::DAYSLOWER;
	if (value == 0x0C)
		return Register::DAYSUPPERFLAGS;
	return Register::NONE;
}

void ggb::MemoryBankControllerThree::RealTimeClock::serialize(Serialization* serialization)
{
	serialization->read_write(m_seconds);
	serialization->read_write(m_minutes);
	serialization->read_write(m_hours);
	serialization->read_write(m_daysLower);
	serialization->read_write(m_daysUpperAndFlags);
	serialization->read_write(m_isLatched);
	serialization->read_write(m_lastLatchValue);
	serialization->read_write(m_selectedRegister);
}

void ggb::MemoryBankControllerThree::RealTimeClock::selectRegister(uint8_t value)
{
	m_selectedRegister = getRegisterFromValue(value);
}

void ggb::MemoryBankControllerThree::RealTimeClock::writeToSelectedRegister(uint8_t value)
{
	auto& reg = getRegister(m_selectedRegister);
	reg = value;
}

uint8_t ggb::MemoryBankControllerThree::RealTimeClock::getSelectedRegisterValue() const
{
	return getRegister(m_selectedRegister);
}

bool ggb::MemoryBankControllerThree::RealTimeClock::registerSelected() const
{
	return m_selectedRegister != Register::NONE;
}

void ggb::MemoryBankControllerThree::RealTimeClock::resetRegisterSelection()
{
	m_selectedRegister = Register::NONE;
}

void ggb::MemoryBankControllerThree::RealTimeClock::handleLatching(uint8_t value)
{
	if ((m_lastLatchValue == 0) && (value == 0x1))
		m_isLatched = !m_isLatched;
	m_lastLatchValue = value;
}

void ggb::MemoryBankControllerThree::write(uint16_t address, uint8_t value)
{
	if (isRAMOrTimerEnableAddress(address)) 
	{
		m_ramAndTimerEnabled = shouldEnableRAM(value);
		return;
	}

	if (isROMBankingAddress(address)) 
	{
		setROMBank(value);
		return;
	}

	if (isRAMBankingOrRTCRegisterSelectAddress(address)) 
	{
		if ((0x0 <= value) && (value <= 0x3)) 
		{
			m_rtc.resetRegisterSelection();
			m_ramBank = value;
			return;
		}
		
		m_rtc.selectRegister(value);
		return;
	}

	if (isLatchClockDataAddress(address)) 
	{
		m_rtc.handleLatching(value);
		return;
	}

	assert(isCartridgeRAMOrRTCRegister(address));
	if (m_rtc.registerSelected())
	{
		m_rtc.writeToSelectedRegister(value);
		return;
	}

	auto ramAddr = convertRawAddressToRAMBankAddress(address, m_ramBank);
	m_ram[ramAddr] = value;
}

uint8_t ggb::MemoryBankControllerThree::read(uint16_t address) const
{
	if (isFirstROMBankAddress(address))
		return m_cartridgeData[address];
	if (isROMBankAddress(address))
		return m_cartridgeData[convertRawAddressToBankAddress(address, m_romBank)];
	if (!m_ramAndTimerEnabled)
		return 0xFF;
	if (m_rtc.registerSelected()) 
	{
		// TODO handle RTC reading correctly,
		// time needs to increase maybe by using unix timestamps 
		// and using the difference to a previous time stamp??
		return m_rtc.getSelectedRegisterValue(); 
	}

	assert(isCartridgeRAMOrRTCRegister(address));
	return m_ram[convertRawAddressToRAMBankAddress(address, m_ramBank)];
}

void ggb::MemoryBankControllerThree::executeDMATransfer(uint16_t startAddress, uint8_t* oam, size_t sizeInBytes) const
{
	// TODO test: this has not been really tested yet, most games seem to not use it, therefore not sure if the code below is correct
	int address = startAddress;
	if (isCartridgeRAMOrRTCRegister(address))
	{
		address = convertRawAddressToRAMBankAddress(address, m_ramBank);
		MemoryBankController::executeDMATransfer(&m_ram[address], oam, sizeInBytes);
		return;
	}

	if (isROMBankAddress(address))
		address = convertRawAddressToBankAddress(address, m_romBank);
	MemoryBankController::executeDMATransfer(&m_cartridgeData[address], oam, sizeInBytes);
}

void ggb::MemoryBankControllerThree::initialize(std::vector<uint8_t>&& cartridgeData)
{
	MemoryBankController::initialize(std::move(cartridgeData));
	if (m_hasRam)
		m_ram = std::vector<uint8_t>(std::max(static_cast<int>(RAM_BANK_SIZE), getRAMSize()), 0);
}

void ggb::MemoryBankControllerThree::serialization(Serialization* serialization)
{
	serialization->read_write(m_ramAndTimerEnabled);
	serialization->read_write(m_romBank);
	serialization->read_write(m_ramBank);
	m_rtc.serialize(serialization);
}

void ggb::MemoryBankControllerThree::setROMBank(uint8_t value)
{
	auto bank = (getROMBankCount() - 1) & value;
	m_romBank = std::max(bank, 1);
}
