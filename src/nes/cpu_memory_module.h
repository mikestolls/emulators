#pragma once

#include "defines.h"
#include "rom.h"

#include <cstdarg>

#define MEMORY_ACCESS_HARD_BLOCK

namespace nes
{
	namespace cpu_memory_module
	{
		enum MEMORY_TYPE
		{
			MEMORY_WORK_RAM = 0,
			MEMORY_WORK_MIRRORS,
			MEMORY_PPU_REGISTERS,
			MEMORY_PPU_REGISTERS_MIRRORS,
			MEMORY_REGISTERS,
			MEMORY_EXPANSION_ROM,
			MEMORY_SRAM,
			MEMORY_PRG_ROM,
			MEMORY_COUNT
		};

		#define MEMORY_WRITABLE		(1 << 0)
		#define MEMORY_READABLE		(1 << 1)

		struct memory_map_object
		{
			std::string map_name;
			u8* memory_ptr;
			u16 addr_min;
			u16 addr_max;
			u8 access;
		};

		rom* rom_ptr;
		u8 memory[0x10000]; // cover memory maps up to index 0xFFFF
		
		memory_map_object memory_map[MEMORY_COUNT] = {
			{ "WRAM", nullptr, 0x0000, 0x07FF, MEMORY_READABLE | MEMORY_WRITABLE },
			{ "MIRR", nullptr, 0x0800, 0x1FFF, MEMORY_READABLE | MEMORY_WRITABLE },
			{ " PPU", nullptr, 0x2000, 0x2007, MEMORY_READABLE | MEMORY_WRITABLE },
			{ "MIRR", nullptr, 0x2008, 0x3FFF, MEMORY_READABLE | MEMORY_WRITABLE },
			{ " REG", nullptr, 0x4000, 0x4017, MEMORY_READABLE | MEMORY_WRITABLE },
			{ "EXPA", nullptr, 0x4018, 0x5FFF, MEMORY_READABLE | MEMORY_WRITABLE },
			{ "SRAM", nullptr, 0x6000, 0x7FFF, MEMORY_READABLE | MEMORY_WRITABLE },
			{ "PROM", nullptr, 0x8000, 0xFFFF, MEMORY_READABLE },
		};

		memory_map_object* find_map(u16 addr)
		{
			for (unsigned int i = 0; i < MEMORY_COUNT; i++)
			{
				if (addr <= memory_map[i].addr_max)
				{
					return &memory_map[i];
				}
			}

			printf("Error - memory map not implemented for this range of addr: 0x%X\n", addr);
			return nullptr;
		}

		bool show_warnings = true;
		void disable_warnings() { show_warnings = false; }
		void enable_warnings() { show_warnings = true; }
		
		void print_warning(const char* str, ...)
		{
			if (show_warnings)
			{
				va_list args{};
				va_start(args, str);

				vprintf(str, args);
			}
		}

		u8* get_memory(u16 addr, bool force = false)
		{
			// loop though memory map
			for (unsigned int i = 0; i < MEMORY_COUNT; i++)
			{
				if (addr <= memory_map[i].addr_max)
				{
					if ((memory_map[i].access & MEMORY_READABLE) == 0)
					{
						print_warning("Warning - reading from memory map %d that is not readable: 0x%X\n", i, addr);
						
#ifdef MEMORY_ACCESS_HARD_BLOCK
						return 0;
#endif
					}
					
					if (memory_map[i].memory_ptr == nullptr)
					{
						return 0;
					}

					return &memory_map[i].memory_ptr[addr - memory_map[i].addr_min];
				}
			}

			printf("Error - memory map not implemented for this range of addr: 0x%X\n", addr);
			return 0;
		}

		u8 read_memory(u16 addr, bool force = false)
		{
			// loop though memory map
			for (unsigned int i = 0; i < MEMORY_COUNT; i++)
			{
				if (addr <= memory_map[i].addr_max)
				{
					if ((memory_map[i].access & MEMORY_READABLE) == 0)
					{
						printf("Warning - reading from memory map %d that is not readable: 0x%X\n", i, addr);

#ifdef MEMORY_ACCESS_HARD_BLOCK
						return 0;
#endif
					}

					if (memory_map[i].memory_ptr == nullptr)
					{
						return 0;
					}

					return memory_map[i].memory_ptr[addr - memory_map[i].addr_min];
				}
			}

			printf("Error - memory map not implemented for this range of addr: 0x%X\n", addr);
			return 0;
		}

		void write_memory(const u16 addr, const u8* value, const u8 size, bool force = false)
		{
			// loop though memory map
			for (unsigned int i = 0; i < MEMORY_COUNT; i++)
			{
				if (addr <= memory_map[i].addr_max)
				{					
					if ((memory_map[i].access & MEMORY_WRITABLE) == 0)
					{
						printf("Warning - writing to memory map %d that is not writable: 0x%X\n", i, addr);
						
#ifdef MEMORY_ACCESS_HARD_BLOCK
						return;
#endif
					}

					if (memory_map[i].memory_ptr == nullptr)
					{
						return;
					}

					memcpy(&memory_map[i].memory_ptr[addr - memory_map[i].addr_min], value, size);

					return;
				}
			}

			printf("Error - memory map not implemented for this range of addr: 0x%X\n", addr);
			return;
		}
		
		void write_memory(const u16 addr, const u8 value, bool force = false)
		{
			write_memory(addr, &value, 1, force);
		}

		int reset()
		{
			memset(memory, 0x0, sizeof(memory));

			memory_map[MEMORY_WORK_RAM].memory_ptr = &memory[0x0];
			memory_map[MEMORY_WORK_MIRRORS].memory_ptr = &memory[0x800];
			memory_map[MEMORY_PPU_REGISTERS].memory_ptr = &memory[0x2000];
			memory_map[MEMORY_PPU_REGISTERS_MIRRORS].memory_ptr = &memory[0x2008];
			memory_map[MEMORY_REGISTERS].memory_ptr = &memory[0x4000];
			memory_map[MEMORY_EXPANSION_ROM].memory_ptr = &memory[0x4018];
			memory_map[MEMORY_SRAM].memory_ptr = &memory[0x6000];
			memory_map[MEMORY_PRG_ROM].memory_ptr = &memory[0x8000];

			// no boot rom set default mem values

			return 0;
		}

		int initialize(rom* rom)
		{
			rom_ptr = rom;

			reset();

			// copy in the rom data
			assert(rom->prg_size <= 0x8000);

			memcpy(memory_map[MEMORY_PRG_ROM].memory_ptr, rom->prg_data, rom->prg_size); // copy the rom to the memory map

			if (rom->prg_size < 0x8000) // if less than 32kb we mirror. make this safer
			{
				memcpy(&memory_map[MEMORY_PRG_ROM].memory_ptr[0x4000], rom->prg_data, rom->prg_size);
			}

			return 0;
		}
	}
}