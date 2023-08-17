cmake_minimum_required (VERSION 3.8)
project(GGBoy)
set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(Core)
add_subdirectory(MainApp)