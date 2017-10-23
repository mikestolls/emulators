#pragma once

#include "gameboy\rom.h"

namespace gameboy
{
	enum MEMORY_TYPE
	{
		MEMORY_CATRIDGE_ROM = 0,
		MEMORY_CATRIDGE_SWITCHABLE_ROM,
		MEMORY_VRAM,
		MEMORY_EXTERNAL_RAM,
		MEMORY_WORKING_RAM,
		MEMORY_ECHO_RAM,
		MEMORY_OAM,
		MEMORY_NOTUSED,
		MEMORY_IO_REGISTERS,
		MEMORY_ZERO_PAGE,
		MEMORY_INTERRUPT_FLAG,
		MEMORY_COUNT
	};

	#define MEMORY_WRITABLE		(1 << 0)
	#define MEMORY_READABLE		(1 << 1)

	struct memory_map_object
	{
		u16 addr_min;
		u16 addr_max;
		u8* memory;
		u8 access;
	};

	class memory_module
	{
	public:
		// NOTE: eventually make memory one full buffer and map inside that.
		u8 working_ram[0xDFFF - 0xC000];
		u8 io_registers[0xFF7F - 0xFF00];
		u8 zero_page[0xFFFE - 0xFF80];
		u8 interupt_enabled[1];
		u8 vram[0x9FFF - 0x8000];
		u8 oam[0xFE9F - 0xFE00];

		memory_map_object memory_map[MEMORY_COUNT];
		
		memory_module(gameboy::rom* rom)
		{
			memory_map[MEMORY_CATRIDGE_ROM]				= { 0x0000, 0x3FFF, rom->romdata, MEMORY_READABLE }; 
			memory_map[MEMORY_CATRIDGE_SWITCHABLE_ROM]	= { 0x4000, 0x7FFF, nullptr, MEMORY_READABLE };
			memory_map[MEMORY_VRAM]						= { 0x8000, 0x9FFF, vram, MEMORY_READABLE | MEMORY_WRITABLE };
			memory_map[MEMORY_EXTERNAL_RAM]				= { 0xA000, 0xBFFF, nullptr, MEMORY_READABLE | MEMORY_WRITABLE }; 
			memory_map[MEMORY_WORKING_RAM]				= { 0xC000, 0xDFFF, working_ram, MEMORY_READABLE | MEMORY_WRITABLE }; 
			memory_map[MEMORY_ECHO_RAM]					= { 0xE000, 0xFDFF, working_ram, MEMORY_READABLE | MEMORY_WRITABLE };
			memory_map[MEMORY_OAM]						= { 0xFE00, 0xFE9F, oam, MEMORY_READABLE | MEMORY_WRITABLE };
			memory_map[MEMORY_NOTUSED]					= { 0xFEA0, 0xFEFF, nullptr, 0 }; 
			memory_map[MEMORY_IO_REGISTERS]				= { 0xFF00, 0xFF7F, io_registers, MEMORY_READABLE | MEMORY_WRITABLE };
			memory_map[MEMORY_ZERO_PAGE]				= { 0xFF80, 0xFFFE, zero_page, MEMORY_READABLE | MEMORY_WRITABLE };
			memory_map[MEMORY_INTERRUPT_FLAG]			= { 0xFFFF, 0xFFFF, interupt_enabled, MEMORY_READABLE | MEMORY_WRITABLE };
		}

		u8* get_memory(u16 addr)
		{
			// loop though memory map
			for (unsigned int i = 0; i < MEMORY_COUNT; i++)
			{
				if (addr <= memory_map[i].addr_max)
				{
					if ((memory_map[i].access & MEMORY_READABLE) == 0)
					{
						printf("Error - reading from memory map that is not readable\n");
						return 0;
					}

					return &memory_map[i].memory[addr - memory_map[i].addr_min];
				}
			}

			printf("Error - memory map not implemented for this range of addr\n");
			return 0;
		}

		u8 read_memory(u16 addr)
		{
			// loop though memory map
			for (unsigned int i = 0; i < MEMORY_COUNT; i++)
			{
				if (addr <= memory_map[i].addr_max)
				{
					if ((memory_map[i].access & MEMORY_READABLE) == 0)
					{
						printf("Error - reading from memory map that is not readable\n");
						return 0;
					}

					return memory_map[i].memory[addr - memory_map[i].addr_min];
				}
			}

			printf("Error - memory map not implemented for this range of addr\n");
			return 0;
		}

		void write_memory(const u16 addr, const u8* value, const u8 size)
		{
			// loop though memory map
			for (unsigned int i = 0; i < MEMORY_COUNT; i++)
			{
				if (addr <= memory_map[i].addr_max)
				{
					if ((memory_map[i].access & MEMORY_WRITABLE) == 0)
					{
						printf("Error - writing to memory map that is not readable\n");
						return;
					}

					// note: check if memory is writable
					memcpy(&memory_map[i].memory[addr - memory_map[i].addr_min], value, size);
					return;
				}
			}

			printf("Error - memory map not implemented for this range of addr\n");
			return;
		}
	};
}