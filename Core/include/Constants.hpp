#pragma once
#include <cstdint>

namespace ggb
{
	// ADDRESSES
	static constexpr uint16_t VBLANK_INTERRUPT_ADDRESS = 0x40;
	static constexpr uint16_t LCD_STAT_INTERRUPT_ADDRESS = 0x48;
	static constexpr uint16_t TIMER_INTERRUPT_ADDRESS = 0x50;
	static constexpr uint16_t SERIAL_INTERRUPT_ADDRESS = 0x58;
	static constexpr uint16_t JOYPAD_INTERRUPT_ADDRESS = 0x60;
	static constexpr uint16_t MBC_TYPE_ADDRESS = 0x0147;
	static constexpr uint16_t TILE_MAP_1_ADDRESS = 0x8000;
	static constexpr uint16_t TILE_MAP_2_ADDRESS = 0x9000;
	static constexpr uint16_t OAM_ADDRESS = 0xFE00; // OAM = object attribute memory
	static constexpr uint16_t INPUT_REGISTER_ADDRESS = 0xFF00;
	static constexpr uint16_t SERIAL_TRANSFER_ADDRESS = 0xFF01;
	static constexpr uint16_t SERIAL_TRANSFER_CONTROL_ADDRESS = 0xFF02;
	static constexpr uint16_t TIMER_DIVIDER_REGISTER_ADDRESS = 0xFF04;
	static constexpr uint16_t TIMER_COUNTER_ADDRESS = 0xFF05;
	static constexpr uint16_t TIMER_MODULO_ADDRESS = 0xFF06;
	static constexpr uint16_t TIMER_CONTROL_ADDRESS = 0xFF07;
	static constexpr uint16_t INTERRUPT_REQUEST_ADDRESS = 0xFF0F;
	static constexpr uint16_t AUDIO_CHANNEL_1_FREQUENCY_SWEEP_ADDRESS = 0xFF10;
	static constexpr uint16_t AUDIO_CHANNEL_1_LENGTH_DUTY_ADDRESS = 0xFF11;
	static constexpr uint16_t AUDIO_CHANNEL_1_VOLUME_ENVELOPE_ADDRESS = 0xFF12;
	static constexpr uint16_t AUDIO_CHANNEL_1_PERIOD_LOW_ADDRESS = 0xFF13;
	static constexpr uint16_t AUDIO_CHANNEL_1_PERIOD_HIGH_CONTROL_ADDRESS = 0xFF14;
	static constexpr uint16_t AUDIO_CHANNEL_2_LENGTH_DUTY_ADDRESS = 0xFF16;
	static constexpr uint16_t AUDIO_CHANNEL_2_VOLUME_ENVELOPE_ADDRESS = 0xFF17;
	static constexpr uint16_t AUDIO_CHANNEL_2_PERIOD_LOW_ADDRESS = 0xFF18;
	static constexpr uint16_t AUDIO_CHANNEL_2_PERIOD_HIGH_CONTROL_ADDRESS = 0xFF19;
	static constexpr uint16_t AUDIO_CHANNEL_3_DAC_ENABLE_ADDRESS = 0xFF1A;
	static constexpr uint16_t AUDIO_CHANNEL_3_LENGTH_TIMER_ADDRESS = 0xFF1B;
	static constexpr uint16_t AUDIO_CHANNEL_3_OUTPUT_LEVEL_ADDRESS = 0xFF1C;
	static constexpr uint16_t AUDIO_CHANNEL_3_PERIOD_LOW_ADDRESS = 0xFF1D;
	static constexpr uint16_t AUDIO_CHANNEL_3_PERIOD_HIGH_CONTROL_ADDRESS = 0xFF1E;
	static constexpr uint16_t AUDIO_CHANNEL_4_LENGTH_TIMER_ADDRESS = 0xFF20;
	static constexpr uint16_t AUDIO_CHANNEL_4_VOLUME_ENVELOPE_ADDRESS = 0xFF21;
	static constexpr uint16_t AUDIO_CHANNEL_4_FREQUENCY_RANDOMNESS_ADDRESS = 0xFF22;
	static constexpr uint16_t AUDIO_CHANNEL_4_CONTROL_ADDRESS = 0xFF23;
	static constexpr uint16_t AUDIO_MASTER_VOLUME_VIN_PANNING_ADDRESS = 0xFF24;
	static constexpr uint16_t AUDIO_SOUND_PANNING_ADDRESS = 0xFF25;
	static constexpr uint16_t AUDIO_MASTER_CONTROL_ADDRESS = 0xFF26;
	static constexpr uint16_t AUDIO_CHANNEL_3_WAVE_PATTERN_RAM_START_ADDRESS = 0xFF30;
	static constexpr uint16_t AUDIO_CHANNEL_3_WAVE_PATTERN_RAM_END_ADDRESS = 0xFF3F;
	static constexpr uint16_t LCD_CONTROL_REGISTER_ADDRESS = 0xFF40;
	static constexpr uint16_t LCD_STATUS_REGISTER_ADDRESS = 0xFF41;
	static constexpr uint16_t LCD_VIEWPORT_Y_ADDRESS = 0xFF42;
	static constexpr uint16_t LCD_VIEWPORT_X_ADDRESS = 0xFF43;
	static constexpr uint16_t LCD_Y_COORDINATE_ADDRESS = 0xFF44;
	static constexpr uint16_t LCD_Y_COMPARE_ADDRESS = 0xFF45;
	static constexpr uint16_t START_DIRECT_MEMORY_ACCESS_ADDRESS = 0xFF46;
	static constexpr uint16_t BACKGROUND_PALETTE_ADDRESS = 0xFF47;
	static constexpr uint16_t OBJECT_PALETTE_0_ADDRESS = 0xFF48;
	static constexpr uint16_t OBJECT_PALETTE_1_ADDRESS = 0xFF49;
	static constexpr uint16_t LCD_WINDOW_Y_ADDRESS = 0xFF4A;
	static constexpr uint16_t LCD_WINDOW_X_ADDRESS = 0xFF4B;
	static constexpr uint16_t GBC_SPEED_SWITCH_ADDRESS = 0xFF4D;
	static constexpr uint16_t GBC_VRAM_BANKING_ADDRESS = 0xFF4F;
	static constexpr uint16_t GBC_VRAM_DMA_SOURCE_HIGH_ADDRESS = 0xFF51;
	static constexpr uint16_t GBC_VRAM_DMA_SOURCE_LOW_ADDRESS = 0xFF52;
	static constexpr uint16_t GBC_VRAM_DMA_DESTINATION_HIGH_ADDRESS = 0xFF53;
	static constexpr uint16_t GBC_VRAM_DMA_DESTINATION_LOW_ADDRESS = 0xFF54;
	static constexpr uint16_t GBC_VRAM_DMA_LENGTH_START_ADDRESS = 0xFF55;
	static constexpr uint16_t GBC_INFRARED_ADDRESS = 0xFF56;
	static constexpr uint16_t GBC_BACKGROUND_PALETTE_SPECIFICATION_ADDRESS = 0xFF68; // Background palette index
	static constexpr uint16_t GBC_BACKGROUND_PALETTE_DATA_ADDRESS = 0xFF69; 
	static constexpr uint16_t GBC_OBJECT_COLOR_PALETTE_SPECIFICATION_ADDRESS = 0xFF6A;
	static constexpr uint16_t GBC_OBJECT_COLOR_PALETTE_DATA_ADDRESS = 0xFF6B;
	static constexpr uint16_t GBC_OBJECT_PRIORITY_MODE_ADDRESS = 0xFF6C;
	static constexpr uint16_t GBC_WRAM_BANKING_ADDRESS = 0xFF70;
	static constexpr uint16_t ENABLED_INTERRUPT_ADDRESS = 0xFFFF;


