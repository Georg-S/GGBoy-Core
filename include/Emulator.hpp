#pragma once
#include <memory>
#include <filesystem>

#include "BUS.hpp"
#include "CPU.hpp"
#include "Cartridge/Cartridge.hpp"
#include "Timer.hpp"
#include "Input.hpp"
#include "Audio/AudioProcessingUnit.hpp"
#include "PixelProcessingUnit.hpp"
#include "RenderingUtility.hpp"
#include "Serialization.hpp"


namespace ggb
{
	class Emulator
	{
	public:
		Emulator();
		bool loadCartridge(const std::filesystem::path& path);
		void step();
		void reset();
		void setTileDataRenderer(std::unique_ptr<ggb::Renderer> renderer);
		void setGameRenderer(std::unique_ptr<ggb::Renderer> renderer);
		// Not const because "serialization" is called and this method is used for read and write and therefore cannot be const
		bool saveEmulatorState(const std::filesystem::path& outputPath);
		bool loadEmulatorState(const std::filesystem::path& filePath);
		void saveRAM(const std::filesystem::path& path);
		void loadRAM(const std::filesystem::path& path);
		void saveRTC(const std::filesystem::path& path) const;
		void loadRTC(const std::filesystem::path& path);
		void setEmulationSpeed(double emulationSpeed); // 1.0 is the normal and default speed, the higher - the faster the emulator runs
		double emulationSpeed() const;
		SampleBuffer* getSampleBuffer();
		Dimensions getTileDataDimensions() const;
		Dimensions getGameWindowDimensions() const;
		void muteChannel(size_t channelID, bool mute);
		bool isChannelMuted(size_t channelID) const;
		double getMaxSpeedup() const;
		bool isCartridgeLoaded() const;
		std::filesystem::path getLoadedCartridgePath() const;
		void pause();
		void resume();
		bool isPaused() const;
		void setInputState(const ggb::GameboyInput& input);
		void setColorCorrectionEnabled(bool enabled);

	private:
		void rewire();
		void synchronizeEmulatorMasterClock(int elapsedCycles);
		void serialization(ggb::Serialization* serialization);

		std::unique_ptr<CPU> m_cpu;
		std::unique_ptr<BUS> m_bus;
		std::unique_ptr<Cartridge> m_currentCartridge;
		std::unique_ptr<PixelProcessingUnit> m_ppu;
		std::unique_ptr<Timer> m_timer;
		std::unique_ptr<Input> m_input;
		std::unique_ptr<AudioProcessingUnit> m_audio;
		std::filesystem::path m_loadedCartridgePath;
		int m_syncCounter = 0;
		long long m_previousTimeStamp = 0;
		double m_emulationSpeed = 1.0;
		double m_lastMaxSpeedup = 1.0;
		double m_masterSynchronizationAfterCPUCycles = 0.0;
		long long m_speedupTimeCounter = 0;
		long long m_speedupCycleCounter = 0;
		bool m_paused = false;
	};
}