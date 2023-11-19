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
		void run();
		void step();
		void reset();
		void setTileDataRenderer(std::unique_ptr<ggb::Renderer> renderer);
		void setGameRenderer(std::unique_ptr<ggb::Renderer> renderer);
		void setInput(std::unique_ptr<Input> input);
		// Not const because "serialization" is called and this method is used for read and write and therefore cannot be const
		void saveEmulatorState(const std::filesystem::path& outputPath);
		void loadEmulatorState(const std::filesystem::path& filePath);
		void saveRAM(const std::filesystem::path& outputPath);
		void loadRAM(const std::filesystem::path& inputPath);
		void setEmulationSpeed(double emulationSpeed); // 1.0 is the normal and default speed, the higher - the faster the emulator runs
		double emulationSpeed() const;
		SampleBuffer* getSampleBuffer();
		Dimensions getTileDataDimensions() const;
		Dimensions getGameWindowDimensions() const;

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
		int m_syncCounter = 0;
		long long m_previousTimeStamp = 0;
		double m_emulationSpeed = 1.0;
	};
}