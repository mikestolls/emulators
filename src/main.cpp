// main.c : Defines the entry point for the console application.
//

#include "common/main_window.h"
#include "chip8/chip8.h"
#include "gameboy/gameboy.h"

int main(int argc, const char* argv[])
{
    // main entry point for all emulators. for now just run emulator, eventually could add more global function to reset all emulators, control mapping, etc.
    //int ret = chip8::run_emulator(argc, argv);
    //int ret = gameboy::run_emulator(argc, argv);
	int ret = common::main_window();

    return ret;
}

