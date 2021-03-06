#pragma once

#include "defines.h"
#include "rom.h"
#include "boot_rom.h"

#include <cstdarg>

//#include "mbc_base.h"

namespace gameboy
{
	namespace cpu
	{
		void reset_timer_counter();
		int update_timer(u8 cycles);
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
			u8** memory_ptr;
			u16 addr_min;
			u16 addr_max;
			u8 access;
		};

		rom* rom_ptr;
		boot_rom* boot_ptr;
		
		memory_map_object memory_map[MEMORY_COUNT] = {
			{ "ROM0", nullptr, 0x0000, 0x3FFF, MEMORY_READABLE },
			{ "ROMS", nullptr, 0x4000, 0x7FFF, MEMORY_READABLE },
			{ "VRAM", nullptr, 0x8000, 0x9FFF, MEMORY_READABLE | MEMORY_WRITABLE },
			{ "ERAM", nullptr, 0xA000, 0xBFFF, 0 },
			{ "WRAM", nullptr, 0xC000, 0xDFFF, MEMORY_READABLE | MEMORY_WRITABLE },
			{ "ECHO", nullptr, 0xE000, 0xFDFF, MEMORY_READABLE },
			{ " OAM", nullptr, 0xFE00, 0xFE9F, MEMORY_READABLE | MEMORY_WRITABLE },
			{ " NOT", nullptr, 0xFEA0, 0xFEFF, 0 },
			{ " IOR", nullptr, 0xFF00, 0xFF7F, MEMORY_READABLE | MEMORY_WRITABLE },
			{ "ZERO", nullptr, 0xFF80, 0xFFFE, MEMORY_READABLE | MEMORY_WRITABLE },
			{ "INTF", nullptr, 0xFFFF, 0xFFFF, MEMORY_READABLE | MEMORY_WRITABLE },
		};

		void enable_external_ram(bool enable)
		{
			memory_map[MEMORY_EXTERNAL_RAM].access = (enable ? MEMORY_READABLE | MEMORY_WRITABLE : 0);
		}

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

		inline void set_memory_access(u8 bank, u8 access) { memory_map[bank].access = access; }
		inline u8 get_memory_access(u8 bank, u8 access) { return memory_map[bank].access; }

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
					if (!force)
					{
						if ((memory_map[i].access & MEMORY_READABLE) == 0)
						{
							print_warning("Warning - reading from memory map that is not readable: 0x%X\n", addr);
							return 0;
						}
					}
					
					if (memory_map[i].memory_ptr == nullptr)
					{
						return 0;
					}

					return &(*memory_map[i].memory_ptr)[addr - memory_map[i].addr_min];
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
					if (!force)
					{
						if ((memory_map[i].access & MEMORY_READABLE) == 0)
						{
							print_warning("Warning - reading from memory map that is not readable: 0x%X\n", addr);
							return 0xFF;
						}
					}

					if (memory_map[i].memory_ptr == nullptr || *memory_map[i].memory_ptr == nullptr)
					{
						return 0;
					}

