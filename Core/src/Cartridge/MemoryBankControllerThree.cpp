#include "Cartridge/MemoryBankControllerThree.hpp"

#include <cassert>

void ggb::MemoryBankControllerThree::RealTimeClock::serialize(Serialization* serialization)
{
	serialization->read_write(seconds);
	serialization->read_write(minutes);
	serialization->read_write(hours);
	serialization->read_write(daysLower);
	serialization->read_write(daysUpperAndFlags);
	serialization->read_write(isLatched);
	serialization->read_write(lastLatchValue);
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
			m_selectedRTCRegister = nullptr;
			m_ramBank = value;
			return;
		}

		if (value == 0x08)
			m_selectedRTCRegister = &m_rtc.seconds;
		else if (value == 0x09)
			m_selectedRTCRegister = &m_rtc.minutes;
		else if (value == 0x0A)
			m_selectedRTCRegister = &m_rtc.hours;
		else if (value == 0x0B)
			m_selectedRTCRegister = &m_rtc.daysLower;
		else if (value == 0x0C)
			m_selectedRTCRegister = &m_rtc.daysUpperAndFlags;
		return;
	}

	if (isLatchClockDataAddress(address)) 
	{
		if ((m_rtc.lastLatchValue == 0) && (value == 0x1)) 
			m_rtc.isLatched = !m_rtc.isLatched;
		m_rtc.lastLatchValue = value;
		return;
	}

	assert(isCartridgeRAMOrRTCRegister(address));
	if (m_selectedRTCRegister) 
	{
		*m_selectedRTCRegister = value;
		return;
	}

	m_ram[convertRawAddressToRAMBankAddress(address, m_ramBank)] = value;
}

uint8_t ggb::MemoryBankControllerThree::read(uint16_t address) const
{
	if (isFirstROMBankAddress(address))
		return m_cartridgeData[address];
	if (isROMBankAddress(address))
		return m_cartridgeData[convertRawAddressToBankAddress(address, m_romBank)];
	if (!m_ramAndTimerEnabled)
		return 0xFF;
	if (m_selectedRTCRegister) 
	{
		// TODO handle RTC reading correctly,
		// time needs to increase maybe by using unix timestamps 
		// and using the difference to a previous time stamp??
		return *m_selectedRTCRegister; 
	}

	assert(isRAMBankingOrRTCRegisterSelectAddress(address));
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
	m_cartridgeData = std::move(cartridgeData);
	if (m_hasRam)
		m_ram = std::vector<uint8_t>(std::max(static_cast<int>(RAM_BANK_SIZE), getRAMSize()), 0);
}

void ggb::MemoryBankControllerThree::serialization(Serialization* serialization)
{
	serialization->read_write(m_ramAndTimerEnabled);
	serialization->read_write(m_romBank);
	serialization->read_write(m_ramBank);
	serialization->read_write(m_selectedRTCRegister);
	m_rtc.serialize(serialization);
}

void ggb::MemoryBankControllerThree::setROMBank(uint8_t value)
{
	auto bank = (getROMBankCount() - 1) & value;
	m_romBank = std::max(bank, 1);
}
