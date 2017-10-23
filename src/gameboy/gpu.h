#pragma once

#include "defines.h"

#include "gameboy\memory_module.h"
#include "gameboy\interrupts.h"

namespace gameboy
{
	namespace gpu
	{
		// main memory module pointer
		gameboy::memory_module* memory_module;
		
		int reset()
		{
			return 0;
		}

		int initialize(gameboy::memory_module* memory)
		{
			memory_module = memory;
			
			reset();

			return 0;
		}

		int update()
		{
			return 0;
		}
	}
}