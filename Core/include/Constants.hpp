#pragma once
#include <cstdint>

static constexpr uint16_t VBLANK_INTERRUPT_ADDRESS =		0x40;
static constexpr uint16_t LCD_STAT_INTERRUPT_ADDRESS =		0x48;
static constexpr uint16_t TIMER_INTERRUPT_ADDRESS =			0x50;
static constexpr uint16_t SERIAL_INTERRUPT_ADDRESS =		0x58;
static constexpr uint16_t JOYPAD_INTERRUPT_ADDRESS =		0x60;
static constexpr uint16_t MBC_TYPE_ADDRESS =				0x0147;
static constexpr uint16_t LCD_CONTROL_REGISTER_ADDRESS =	0xFF40;
static constexpr uint16_t LCD_STATUS_REGISTER_ADDRESS =		0xFF41;
static constexpr uint16_t LCD_Y_COORDINATE_ADDRESS =		0xFF44;
static constexpr uint16_t LCD_Y_COMPARE_ADDRESS =			0xFF45;
static constexpr uint16_t INTERRUPT_REQUEST_ADDRESS =		0xFF0F;
static constexpr uint16_t ENABLED_INTERRUPT_ADDRESS =		0xFFFF;