					return (*memory_map[i].memory_ptr)[addr - memory_map[i].addr_min];
				}
			}

			printf("Error - memory map not implemented for this range of addr: 0x%X\n", addr);
			return 0;
		}

		void write_memory(const u16 addr, const u8* value, const u8 size, bool force = false)
		{
			if (mbc::mbc_write_memory(addr, *value)) // if memory controller handles addr, return here
			{
				return;
			}

			if (addr == 0xFF44) // current scanline. if anyone tries to write to this value we reset to 0
			{
				mbc::memory[addr] = 0x0;
				return;
			}
			else if (addr == 0xFF04) // divide register is reset if someone tries to write to it
			{
				mbc::memory[addr] = 0x0;
				return;
			}
			else if (addr == 0xFF07) // timer controller. check if frequency has changed and reset timer if so
			{
				u8 timer_controller = mbc::memory[addr];
				memcpy(&mbc::memory[addr], value, size);

				if ((timer_controller & 0x3) != (*value & 0x3)) // not equal
				{
					cpu::reset_timer_counter(); // reset timer
				}
				return;
			}
			else if (addr == 0xFF50)
			{
				// unload the boot rom
				memcpy(mbc::memory_rom, rom_ptr->romdata, 0x100);
				return;
			}
			else if (addr == 0xFF46)
			{
				// transfer OAM data
				u16 src_addr = *value;
				src_addr *= 0x100;
				memcpy(&mbc::memory[0xFE00], &mbc::memory[src_addr], 0x9F);
			}

			// loop though memory map
			for (unsigned int i = 0; i < MEMORY_COUNT; i++)
			{
				if (addr <= memory_map[i].addr_max)
				{
					if (!force)
					{						
						if ((memory_map[i].access & MEMORY_WRITABLE) == 0)
						{
							print_warning("Warning - writing to memory map that is not writable: 0x%X map: %d\n", addr, i);
							return;
						}
					}

					if (memory_map[i].memory_ptr == nullptr)
					{
						return;
					}

					memcpy(&(*memory_map[i].memory_ptr)[addr - memory_map[i].addr_min], value, size);

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
			mbc::mbc_reset();
			mbc::mbc_initialize(rom_ptr->romheader.romSize, rom_ptr->romheader.ramSize, rom_ptr->romdata, (u64)rom_ptr->romsize);

			memory_map[MEMORY_CARTRIDGE_ROM].memory_ptr = &mbc::memory_rom;
			memory_map[MEMORY_CARTRIDGE_SWITCHABLE_ROM].memory_ptr = &mbc::memory_switchable_rom;
			memory_map[MEMORY_VRAM].memory_ptr = &mbc::memory_vram;
			memory_map[MEMORY_EXTERNAL_RAM].memory_ptr = &mbc::memory_external_ram;
			memory_map[MEMORY_WORKING_RAM].memory_ptr = &mbc::memory_working_ram;
			memory_map[MEMORY_ECHO_RAM].memory_ptr = &mbc::memory_working_ram;
			memory_map[MEMORY_OAM].memory_ptr = &mbc::memory_oam;
			memory_map[MEMORY_NOTUSED].memory_ptr = nullptr;
			memory_map[MEMORY_IO_REGISTERS].memory_ptr = &mbc::memory_io_registers;
			memory_map[MEMORY_ZERO_PAGE].memory_ptr = &mbc::memory_zero_page;
			memory_map[MEMORY_INTERRUPT_FLAG].memory_ptr = &mbc::memory_interrupt_flag;

			// copy boot rom
			if (boot_ptr)
			{
				memcpy(mbc::memory_rom, boot_ptr->romdata, 0x100);

				write_memory(0xFF4D, 0xFF); // KEY1 - CGB only
				write_memory(0xFF41, 0x84); // LCDS
			}
			else
			{
				// no boot rom set default mem values
				write_memory(0xFF00, 0x30); // JOYPAD
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
				write_memory(0xFF41, 0x85); // LCDS
				write_memory(0xFF42, 0x00); // SCY
				write_memory(0xFF43, 0x00); // SCX
				write_memory(0xFF45, 0x00); // LYC
				write_memory(0xFF47, 0xFC); // BGP
				write_memory(0xFF48, 0xFF); // OBP0
				write_memory(0xFF49, 0xFF); // OBP1
				write_memory(0xFF4A, 0x00); // WY
				write_memory(0xFF4B, 0x00); // WX
				write_memory(0xFF4D, 0xFF); // KEY1 - CGB only
				write_memory(0xFFFF, 0x00); // IE
			}

			return 0;
		}

		int initialize(boot_rom* boot, rom* rom)
		{
			boot_ptr = boot;
			rom_ptr = rom;

			reset();

			return 0;
		}
	}
}