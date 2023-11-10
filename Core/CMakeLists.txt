cmake_minimum_required (VERSION 3.8)
project(GGBoyCore)
#set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set (AUDIO_HEADERS
	"include/Audio/AudioProcessingUnit.hpp"
	"include/Audio/SquareWaveChannel.hpp"
	"include/Audio/WaveChannel.hpp"
	"include/Audio/NoiseChannel.hpp"
	"include/Audio/AudioCommon.hpp"
	)
	
set (CARTRIDGE_HEADERS
	"include/Cartridge/Cartridge.hpp"
	"include/Cartridge/MemoryBankController.hpp"
	"include/Cartridge/MemoryBankControllerNone.hpp"
	"include/Cartridge/MemoryBankControllerOne.hpp"
	"include/Cartridge/MemoryBankControllerFive.hpp"
	)


set (MISCELLANEOUS_HEADERS
	"include/BUS.hpp"
	"include/CPU.hpp"
	"include/CPUInstructions.hpp"
	"include/Logging.hpp"
	"include/Emulator.hpp"
	"include/CPUState.hpp"
	"include/Utility.hpp"
	"include/PixelProcessingUnit.hpp"
	"include/RenderingUtility.hpp"
	"include/Constants.hpp"
	"include/Timer.hpp"
	"include/Input.hpp"
	"include/Ringbuffer.hpp"
	"include/Serialization.hpp"
	"include/GBCColorRAM.hpp"
	)

set(HEADERS 
	${MISCELLANEOUS_HEADERS}
	${AUDIO_HEADERS}
	${CARTRIDGE_HEADERS}
	)

set (AUDIO_SOURCES
	"src/Audio/AudioProcessingUnit.cpp"
	"src/Audio/SquareWaveChannel.cpp"
	"src/Audio/WaveChannel.cpp"
	"src/Audio/NoiseChannel.cpp"
	"src/Audio/AudioCommon.cpp"
	)

set (CARTRIDGE_SOURCES
	"src/Cartridge/Cartridge.cpp"
	"src/Cartridge/MemoryBankController.cpp"
	"src/Cartridge/MemoryBankControllerNone.cpp"
	"src/Cartridge/MemoryBankControllerOne.cpp"
	"src/Cartridge/MemoryBankControllerFive.cpp"
	)
	

set (MISCELLANEOUS_SOURCES
	"src/BUS.cpp"
	"src/CPU.cpp"
	"src/CPUInstructions.cpp"
	"src/Logging.cpp"
	"src/Emulator.cpp"
	"src/CPUState.cpp"
	"src/PixelProcessingUnit.cpp"
	"src/RenderingUtility.cpp"
	"src/Utility.cpp"
	"src/Timer.cpp"
	"src/Input.cpp"
	"src/GBCColorRAM.cpp"
	)

set(SOURCES 
	${MISCELLANEOUS_SOURCES}
	${AUDIO_SOURCES}
	${CARTRIDGE_SOURCES}
	)


source_group("Header Files/Audio" FILES ${AUDIO_HEADERS})
source_group("Header Files/Cartridge" FILES ${CARTRIDGE_HEADERS})
source_group("Header Files" FILES ${MISCELLANEOUS_HEADERS})
source_group("Source Files/Audio" FILES ${AUDIO_SOURCES})
source_group("Source Files/Cartridge" FILES ${CARTRIDGE_SOURCES})
source_group("Source Files" FILES ${MISCELLANEOUS_SOURCES})

add_library(GGBoyCore STATIC 
	${SOURCES}
	${HEADERS}
	)
target_include_directories(GGBoyCore PUBLIC "include")