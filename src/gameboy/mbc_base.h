#pragma once

#include "defines.h"

namespace gameboy
{
	enum CATRIDGE_TYPE
	{
		ROM_ONLY = 0,
		ROM_MBC1,
		ROM_MBC1_RAM,
		ROM_MBC1_RAM_BATTERY,
		ROM_MBC2,
		ROM_MBC2_BATTERY,
	};

	enum ROM_SIZE
	{
		ROM_32KB = 0,
		ROM_64KB,
		ROM_128KB,
		ROM_256KB,
		ROM_512KB,
		ROM_1MB,
		ROM_2MB,
		ROM_4MB,
	};

	enum RAM_SIZE
	{
		RAM_NONE = 0,
		RAM_2KB,
		RAM_8KB,
		RAM_32KB,
	};

	struct mbc_base
	{
	public:

		u8 memory[0x10000]; // cover memory maps up to index 0xFFFF

		u8* memory_rom;
		u8* memory_switchable_rom;
		u8* memory_vram;
		u8* memory_external_ram;
		u8* memory_working_ram;
		u8* memory_oam;
		u8* memory_io_registers;
		u8* memory_zero_page;
		u8* memory_interrupt_flag;
		
		virtual int initialize(ROM_SIZE romsize, RAM_SIZE ramsize, u8* romdata, u64 datasize)
		{
			memset(memory, 0x0, sizeof(memory));

			// copy in the rom data
			assert(datasize <= 0x8000);
			
			memcpy(memory, romdata, datasize);

			return 0;
		}

		virtual int write_memory(u16 addr, u8 value)
		{
			return 0;
		}

		mbc_base()
		{
			memory_rom = &memory[0x0000];
			memory_switchable_rom = &memory[0x4000];
			memory_vram = &memory[0x8000];
			memory_external_ram = &memory[0xA000];
			memory_working_ram = &memory[0xC000];
			memory_oam = &memory[0xFE00];
			memory_io_registers = &memory[0xFF00];
			memory_zero_page = &memory[0xFF80];
			memory_interrupt_flag = &memory[0xFFFF];
		}

		virtual ~mbc_base()
		{

		}
	};
}