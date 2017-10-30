#pragma once

#include "rom.h"

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
		warning("NOTE: eventually make memory one full buffer and map inside that");
		warning("NOTE: lock memory access during lcd modes");
		u8 working_ram[0xDFFF - 0xC000];
		u8 io_registers[0xFF7F - 0xFF00];
		u8 zero_page[0xFFFE - 0xFF80];
		u8 interrupt_enabled[1];
		u8 vram[0x9FFF - 0x8000];
		u8 oam[0xFE9F - 0xFE00];

		memory_map_object memory_map[MEMORY_COUNT];
		
		u8* get_memory(u16 addr)
		{
			// loop though memory map
			for (unsigned int i = 0; i < MEMORY_COUNT; i++)
			{
				if (addr <= memory_map[i].addr_max)
				{
					if ((memory_map[i].access & MEMORY_READABLE) == 0 || memory_map[i].memory == nullptr)
					{
						printf("Error - reading from memory map that is not readable: 0x%X\n", addr);
						return 0;
					}

					return &memory_map[i].memory[addr - memory_map[i].addr_min];
				}
			}

			printf("Error - memory map not implemented for this range of addr: 0x%X\n", addr);
			return 0;
		}

		u8 read_memory(u16 addr)
		{
			// loop though memory map
			for (unsigned int i = 0; i < MEMORY_COUNT; i++)
			{
				if (addr <= memory_map[i].addr_max)
				{
					u8 mode = memory_map[MEMORY_IO_REGISTERS].memory[0xFF41 - 0xFF00] &= 0x3; // lcd_status
					if (i == MEMORY_VRAM && mode > 2)
					{
						printf("Error - reading from memory during the wrong mode: 0x%X\n", addr);
						return 0xFF;
					}

					if (i == MEMORY_OAM && mode > 1)
					{
						printf("Error - reading from memory during the wrong mode: 0x%X\n", addr);
						return 0xFF;
					}

					if ((memory_map[i].access & MEMORY_READABLE) == 0 || memory_map[i].memory == nullptr)
					{
						printf("Error - reading from memory map that is not readable: 0x%X\n", addr);
						return 0;
					}

					return memory_map[i].memory[addr - memory_map[i].addr_min];
				}
			}

			printf("Error - memory map not implemented for this range of addr: 0x%X\n", addr);
			return 0;
		}

		void write_memory(const u16 addr, const u8 value)
		{
			write_memory(addr, &value, 1);
		}

		void write_memory(const u16 addr, const u8* value, const u8 size)
		{
			if (addr == 0xFF44) // current scanline. if anyone tries to write to this value we reset to 0
			{
				memory_map[MEMORY_IO_REGISTERS].memory[addr - memory_map[MEMORY_IO_REGISTERS].addr_min] = 0x0;
				return;
			}

			// loop though memory map
			for (unsigned int i = 0; i < MEMORY_COUNT; i++)
			{
				if (addr <= memory_map[i].addr_max)
				{
					u8 mode = memory_map[MEMORY_IO_REGISTERS].memory[0xFF41 - 0xFF00] &= 0x3; // lcd_status
					if (i == MEMORY_VRAM && mode > 2)
					{
						printf("Error - writing to memory during the wrong mode: 0x%X\n", addr);
						return;
					}

					if (i == MEMORY_OAM && mode > 1)
					{
						printf("Error - writing to memory during the wrong mode: 0x%X\n", addr);
						return;
					}

					if ((memory_map[i].access & MEMORY_WRITABLE) == 0 || memory_map[i].memory == nullptr)
					{
						printf("Error - writing to memory map that is not writable: 0x%X\n", addr);
						return;
					}

					warning("NOTE: check if memory is writable");
					memcpy(&memory_map[i].memory[addr - memory_map[i].addr_min], value, size);
					return;
				}
			}

			printf("Error - memory map not implemented for this range of addr: 0x%X\n", addr);
			return;
		}

		memory_module(gameboy::rom* rom)
		{
			memory_map[MEMORY_CATRIDGE_ROM]				= { 0x0000, 0x3FFF, rom->romdata, MEMORY_READABLE }; 
			memory_map[MEMORY_CATRIDGE_SWITCHABLE_ROM]	= { 0x4000, 0x7FFF, &rom->romdata[0x4000], MEMORY_READABLE };
			memory_map[MEMORY_VRAM]						= { 0x8000, 0x9FFF, vram, MEMORY_READABLE | MEMORY_WRITABLE };
			memory_map[MEMORY_EXTERNAL_RAM]				= { 0xA000, 0xBFFF, nullptr, MEMORY_READABLE | MEMORY_WRITABLE }; 
			memory_map[MEMORY_WORKING_RAM]				= { 0xC000, 0xDFFF, working_ram, MEMORY_READABLE | MEMORY_WRITABLE }; 
			memory_map[MEMORY_ECHO_RAM]					= { 0xE000, 0xFDFF, working_ram, MEMORY_READABLE | MEMORY_WRITABLE };
			memory_map[MEMORY_OAM]						= { 0xFE00, 0xFE9F, oam, MEMORY_READABLE | MEMORY_WRITABLE };
			memory_map[MEMORY_NOTUSED]					= { 0xFEA0, 0xFEFF, nullptr, 0 }; 
			memory_map[MEMORY_IO_REGISTERS]				= { 0xFF00, 0xFF7F, io_registers, MEMORY_READABLE | MEMORY_WRITABLE };
			memory_map[MEMORY_ZERO_PAGE]				= { 0xFF80, 0xFFFE, zero_page, MEMORY_READABLE | MEMORY_WRITABLE };
			memory_map[MEMORY_INTERRUPT_FLAG]			= { 0xFFFF, 0xFFFF, interrupt_enabled, MEMORY_READABLE | MEMORY_WRITABLE };

			reset();
		}

		int reset()
		{
			memset(vram, 0x0, sizeof(vram));
			memset(working_ram, 0x0, sizeof(working_ram));

			write_memory(0xFF05, 0x00); // TIMA
			write_memory(0xFF06, 0x00); // TMA
			write_memory(0xFF07, 0x00); // TAC
			write_memory(0xFF10, 0x80); // NR10
			write_memory(0xFF11, 0xBF); // NR11
			write_memory(0xFF12, 0xF3); // NR12
			write_memory(0xFF14, 0xBF); // NR14
			write_memory(0xFF16, 0x3F); // NR21
			write_memory(0xFF17, 0x00); // NR22
			write_memory(0xFF19, 0xBF); // NR24
			write_memory(0xFF1A, 0x7F); // NR30
			write_memory(0xFF1B, 0xFF); // NR31
			write_memory(0xFF1C, 0x9F); // NR32
			write_memory(0xFF1E, 0xBF); // NR33
			write_memory(0xFF20, 0xFF); // NR41
			write_memory(0xFF21, 0x00); // NR42
			write_memory(0xFF22, 0x00); // NR43
			write_memory(0xFF23, 0xBF); // NR30
			write_memory(0xFF24, 0x77); // NR50
			write_memory(0xFF25, 0xF3); // NR51
			write_memory(0xFF26, 0xF1); // GB
			write_memory(0xFF40, 0x91); // LCDC
			write_memory(0xFF42, 0x00); // SCY
			write_memory(0xFF43, 0x00); // SCX
			write_memory(0xFF44, 0x00); // SCANLINE
			write_memory(0xFF45, 0x00); // LYC
			write_memory(0xFF47, 0xFC); // BGP
			write_memory(0xFF48, 0xFF); // OBP0
			write_memory(0xFF49, 0xFF); // OBP1
			write_memory(0xFF4A, 0x00); // WY
			write_memory(0xFF4B, 0x00); // WX
			write_memory(0xFFFF, 0x00); // IE

			// NOTE: writing to 1 to 0xFF50 unmaps boot rom
			return 0;
		}
	};
}