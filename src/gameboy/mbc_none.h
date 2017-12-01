#pragma once

#include "defines.h"
#include "rom.h"
#include "boot_rom.h"

namespace gameboy
{	
	struct mbc_none
	{
		u8 memory[0x10000]; // cover memory maps up to index 0xFFFF

		int initialize()
		{
			memset(memory, 0x0, sizeof(memory));

			return 0;
		}
		
		u8* get_switchable_rom_bank()
		{
			return &memory[0x4000]; // return second rom bank area
		}

		u8* get_external_ram_bank()
		{
			return &memory[0xA000]; // return second rom bank area
		}

		mbc_none()
		{
			initialize();
		}

		~mbc_none()
		{

		}
	};
}