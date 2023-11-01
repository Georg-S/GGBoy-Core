cmake_minimum_required (VERSION 3.8)
project(GGBoyApp)
#set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(SOURCES "src/main.cpp")
set(HEADERS "")

#source_group("" FILES ${miscFiles})

if	(WIN32)
	set(SDL2_INCLUDE_DIRS $ENV{SDL2_INCLUDE})
	set(SDL2_LIBRARIES "$ENV{SDL2_BIN}/SDL2.lib")
endif(WIN32)

add_executable(GGBoyApp 
	${SOURCES}
	${HEADERS}
	)
target_include_directories(GGBoyApp PUBLIC "include" ${SDL2_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS})
target_link_libraries(GGBoyApp "GGBoyCore" ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT GGBoyApp)

if (MSVC)
    add_custom_command(TARGET GGBoyApp
                        POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "$ENV{SDL2_BIN}/SDL2.dll"
                        $<TARGET_FILE_DIR:GGBoyApp>)
endif (MSVC)