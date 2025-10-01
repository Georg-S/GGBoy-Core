#pragma once
#include <cstdint>

namespace ggb
{
	// Bits
	constexpr int BIT0 = 0b00000001;
	constexpr int BIT1 = 0b00000010;
	constexpr int BIT2 = 0b00000100;
	constexpr int BIT3 = 0b00001000;
	constexpr int BIT4 = 0b00010000;
	constexpr int BIT5 = 0b00100000;
	constexpr int BIT6 = 0b01000000;
	constexpr int BIT7 = 0b10000000;
	constexpr uint16_t INTERRUPT_VBLANK_BIT = BIT0;
	constexpr uint16_t INTERRUPT_LCD_STAT_BIT = BIT1;
	constexpr uint16_t INTERRUPT_TIMER_BIT = BIT2;
	constexpr uint16_t INTERRUPT_SERIAL_BIT = BIT3;
	constexpr uint16_t INTERRUPT_JOYPAD_BIT = BIT4;
	// ADDRESSES
	constexpr uint16_t CARTRIDGE_ROM_START_ADDRESS = 0x0000;
	constexpr uint16_t VBLANK_INTERRUPT_ADDRESS = 0x40;
	constexpr uint16_t LCD_STAT_INTERRUPT_ADDRESS = 0x48;
	constexpr uint16_t TIMER_INTERRUPT_ADDRESS = 0x50;
	constexpr uint16_t SERIAL_INTERRUPT_ADDRESS = 0x58;
	constexpr uint16_t JOYPAD_INTERRUPT_ADDRESS = 0x60;
	constexpr uint16_t GBC_FLAG_ADDRESS = 0x143;
	constexpr uint16_t MBC_TYPE_ADDRESS = 0x147;
	constexpr uint16_t CARTRIDGE_ROM_END_ADDRESS = 0x7FFF;
	constexpr uint16_t VRAM_START_ADDRESS = 0x8000;
	constexpr uint16_t TILE_MAP_1_ADDRESS = VRAM_START_ADDRESS;
	constexpr uint16_t TILE_MAP_2_ADDRESS = 0x9000;
	constexpr uint16_t VRAM_END_ADDRESS = 0x9FFF;
	constexpr uint16_t CARTRIDGE_RAM_START_ADDRESS = 0xA000;
	constexpr uint16_t CARTRIDGE_RAM_END_ADDRESS = 0xBFFF;
	constexpr uint16_t WRAM_START_ADDRESS = 0xC000;
	constexpr uint16_t WRAM_SWITCHABLE_BANK_START_ADDRESS = 0xD000;
	constexpr uint16_t WRAM_END_ADDRESS = 0xDFFF;
	constexpr uint16_t ECHO_RAM_START_ADDRESS = 0xE000;
	constexpr uint16_t ECHO_RAM_END_ADDRESS = 0xFDFF;
	constexpr uint16_t OAM_ADDRESS = 0xFE00; // OAM = object attribute memory
	constexpr uint16_t UNUSED_MEMORY_START_ADDRESS = 0xFEA0;
	constexpr uint16_t UNUSED_MEMORY_END_ADDRESS = 0xFEFF;
	constexpr uint16_t INPUT_REGISTER_ADDRESS = 0xFF00;
	constexpr uint16_t SERIAL_TRANSFER_ADDRESS = 0xFF01;
	constexpr uint16_t SERIAL_TRANSFER_CONTROL_ADDRESS = 0xFF02;
	constexpr uint16_t TIMER_DIVIDER_REGISTER_ADDRESS = 0xFF04;
	constexpr uint16_t TIMER_COUNTER_ADDRESS = 0xFF05;
	constexpr uint16_t TIMER_MODULO_ADDRESS = 0xFF06;
	constexpr uint16_t TIMER_CONTROL_ADDRESS = 0xFF07;
	constexpr uint16_t INTERRUPT_REQUEST_ADDRESS = 0xFF0F;
	constexpr uint16_t AUDIO_CHANNEL_1_FREQUENCY_SWEEP_ADDRESS = 0xFF10;
	constexpr uint16_t AUDIO_MEMORY_START_ADDRESS= AUDIO_CHANNEL_1_FREQUENCY_SWEEP_ADDRESS;
	constexpr uint16_t AUDIO_CHANNEL_1_LENGTH_DUTY_ADDRESS = 0xFF11;
	constexpr uint16_t AUDIO_CHANNEL_1_VOLUME_ENVELOPE_ADDRESS = 0xFF12;
	constexpr uint16_t AUDIO_CHANNEL_1_PERIOD_LOW_ADDRESS = 0xFF13;
	constexpr uint16_t AUDIO_CHANNEL_1_PERIOD_HIGH_CONTROL_ADDRESS = 0xFF14;
	constexpr uint16_t AUDIO_CHANNEL_2_LENGTH_DUTY_ADDRESS = 0xFF16;
	constexpr uint16_t AUDIO_CHANNEL_2_VOLUME_ENVELOPE_ADDRESS = 0xFF17;
	constexpr uint16_t AUDIO_CHANNEL_2_PERIOD_LOW_ADDRESS = 0xFF18;
	constexpr uint16_t AUDIO_CHANNEL_2_PERIOD_HIGH_CONTROL_ADDRESS = 0xFF19;
	constexpr uint16_t AUDIO_CHANNEL_3_DAC_ENABLE_ADDRESS = 0xFF1A;
	constexpr uint16_t AUDIO_CHANNEL_3_LENGTH_TIMER_ADDRESS = 0xFF1B;
	constexpr uint16_t AUDIO_CHANNEL_3_OUTPUT_LEVEL_ADDRESS = 0xFF1C;
	constexpr uint16_t AUDIO_CHANNEL_3_PERIOD_LOW_ADDRESS = 0xFF1D;
	constexpr uint16_t AUDIO_CHANNEL_3_PERIOD_HIGH_CONTROL_ADDRESS = 0xFF1E;
	constexpr uint16_t AUDIO_CHANNEL_4_LENGTH_TIMER_ADDRESS = 0xFF20;
	constexpr uint16_t AUDIO_CHANNEL_4_VOLUME_ENVELOPE_ADDRESS = 0xFF21;
	constexpr uint16_t AUDIO_CHANNEL_4_FREQUENCY_RANDOMNESS_ADDRESS = 0xFF22;
	constexpr uint16_t AUDIO_CHANNEL_4_CONTROL_ADDRESS = 0xFF23;
	constexpr uint16_t AUDIO_MASTER_VOLUME_VIN_PANNING_ADDRESS = 0xFF24;
	constexpr uint16_t AUDIO_SOUND_PANNING_ADDRESS = 0xFF25;
	constexpr uint16_t AUDIO_MASTER_CONTROL_ADDRESS = 0xFF26;
	constexpr uint16_t AUDIO_CHANNEL_3_WAVE_PATTERN_RAM_START_ADDRESS = 0xFF30;
	constexpr uint16_t AUDIO_CHANNEL_3_WAVE_PATTERN_RAM_END_ADDRESS = 0xFF3F;
	constexpr uint16_t AUDIO_MEMORY_END_ADDRESS = AUDIO_CHANNEL_3_WAVE_PATTERN_RAM_END_ADDRESS;
	constexpr uint16_t LCD_CONTROL_REGISTER_ADDRESS = 0xFF40;
	constexpr uint16_t LCD_STATUS_REGISTER_ADDRESS = 0xFF41;
	constexpr uint16_t LCD_VIEWPORT_Y_ADDRESS = 0xFF42;
	constexpr uint16_t LCD_VIEWPORT_X_ADDRESS = 0xFF43;
	constexpr uint16_t LCD_Y_COORDINATE_ADDRESS = 0xFF44;
	constexpr uint16_t LCD_Y_COMPARE_ADDRESS = 0xFF45;
	constexpr uint16_t START_DIRECT_MEMORY_ACCESS_ADDRESS = 0xFF46;
	constexpr uint16_t BACKGROUND_PALETTE_ADDRESS = 0xFF47;
	constexpr uint16_t OBJECT_PALETTE_0_ADDRESS = 0xFF48;
	constexpr uint16_t OBJECT_PALETTE_1_ADDRESS = 0xFF49;
	constexpr uint16_t LCD_WINDOW_Y_ADDRESS = 0xFF4A;
	constexpr uint16_t LCD_WINDOW_X_ADDRESS = 0xFF4B;
	constexpr uint16_t GBC_SPEED_SWITCH_ADDRESS = 0xFF4D;
	constexpr uint16_t GBC_VRAM_BANKING_ADDRESS = 0xFF4F;
	constexpr uint16_t GBC_VRAM_DMA_SOURCE_HIGH_ADDRESS = 0xFF51;
	constexpr uint16_t GBC_VRAM_DMA_SOURCE_LOW_ADDRESS = 0xFF52;
	constexpr uint16_t GBC_VRAM_DMA_DESTINATION_HIGH_ADDRESS = 0xFF53;
	constexpr uint16_t GBC_VRAM_DMA_DESTINATION_LOW_ADDRESS = 0xFF54;
	constexpr uint16_t GBC_VRAM_DMA_LENGTH_START_ADDRESS = 0xFF55;
	constexpr uint16_t GBC_INFRARED_ADDRESS = 0xFF56;
	constexpr uint16_t GBC_BACKGROUND_PALETTE_SPECIFICATION_ADDRESS = 0xFF68; // Background palette index
	constexpr uint16_t GBC_BACKGROUND_PALETTE_DATA_ADDRESS = 0xFF69; 
	constexpr uint16_t GBC_OBJECT_COLOR_PALETTE_SPECIFICATION_ADDRESS = 0xFF6A;
	constexpr uint16_t GBC_OBJECT_COLOR_PALETTE_DATA_ADDRESS = 0xFF6B;
	constexpr uint16_t GBC_OBJECT_PRIORITY_MODE_ADDRESS = 0xFF6C;
	constexpr uint16_t GBC_WRAM_BANKING_ADDRESS = 0xFF70;
	constexpr uint16_t ENABLED_INTERRUPT_ADDRESS = 0xFFFF;


