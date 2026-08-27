#pragma once

#include "defines.h"
#include "rom.h"
#include "boot_rom.h"

#include <cstdarg>

//#include "mbc_base.h"

#define FORCE_VRAM_OAM

namespace gameboy
{
	namespace cpu
	{
		void reset_timer_counter();
		int update_timer(u8 cycles);
		u16 get_current_pc();
		void set_double_speed(bool double_speed);
	}

	namespace apu
	{
		int trigger_channel1();
		int trigger_channel2();
	}

	namespace gpu
	{
		u8 get_lcd_status_mode();
		u16 get_dots();
		u8 get_scanline();
	}

	namespace input
	{
		u8 get_button_register(bool is_directional);
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
#ifdef FORCE_VRAM_OAM
					if (i == MEMORY_VRAM)
					{
						force = true;
					}
#endif

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
			if (addr == 0xFF00) // special handle for joypad
			{
				u8 mem = mbc::memory[0xFF00];
				u8 result = 0xCF; // bit 6 and 7 are always 1

				result |= (mem & 0x30); // bring selection bits 4 and 5

				// check which button set is selected
				if ((mem & 0x20) == 0) // bit 5 = 0 - select action buttons
				{
					result &= 0xF0; // clears lower bits
					result |= (input::get_button_register(false) & 0x0F); // add input button lower bits
				}
				else if ((mem & 0x10) == 0) // bit 4 = 0 - select direction buttons
				{
					result &= 0xF0; // clears lower bits
					result |= (input::get_button_register(true) & 0x0F); // add input directional lower bits
				}
				else
				{
					// nothing select bit 0 - 3 are high
					result |= 0x0F;
				}

				return result;
			}

