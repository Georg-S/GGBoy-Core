#include "Cartridge/MemoryBankControllerThree.hpp"

#include <cassert>

#include "Utility.hpp"

uint8_t& ggb::MemoryBankControllerThree::RealTimeClock::getRegister(Registers& registers, RegisterType selectedRegister) const
{
	if (selectedRegister == RegisterType::SECONDS)
		return registers.m_seconds;
	if (selectedRegister == RegisterType::MINUTES)
		return registers.m_minutes;
	if (selectedRegister == RegisterType::HOURS)
		return registers.m_hours;
	if (selectedRegister == RegisterType::DAYSLOWER)
		return registers.m_daysLower;
	if (selectedRegister == RegisterType::DAYSUPPERFLAGS)
		return registers.m_daysUpperAndFlags;
	assert(!"Invalid Register");
	return registers.m_daysUpperAndFlags;
}

ggb::MemoryBankControllerThree::RealTimeClock::RegisterType ggb::MemoryBankControllerThree::RealTimeClock::getRegisterFromValue(uint8_t value)
{
	if (value == 0x08)
		return RegisterType::SECONDS;
	if (value == 0x09)
		return RegisterType::MINUTES;
	if (value == 0x0A)
		return RegisterType::HOURS;
	if (value == 0x0B)
		return RegisterType::DAYSLOWER;
	if (value == 0x0C)
		return RegisterType::DAYSUPPERFLAGS;
	return RegisterType::NONE;
}

bool ggb::MemoryBankControllerThree::RealTimeClock::isStopped() const
{
	return isBitSet(m_registers.m_daysUpperAndFlags, 6);
}

void ggb::MemoryBankControllerThree::RealTimeClock::update()
{
	if (isStopped())
		return;

	if (!m_lastTimeStamp) 
	{
		m_lastTimeStamp = getCurrentTimeInNanoSeconds();
		return;
	}

	constexpr int SECONDS_PER_MINUTE = 60;
	constexpr int SECONDS_PER_HOUR = SECONDS_PER_MINUTE * 60;
	constexpr int SECONDS_PER_DAY = SECONDS_PER_HOUR * 24;
	constexpr int NANO_SECONDS_PER_SECOND = 1000000000;

	auto setRegistersWithSeconds = [this, SECONDS_PER_DAY, SECONDS_PER_HOUR, SECONDS_PER_MINUTE](long long secondsPassed)
	{
		uint32_t days = static_cast<int>(secondsPassed / SECONDS_PER_DAY);
		if (days > 0x1FF)
			setBit(m_registers.m_daysUpperAndFlags, 7);
		setBitToValue(m_registers.m_daysUpperAndFlags, 0, isBitSet(days, 8));
		m_registers.m_daysLower = static_cast<uint8_t>(days & 0XFF);
		secondsPassed -= (days * SECONDS_PER_DAY);
		m_registers.m_seconds = secondsPassed % 60;
		secondsPassed -= m_registers.m_seconds;
		m_registers.m_hours = static_cast<uint8_t>(secondsPassed / SECONDS_PER_HOUR);
		secondsPassed -= m_registers.m_hours * SECONDS_PER_HOUR;
		m_registers.m_minutes = static_cast<uint8_t>(secondsPassed / SECONDS_PER_MINUTE);
	};

	const auto currentTime = getCurrentTimeInNanoSeconds();
	auto timePassed = currentTime - m_lastTimeStamp + m_subSeconds;
	m_subSeconds = timePassed % NANO_SECONDS_PER_SECOND;
	auto secondsPassed = timePassed / NANO_SECONDS_PER_SECOND;

	uint32_t days = m_registers.m_daysLower;
	bool mostSignificantDayBitSet = isBitSet(m_registers.m_daysUpperAndFlags, 0);
	setBitToValue(days, 8, mostSignificantDayBitSet);

	secondsPassed += m_registers.m_seconds;
	secondsPassed += m_registers.m_minutes * SECONDS_PER_MINUTE;
	secondsPassed += m_registers.m_hours * SECONDS_PER_HOUR;
	secondsPassed += days * SECONDS_PER_DAY;

	setRegistersWithSeconds(secondsPassed);
	m_lastTimeStamp = currentTime;
}

void ggb::MemoryBankControllerThree::RealTimeClock::serialize(Serialization* serialization)
{
	auto serializeRegisters = [](Registers& registers, Serialization* serialization)
	{
		serialization->read_write(registers.m_seconds);
		serialization->read_write(registers.m_minutes);
		serialization->read_write(registers.m_hours);
		serialization->read_write(registers.m_daysLower);
		serialization->read_write(registers.m_daysUpperAndFlags);
	};

	serializeRegisters(m_latchedRegisters, serialization);
	serializeRegisters(m_registers, serialization);
	serialization->read_write(m_lastTimeStamp);
	serialization->read_write(m_subSeconds);
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
	const bool stopped = isStopped();
	update();

	auto& reg = getRegister(m_registers, m_selectedRegister);
	reg = value;

	const bool stoppedNow = isStopped();
	if (stopped && !stoppedNow) 
		m_lastTimeStamp = getCurrentTimeInNanoSeconds();
}

uint8_t ggb::MemoryBankControllerThree::RealTimeClock::getSelectedRegisterValue()
{
	if (!isStopped())
		update();

	if (m_isLatched)
		return getRegister(m_latchedRegisters, m_selectedRegister);

	return getRegister(m_registers, m_selectedRegister);
}

bool ggb::MemoryBankControllerThree::RealTimeClock::registerSelected() const
{
	return m_selectedRegister != RegisterType::NONE;
}

void ggb::MemoryBankControllerThree::RealTimeClock::resetRegisterSelection()
{
	m_selectedRegister = RegisterType::NONE;
}

void ggb::MemoryBankControllerThree::RealTimeClock::handleLatching(uint8_t value)
{
	if ((m_lastLatchValue == 0) && (value == 0x1)) 
	{
		m_isLatched = !m_isLatched;
		if (m_isLatched)
			m_latchedRegisters = m_registers;
	}
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

void ggb::MemoryBankControllerThree::initialize(std::vector<uint8_t>&& cartridgeData)
{
	MemoryBankController::initialize(std::move(cartridgeData));
	if (m_hasRam)
		m_ram = std::vector<uint8_t>(std::max(static_cast<int>(RAM_BANK_SIZE), getRAMSize()), 0);
}

void ggb::MemoryBankControllerThree::serialization(Serialization* serialization)
{
	MemoryBankController::serialization(serialization);
	serialization->read_write(m_ramAndTimerEnabled);
	serialization->read_write(m_romBank);
	serialization->read_write(m_ramBank);
	m_rtc.serialize(serialization);
}

void ggb::MemoryBankControllerThree::saveRTC(const std::filesystem::path& path)
{
	Serialize serialize = Serialize(path);
	m_rtc.serialize(&serialize);
}

void ggb::MemoryBankControllerThree::loadRTC(const std::filesystem::path& path)
{
	Deserialize deserialize = Deserialize(path);
	m_rtc.serialize(&deserialize);
}

void ggb::MemoryBankControllerThree::setROMBank(uint8_t value)
{
	auto bank = (getROMBankCount() - 1) & value;
	m_romBank = std::max(bank, 1);
}