	constexpr uint32_t CPU_BASE_CLOCK = 4194304; // frequency in hz
	constexpr uint32_t PERIOD_DIVIDER_CLOCK = 1048576; // frequency in hz
	constexpr double NANO_SECONDS_PER_CYCLE = 1000000000.0 / CPU_BASE_CLOCK;
	constexpr uint32_t STANDARD_SAMPLE_RATE = 44100; // In hertz
	constexpr uint16_t TIMER_DIVIDER_REGISTER_INCREMENT_COUNT = CPU_BASE_CLOCK / 16384;
	constexpr uint16_t GAME_WINDOW_WIDTH = 160;
	constexpr uint16_t GAME_WINDOW_HEIGHT = 144;
	constexpr uint16_t TILE_DATA_WIDTH = 300;
	constexpr uint16_t TILE_DATA_HEIGHT = 200;
	constexpr uint16_t RAM_BANK_SIZE = 0x2000;
	constexpr uint16_t ROM_BANK_SIZE = 0x4000;
	constexpr uint16_t OAM_MEMORY_SIZE = 0xA0;
	constexpr uint16_t OBJECT_COUNT = 40;
	constexpr uint16_t OBJECT_MEMORY_SIZE = 4; // in bytes
	constexpr uint16_t MAX_ALLOWED_OBJS_PER_SCANLINE = 40;
	constexpr uint16_t TILE_MEMORY_SIZE = 16; // in bytes
	constexpr uint16_t TILE_WIDTH = 8; // in pixel
	constexpr uint16_t TILE_HEIGHT = 8; // in pixel
	constexpr uint16_t TILE_MAP_WIDTH = 32;
	constexpr uint16_t TILE_MAP_HEIGHT = 32;
	constexpr uint16_t TILE_MAP_SIZE = TILE_MAP_WIDTH * TILE_MAP_HEIGHT;
	constexpr uint16_t VRAM_BANK_MEMORY_SIZE = 0x1FFF + 1;
	constexpr uint16_t WRAM_BANK_MEMORY_SIZE = 0x1000;
	constexpr uint16_t GBC_VRAM_BANK_COUNT = 2;
	constexpr uint16_t GBC_WRAM_BANK_COUNT = 8;
	constexpr uint16_t GBC_COLOR_RAM_MEMORY_SIZE = 64; // in bytes
	constexpr uint16_t GBC_COLOR_PALETTE_COUNT = 8;
	using AUDIO_FORMAT = int16_t;