			// loop though memory map
			for (unsigned int i = 0; i < MEMORY_COUNT; i++)
			{
				if (addr <= memory_map[i].addr_max)
				{
#ifdef FORCE_VRAM_OAM
					if (i == MEMORY_VRAM)
					{
						force = true;
					}
#endif

					if (!force)
					{
						if ((memory_map[i].access & MEMORY_READABLE) == 0)
						{
							printf("Warning - reading from memory map %d that is not readable: 0x%X LCD Mode: %d Dot: %d Scanline: %d PC: 0x%X\n",
								i, addr, gpu::get_lcd_status_mode(), gpu::get_dots(), gpu::get_scanline(), cpu::get_current_pc());
							return 0;
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

			if (addr == 0xFF00) // joypad registry
			{
				// only bits 4 and 5 are writable
				u8 cur_value = mbc::memory[0xFF00];
				u8 masked_value = (cur_value & 0x0F) | (*value & 0x30) | 0xC0; // bits 6 and 7 are always high, take value bits 4 and 5 to write
				mbc::memory[0xFF00] = masked_value;
			}
			else if (addr == 0xFF02 && (*value & 0x80)) // Serial transfer control - bit 7 set = transfer
			{
				u8 serial_data = mbc::memory[0xFF01]; // read serial data register
				printf("%c", serial_data); // print char
				fflush(stdout); // flush immediately

				// clear transfer flag after transfer
				mbc::memory[0xFF02] = *value & 0x7F;
				return;
			}
			else if (addr == 0xFF44) // current scanline. if anyone tries to write to this value we reset to 0
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
			else if (addr == 0xFF4D)
			{
				// speed switch
				bool is_double_speed = (*value & 0x80) != 0;
				cpu::set_double_speed(is_double_speed);

				if (is_double_speed)
				{
					printf("Double Speed Enabled - PC: 0x%X\n", cpu::get_current_pc());
				}
				else
				{
					printf("Double Speed Disabled - PC: 0x%X\n", cpu::get_current_pc());
				}
			}
			else if (addr == 0xFF50)
			{
				// unload the boot rom
				memcpy(mbc::memory_rom, rom_ptr->rom_data, 0x100);
				return;
			}
			else if (addr == 0xFF46)
			{
				// transfer OAM data
				u16 src_addr = *value;
				src_addr *= 0x100;
				memcpy(&mbc::memory[0xFE00], &mbc::memory[src_addr], 0x9F);
			}
			else if (addr >= 0xFF10 && addr <= 0xFF26)
			{
				// APU registers - handle special cases
				if (addr == 0xFF14 && (*value & 0x80)) // NR14 - Channel 1 trigger
				{
					// Trigger channel 1
					mbc::memory[0xFF26] |= 0x01;  // Set channel 1 active in NR52

					apu::trigger_channel1();

					// Write the value (but clear trigger bit as it's write-only)
					u8 write_val = *value & 0x7F;  // Clear bit 7 after trigger
					memcpy(&mbc::memory[0xFF14], &write_val, size);
				}
				else if (addr == 0xFF19 && (*value & 0x80)) // NR24 - Channel 2 trigger
				{
					// Trigger channel 1
					mbc::memory[0xFF26] |= 0x02;  // Set channel 2 active in NR52

					apu::trigger_channel2();

					// Write the value (but clear trigger bit as it's write-only)
					u8 write_val = *value & 0x7F;  // Clear bit 7 after trigger
					memcpy(&mbc::memory[0xFF19], &write_val, size);
				}
				else if (addr == 0xFF26) // NR52 - Sound on/off
				{
					if (!(*value & 0x80))
					{
						// Power off - clear all sound registers except NR52 bit 7
						for (u16 a = 0xFF10; a <= 0xFF25; a++)
						{
							mbc::memory[a] = 0;
						}
						mbc::memory[0xFF26] = *value & 0x80;  // Keep only power bit
					}
					else
					{
						memcpy(&mbc::memory[addr], value, size);
					}
				}
				else
				{
					// Normal write for other APU registers
					memcpy(&mbc::memory[addr], value, size);
				}

				return;
			}

			// loop though memory map
			for (unsigned int i = 0; i < MEMORY_COUNT; i++)
			{
				if (addr <= memory_map[i].addr_max)
				{
#ifdef FORCE_VRAM_OAM
					if (i == MEMORY_VRAM)
					{
						if ((memory_map[i].access & MEMORY_WRITABLE) == 0)
						{
							printf("Warning - writing to memory map %d that is not writable: 0x%X LCD Mode: %d Dot: %d Scanline: %d PC: 0x%X\n",
								i, addr, gpu::get_lcd_status_mode(), gpu::get_dots(), gpu::get_scanline(), cpu::get_current_pc());
						}

						force = true;
					}
#endif

					if (!force)
					{						
						if ((memory_map[i].access & MEMORY_WRITABLE) == 0)
						{
							printf("Warning - writing to memory map %d that is not writable: 0x%X LCD Mode: %d Dot: %d Scanline: %d PC: 0x%X\n",
								i, addr, gpu::get_lcd_status_mode(), gpu::get_dots(), gpu::get_scanline(), cpu::get_current_pc());
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
			mbc::mbc_initialize(rom_ptr->rom_header.rom_size, rom_ptr->rom_header.ram_size, rom_ptr->rom_data, (u64)rom_ptr->rom_size);

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

			bool is_cgb_mode = (rom_ptr->rom_header.cgb_flag == 0x80 || rom_ptr->rom_header.cgb_flag == 0xC0);

			// default key1 depending on DMG or CGB
			u8 key1 = 0xFF;
			if (is_cgb_mode)
			{
				key1 = 0x7E;
			}

			// copy boot rom
			if (boot_ptr)
			{
				memcpy(mbc::memory_rom, boot_ptr->rom_data, 0x100);

				write_memory(0xFF4D, key1); // KEY1
				write_memory(0xFF41, is_cgb_mode ? 0x04 : 0x84); // LCDS
			}
			else
			{
				// no boot rom set default mem values
				write_memory(0xFF00, 0x30); // JOYPAD
				write_memory(0xFF05, 0x00); // TIMA
				write_memory(0xFF06, 0x00); // TMA
				write_memory(0xFF07, 0x00); // TMC
				write_memory(0xFF0F, 0xF1); // IF
				write_memory(0xFF10, 0x80); // NR10
				write_memory(0xFF11, 0xBF); // NR11
				write_memory(0xFF12, 0xF3); // NR12
				write_memory(0xFF14, 0x3F); // NR14
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
				write_memory(0xFF41, is_cgb_mode ? 0x05 : 0x85); // LCDS
				write_memory(0xFF42, 0x00); // SCY
				write_memory(0xFF43, 0x00); // SCX
				write_memory(0xFF45, 0x00); // LYC
				write_memory(0xFF47, 0xFC); // BGP
				write_memory(0xFF48, 0xFF); // OBP0
				write_memory(0xFF49, 0xFF); // OBP1
				write_memory(0xFF4A, 0x00); // WY
				write_memory(0xFF4B, 0x00); // WX
				write_memory(0xFF4D, key1); // KEY1
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