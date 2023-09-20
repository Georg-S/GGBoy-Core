cmake_minimum_required (VERSION 3.8)
project(GGBoyCore)
#set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(HEADERS 
	"include/BUS.hpp"
	"include/CPU.hpp"
	"include/CPUInstructions.hpp"
	"include/Cartridge.hpp"
	"include/Logging.hpp"
	"include/Emulator.hpp"
	"include/CPUState.hpp"
	"include/Utility.hpp"
	"include/PixelProcessingUnit.hpp"
	"include/RenderingUtility.hpp"
	"include/Constants.hpp"
	"include/Timer.hpp"
	"include/Input.hpp"
	)

set(SOURCES 
	"src/BUS.cpp"
	"src/CPU.cpp"
	"src/CPUInstructions.cpp"
	"src/Cartridge.cpp"
	"src/Logging.cpp"
	"src/Emulator.cpp"
	"src/CPUState.cpp"
	"src/PixelProcessingUnit.cpp"
	"src/RenderingUtility.cpp"
	"src/Utility.cpp"
	"src/Timer.cpp"
	"src/Input.cpp"
	)


#source_group("" FILES ${miscFiles})

add_library(GGBoyCore STATIC 
	${SOURCES}
	${HEADERS}
	)
target_include_directories(GGBoyCore PUBLIC "include")