#pragma once

#include "defines.h"

#include "gameboy\memory_module.h"
/*
namespace gameboy
{
	namespace cpu
	{
		static void func_nop()
		{

		}

		static void func_ld_bc_nn(u16 operand)
		{
			R.bc = operand;
		}

		struct instruction
		{
			const char* mnemonic;
			void* execute;
			u8 operands;
			u8 cycles;
		};

		static const instruction instructions[] = {
			{ "NOP", &func_nop , 0, 0 },
			{ "LD BC, nn", &func_ld_bc_nn, 2, 0 },
		};

		static const u16 instructionCount = sizeof(instructions) / sizeof(instructions[0]);
	}
}*/