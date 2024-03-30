#include "EmulatorMain.hpp"

// Needed because of SDL
#undef main

int main(int argc, char* argv[])
{
	EmulatorApplication application;

	return application.run();
}