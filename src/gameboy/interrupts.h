#pragma once

#include "defines.h"

#include "gameboy\memory_module.h"

namespace gameboy
{
	namespace cpu
	{
		bool interrupt_master;

		inline void disable_interrupts()
		{
			interrupt_master = false;
		}

		inline void enable_interrupts()
		{
			interrupt_master = true;
		}
	}
}