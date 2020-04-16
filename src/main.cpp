// main.c : Defines the entry point for the console application.
//

#include "chip8\chip8.h"
#include "gameboy\gameboy.h"

int main(int argc, const char* argv[])
{
    // main entry point for all emulators. for now just run emulator, eventually could add more global function to reset all emulators, control mapping, etc.
    //chip8::run_emulator(argc, argv);
    gameboy::run_emulator(argc, argv);

    return 0;
}

