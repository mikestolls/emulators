#pragma once

#include "gameboy\rom.h"

namespace gameboy
{
	class memory_module
	{
	public:
		gameboy::rom* rom;

		memory_module(gameboy::rom* _rom)
		{
			rom = _rom;
		}


		u8* get_memory(u16 addr)
		{
			if (addr <= 0x4000) // catridge rom data
			{
				return &rom->romdata[addr];
			}
			else
			{
				printf("Error - memory map not implemented for this range of addr\n");
				return 0;
			}
		}

		u8 read_memory(u16 addr)
		{
			if (addr <= 0x4000) // catridge rom data
			{
				return rom->romdata[addr];
			}
			else
			{
				printf("Error - memory map not implemented for this range of addr\n");
				return 0;
			}
		}

		void write_memory(const u16 addr, const u8* value, const u8 size)
		{
			if (addr <= 0x4000) // catridge rom data
			{
				printf("Error - memory map write to this range of addr\n");
				return;
			}
			else
			{
				printf("Error - memory map not implemented for this range of addr\n");
				return;
			}
		}
	};
}