	template <uint16_t from, uint16_t to>
	struct AddressRange
	{
		constexpr bool operator()(uint16_t address) const
		{
			static_assert(from <= to, "Invalid range: from is greater than to");
			return (from <= address) && (address <= to);
		}
	};
	// Memory map
	constexpr AddressRange<CARTRIDGE_ROM_START_ADDRESS, CARTRIDGE_ROM_END_ADDRESS> isCartridgeROMAddress = {};
	constexpr AddressRange<VRAM_START_ADDRESS, VRAM_END_ADDRESS> isVRAMAddress = {};
	constexpr AddressRange<CARTRIDGE_RAM_START_ADDRESS, CARTRIDGE_RAM_END_ADDRESS> isCartridgeRAMAddress = {};
	constexpr AddressRange<WRAM_START_ADDRESS, WRAM_END_ADDRESS> isWRAMAddress = {};
	constexpr AddressRange<ECHO_RAM_START_ADDRESS, ECHO_RAM_END_ADDRESS> isEchoRAMAddress = {};
	constexpr AddressRange<UNUSED_MEMORY_START_ADDRESS, UNUSED_MEMORY_END_ADDRESS> isUnusedMemoryAddress = {};
	constexpr AddressRange<AUDIO_MEMORY_START_ADDRESS, AUDIO_MEMORY_END_ADDRESS> isAudioAddress = {};
}