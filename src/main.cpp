// main.c : Defines the entry point for the console application.
//

#include "emulators\chip8.h"

int main(int argc, char** argv)
{
	// main entry point for all emulators. for now just run emulator, eventually could add more global function to reset all emulators, control mapping, etc.
	chip8::run_chip8(argc, argv);

    return 0;
}

