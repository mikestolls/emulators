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

		u8* scanline = 0;
		u8* coincidence_scanline = 0;
		u8* lcd_control = 0;
		u8* lcd_status = 0;
		
		const s32 horz_cycles = 456; // cycles per horz scanline
		s32 horz_cycle_count = horz_cycles;

		int reset()
		{
			*scanline = 0;

			horz_cycle_count = horz_cycles;

			return 0;
		}

		int initialize(gameboy::memory_module* memory)
		{
			memory_module = memory;
			
			// setup memory ptrs
			scanline = memory_module->get_memory(0xFF44);
			coincidence_scanline = memory_module->get_memory(0xFF45);
			lcd_control = memory_module->get_memory(0xFF40);
			lcd_status = memory_module->get_memory(0xFF41);

			reset();

			return 0;
		}

		int update(u8 cycles)
		{
			horz_cycle_count -= cycles;
			if (horz_cycle_count <= 0)
			{
				// continue to next scanline
				horz_cycle_count = horz_cycles;

				(*scanline)++;

				if (*scanline < 144)
				{
					// draw the scan line
				}
				else if (*scanline == 144)
				{
					// start of vblank
				}
				else if (*scanline > 153)
				{
					// reset scanline to 0
					*scanline = 0;
				}
			}

			return 0;
		}
	}
}