	static constexpr uint32_t CPU_BASE_CLOCK = 4194304; // frequency in hz
	static constexpr uint32_t PERIOD_DIVIDER_CLOCK = 1048576; // frequency in hz
	static constexpr double NANO_SECONDS_PER_CYCLE = 1000000000.0 / CPU_BASE_CLOCK;
	static constexpr uint32_t STANDARD_SAMPLE_RATE = 44100; // In hertz
	static constexpr uint16_t TIMER_DIVIDER_REGISTER_INCREMENT_COUNT = CPU_BASE_CLOCK / 16384;
	static constexpr uint16_t INTERRUPT_VBLANK_BIT = 0;
	static constexpr uint16_t INTERRUPT_LCD_STAT_BIT = 1;
	static constexpr uint16_t INTERRUPT_TIMER_BIT = 2;
	static constexpr uint16_t INTERRUPT_SERIAL_BIT = 3;
	static constexpr uint16_t INTERRUPT_JOYPAD_BIT = 4;
	static constexpr uint16_t GAME_WINDOW_WIDTH = 160;
	static constexpr uint16_t GAME_WINDOW_HEIGHT = 144;
	static constexpr uint16_t TILE_DATA_WIDTH = 300;
	static constexpr uint16_t TILE_DATA_HEIGHT = 200;
	static constexpr uint16_t RAM_BANK_SIZE = 0x2000;
	static constexpr uint16_t ROM_BANK_SIZE = 0x4000;
	static constexpr uint16_t OAM_MEMORY_SIZE = 0xA0;
	static constexpr uint16_t OBJECT_COUNT = 40;
	static constexpr uint16_t MAX_ALLOWED_OBJS_PER_SCANLINE = 40;
	static constexpr uint16_t TILE_MEMORY_SIZE = 16; // in bytes
	static constexpr uint16_t TILE_WIDTH = 8; // in pixel
	static constexpr uint16_t TILE_HEIGHT = 8; // in pixel
	static constexpr uint16_t TILE_MAP_WIDTH = 32;
	static constexpr uint16_t TILE_MAP_HEIGHT = 32;
	static constexpr uint16_t TILE_MAP_SIZE = TILE_MAP_WIDTH * TILE_MAP_HEIGHT;
	static constexpr uint16_t VRAM_BANK_MEMORY_SIZE = 0x1FFF + 1;
	static constexpr uint16_t GBC_VRAM_BANK_COUNT = 2;
	static constexpr uint16_t GBC_COLOR_RAM_MEMORY_SIZE = 64; // in bytes
	using AUDIO_FORMAT = int16_t;
}