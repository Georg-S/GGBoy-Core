#pragma once
#include <cstdint>

// ADDRESSES
static constexpr uint16_t VBLANK_INTERRUPT_ADDRESS =				0x40;
static constexpr uint16_t LCD_STAT_INTERRUPT_ADDRESS =				0x48;
static constexpr uint16_t TIMER_INTERRUPT_ADDRESS =					0x50;
static constexpr uint16_t SERIAL_INTERRUPT_ADDRESS =				0x58;
static constexpr uint16_t JOYPAD_INTERRUPT_ADDRESS =				0x60;
static constexpr uint16_t MBC_TYPE_ADDRESS =						0x0147;
static constexpr uint16_t TIMER_DIVIDER_REGISTER_ADDRESS=			0xFF04;
static constexpr uint16_t TIMER_COUNTER_ADDRESS =					0xFF05;
static constexpr uint16_t TIMER_MODULO_ADDRESS =					0xFF06;
static constexpr uint16_t TIMER_CONTROL_ADDRESS =					0xFF07;
static constexpr uint16_t LCD_CONTROL_REGISTER_ADDRESS =			0xFF40;
static constexpr uint16_t LCD_STATUS_REGISTER_ADDRESS =				0xFF41;
static constexpr uint16_t LCD_VIEWPORT_Y_ADDRESS =					0xFF42;
static constexpr uint16_t LCD_VIEWPORT_X_ADDRESS =					0xFF43;
static constexpr uint16_t LCD_Y_COORDINATE_ADDRESS =				0xFF44;
static constexpr uint16_t LCD_Y_COMPARE_ADDRESS =					0xFF45;
static constexpr uint16_t BACKGROUND_PALETTE_ADDRESS =				0xFF47;
static constexpr uint16_t OBJECT_PALETTE_0_ADDRESS =				0xFF48;
static constexpr uint16_t OBJECT_PALETTE_1_ADDRESS =				0xFF49;
static constexpr uint16_t LCD_WINDOW_Y_ADDRESS =					0xFF4A;
static constexpr uint16_t LCD_WINDOW_X_ADDRESS =					0xFF4B;
static constexpr uint16_t INTERRUPT_REQUEST_ADDRESS =				0xFF0F;
static constexpr uint16_t ENABLED_INTERRUPT_ADDRESS =				0xFFFF;


static constexpr uint16_t TIMER_DIVIDER_REGISTER_INCREMENT_COUNT = 16384;
static constexpr uint16_t INTERRUPT_VBLANK_BIT = 0;
static constexpr uint16_t INTERRUPT_LCD_STAT_BIT = 1;
static constexpr uint16_t INTERRUPT_TIMER_BIT = 2;
static constexpr uint16_t INTERRUPT_SERIAL_BIT = 3;
static constexpr uint16_t INTERRUPT_JOYPAD_BIT = 4;