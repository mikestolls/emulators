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
		MEMORY_HRAM,
		MEMORY_IE_REGISTERS,
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
		u8 working_ram[0xDFFF - 0xC000];

		memory_map_object memory_map[MEMORY_COUNT];
		
		memory_module(gameboy::rom* rom)
		{
			memory_map[MEMORY_CATRIDGE_ROM]				= { 0x0000, 0x3FFF, rom->romdata, MEMORY_READABLE }; // MEMORY_CATRIDGE_ROM - assert if addr > romsize
			memory_map[MEMORY_CATRIDGE_SWITCHABLE_ROM]	= { 0x4000, 0x7FFF, nullptr, MEMORY_READABLE }; // MEMORY_CATRIDGE_SWITCHABLE_ROM
			memory_map[MEMORY_VRAM]						= { 0x8000, 0x9FFF, nullptr, MEMORY_READABLE | MEMORY_WRITABLE }; // MEMORY_VRAM
			memory_map[MEMORY_EXTERNAL_RAM]				= { 0xA000, 0xBFFF, nullptr, MEMORY_READABLE | MEMORY_WRITABLE }; // MEMORY_EXTERNAL_RAM
			memory_map[MEMORY_WORKING_RAM]				= { 0xC000, 0xDFFF, working_ram, MEMORY_READABLE | MEMORY_WRITABLE }; //MEMORY_WORKING_RAM
			memory_map[MEMORY_ECHO_RAM]					= { 0xE000, 0xFDFF, working_ram, MEMORY_READABLE | MEMORY_WRITABLE }; //MEMORY_ECHO_RAM
			memory_map[MEMORY_OAM]						= { 0xFE00, 0xFE9F, nullptr, MEMORY_READABLE | MEMORY_WRITABLE }; //MEMORY_ECHO_RAM
			memory_map[MEMORY_NOTUSED]					= { 0xFEA0, 0xFEFF, nullptr, 0 }; //MEMORY_ECHO_RAM
			memory_map[MEMORY_IO_REGISTERS]				= { 0xFF00, 0xFF7F, nullptr, MEMORY_READABLE | MEMORY_WRITABLE }; //MEMORY_ECHO_RAM
			memory_map[MEMORY_HRAM]						= { 0xFF80, 0xFFFE, nullptr, MEMORY_READABLE | MEMORY_WRITABLE }; //MEMORY_ECHO_RAM
			memory_map[MEMORY_IE_REGISTERS]				= { 0xFFFF, 0xFFFF, nullptr, MEMORY_READABLE | MEMORY_WRITABLE }; //MEMORY_ECHO_RAM
		}

		u8* get_memory(u16 addr)
		{
			// loop though memory map
			for (unsigned int i = 0; i < MEMORY_COUNT; i++)
			{
				if (addr <= memory_map[i].addr_max)
				{
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