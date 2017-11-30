#pragma once

#include "defines.h"
#include "rom.h"
#include "boot_rom.h"

namespace gameboy
{
	namespace cpu
	{
		extern void reset_timer_counter();
	}
	
	namespace memory_module
	{
		enum MEMORY_TYPE
		{
			MEMORY_CARTRIDGE_ROM = 0,
			MEMORY_CARTRIDGE_SWITCHABLE_ROM,
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
			std::string map_name;
			u8* memory_ptr;
			u16 addr_min;
			u16 addr_max;
			u8 access;
		};

		rom* rom_ptr;
		boot_rom* boot_ptr;
		u8 memory[0x10000]; // cover memory maps up to index 0xFFFF

		memory_map_object memory_map[MEMORY_COUNT] = {
			{ "ROM0", &memory[0x0000], 0x0000, 0x3FFF, MEMORY_READABLE },
			{ "ROM1", &memory[0x4000], 0x4000, 0x7FFF, MEMORY_READABLE },
			{ "VRAM", &memory[0x8000], 0x8000, 0x9FFF, MEMORY_READABLE | MEMORY_WRITABLE },
			{ "ERAM", &memory[0xA000], 0xA000, 0xBFFF, MEMORY_READABLE | MEMORY_WRITABLE },
			{ "WRAM", &memory[0xC000], 0xC000, 0xDFFF, MEMORY_READABLE | MEMORY_WRITABLE },
			{ "ECHO", &memory[0xC000], 0xE000, 0xFDFF, MEMORY_READABLE },
			{ " OAM", &memory[0xFE00], 0xFE00, 0xFE9F, MEMORY_READABLE | MEMORY_WRITABLE },
			{ " NOT", &memory[0xFEA0], 0xFEA0, 0xFEFF, 0 },
			{ " IOR", &memory[0xFF00], 0xFF00, 0xFF7F, MEMORY_READABLE | MEMORY_WRITABLE },
			{ "ZERO", &memory[0xFF80], 0xFF80, 0xFFFE, MEMORY_READABLE | MEMORY_WRITABLE },
			{ "INTF", &memory[0xFFFF], 0xFFFF, 0xFFFF, MEMORY_READABLE | MEMORY_WRITABLE },
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
				
		u8* get_memory(u16 addr, bool force = false)
		{
			// loop though memory map
			for (unsigned int i = 0; i < MEMORY_COUNT; i++)
			{
				if (addr <= memory_map[i].addr_max)
				{
					if (!force)
					{
						if ((memory_map[i].access & MEMORY_READABLE) == 0)
						{
							printf("Error - reading from memory map that is not readable: 0x%X\n", addr);
							return 0;
						}
					}

					return &memory_map[i].memory_ptr[addr - memory_map[i].addr_min];
				}
			}

			printf("Error - memory map not implemented for this range of addr: 0x%X\n", addr);
			return 0;
		}

		u8 read_memory(u16 addr, bool force = false)
		{
			if (addr == 0xFF00) // special case for joystick register
			{
				u8 val = memory[0xFF00]; // bits 4 and 5 decide which joystick bits to return (0 - 3)
				val |= 0xF; // for now all input is off
				return val;
			}

			// loop though memory map
			for (unsigned int i = 0; i < MEMORY_COUNT; i++)
			{
				if (addr <= memory_map[i].addr_max)
				{
					if (!force)
					{
						u8 mode = memory[0xFF41] & 0x3; // lcd_status
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

						if ((memory_map[i].access & MEMORY_READABLE) == 0)
						{
							printf("Error - reading from memory map that is not readable: 0x%X\n", addr);
							return 0;
						}
					}

					return memory_map[i].memory_ptr[addr - memory_map[i].addr_min];
				}
			}

			printf("Error - memory map not implemented for this range of addr: 0x%X\n", addr);
			return 0;
		}

		void write_memory(const u16 addr, const u8* value, const u8 size, bool force = false)
		{
			if (addr == 0xFF44) // current scanline. if anyone tries to write to this value we reset to 0
			{
				memory[addr] = 0x0;
				return;
			}
			else if (addr == 0xFF04) // divide register is reset if someone tries to write to it
			{
				memory[addr] = 0x0;
				return;
			}
			else if (addr == 0xFF07) // timer controller. check if frequency has changed and reset timer if so
			{
				u8 timer_controller = memory[addr];

				if ((timer_controller & 0x3) != (*value & 0x3)) // not equal
				{
					memory[0xFF05] = 0x0; // reset timer
					cpu::reset_timer_counter();
				}

				memcpy(&memory[addr], value, size);
				return;
			}
			else if (addr == 0xFF50)
			{
				// unload the boot rom
				memcpy(memory, rom_ptr->romdata, 0x100);
				return;
			}

			// loop though memory map
			for (unsigned int i = 0; i < MEMORY_COUNT; i++)
			{
				if (addr <= memory_map[i].addr_max)
				{
					if (!force)
					{
						u8 mode = memory[0xFF41] & 0x3; // lcd_status
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
						
						if ((memory_map[i].access & MEMORY_WRITABLE) == 0)
						{
							printf("Error - writing to memory map that is not writable: 0x%X\n", addr);
							return;
						}
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

#ifdef NO_BOOTROM
			write_memory(0xFF05, 0x00); // TIMA
			write_memory(0xFF06, 0x00); // TMA
			write_memory(0xFF07, 0x00); // TMC
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
			write_memory(0xFF45, 0x00); // LYC
			write_memory(0xFF47, 0xFC); // BGP
			write_memory(0xFF48, 0xFF); // OBP0
			write_memory(0xFF49, 0xFF); // OBP1
			write_memory(0xFF4A, 0x00); // WY
			write_memory(0xFF4B, 0x00); // WX
			write_memory(0xFFFF, 0x00); // IE
#endif
			
			// memcpy static rom bank
			u64 size = memory_map[MEMORY_CARTRIDGE_ROM + 1].addr_max - 1;
			if (rom_ptr->romsize < size)
			{
				size = rom_ptr->romsize;
			}

			memcpy(memory, rom_ptr->romdata, size);

			// copy boot rom
			if (boot_ptr)
			{
				memcpy(memory, boot_ptr->romdata, 0x100);
			}

			return 0;
		}

		int initialize(boot_rom* boot, rom* rom)
		{
			assert(rom->romsize - 1 <= memory_map[MEMORY_CARTRIDGE_SWITCHABLE_ROM].addr_max);

			boot_ptr = boot;
			rom_ptr = rom;

			reset();

			return 0;
		}
	}
}