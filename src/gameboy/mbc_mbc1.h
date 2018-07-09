#pragma once

#include "defines.h"
#include "mbc.h"

namespace gameboy
{	
	namespace memory_module
	{
		extern void enable_external_ram(bool enable);
	}

	namespace mbc_mbc1
	{
		enum MODE_SELECT
		{
			MODE_ROM_BANK = 0,
			MODE_RAM_BANK
		};

		MODE_SELECT mode_select;
		u8 rom_bank_idx;
		u8 ram_bank_idx;
		std::vector<u8*> rom_banks;
		std::vector<u8*> ram_banks;

		int initialize(ROM_SIZE romsize, RAM_SIZE ramsize, u8* romdata, u64 datasize)
		{
			mbc::memory_rom = &mbc::memory[0x0000];
			mbc::memory_switchable_rom = &mbc::memory[0x4000];
			mbc::memory_vram = &mbc::memory[0x8000];
			mbc::memory_external_ram = &mbc::memory[0xA000];
			mbc::memory_working_ram = &mbc::memory[0xC000];
			mbc::memory_oam = &mbc::memory[0xFE00];
			mbc::memory_io_registers = &mbc::memory[0xFF00];
			mbc::memory_zero_page = &mbc::memory[0xFF80];
			mbc::memory_interrupt_flag = &mbc::memory[0xFFFF];

			u32 banksize = 0x4000;

			// copy in the static rom bank
			u8* rom_ptr = romdata;

			// based on the size, create and copy the rom banks
			u64 num_banks = datasize / banksize;
			for (u32 i = 0; i < num_banks; i++)
			{
				u8* bank = new u8[banksize];
				memcpy(bank, rom_ptr, banksize);
				rom_banks.push_back(bank);

				rom_ptr += banksize;
			}

			// point rom to default bank
			mode_select = MODE_ROM_BANK;
			rom_bank_idx = 0x1;
			mbc::memory_rom = rom_banks[0x0];
			mbc::memory_switchable_rom = rom_banks[0x1];

			// based on the ram setting. create external ram banks
			switch (ramsize)
			{
			case RAM_NONE:
			case RAM_2KB:
			case RAM_8KB:
				ram_bank_idx = 0x0;
				mbc::memory_external_ram = &mbc::memory[0xA000];
				break;
			default:
				banksize = 0x2000;
				for (u32 i = 0; i < 4; i++)
				{
					u8* bank = new u8[banksize];
					memcpy(bank, 0x0, banksize);
					ram_banks.push_back(bank);
				}

				ram_bank_idx = 0x0;
				mbc::memory_external_ram = ram_banks[ram_bank_idx];
				break;
			}

			return 0;
		}

		int reset()
		{
			mbc::reset();

			while (!rom_banks.empty())
			{
				u8* temp = rom_banks.back();
				rom_banks.pop_back();
				delete[] temp;
			}

			while (!ram_banks.empty())
			{
				u8* temp = ram_banks.back();
				ram_banks.pop_back();
				delete[] temp;
			}

			return 0;
		}

		bool write_memory(u16 addr, u8 value)
		{
			bool handled = false;

			if (addr < 0x2000)
			{
				u8 val = value & 0xF;

				if (val == 0xA)
				{
					// enable external ram
					memory_module::enable_external_ram(true);
				}
				else
				{
					memory_module::enable_external_ram(false);
				}

				handled = true;
			}
			else if (addr < 0x4000)
			{
				// set the lower 5 bits of rom bank
				u8 val = value & 0x1F;
				if (val == 0x0)
				{
					val = 0x1; // cant be 0
				}
				
				rom_bank_idx &= 0xE0; // clear lower 5 bits
				rom_bank_idx |= val;
				
				//printf("Rom bank: %d\n", rom_bank_idx);
				mbc::memory_switchable_rom = rom_banks[rom_bank_idx];

				handled = true;
			}
			else if (addr < 0x6000)
			{
				u8 bits = value & 0x3;
				// set the ram bank number
				if (mode_select == MODE_ROM_BANK)
				{
					// 2 bits are bits 5 and 6 or rom bank
					rom_bank_idx &= 0x1F; // clear high 3 bits
					rom_bank_idx |= (bits << 5);

					//printf("Rom bank: %d\n", rom_bank_idx);
					mbc::memory_switchable_rom = rom_banks[rom_bank_idx];

					handled = true;
				}
				else
				{
					// two bits are the ram bank
					ram_bank_idx = bits;

					mbc::memory_external_ram = ram_banks[ram_bank_idx];

					handled = true;
				}
			}
			else if (addr < 0x8000)
			{
				// set the rom/ram mode
				MODE_SELECT mode = (MODE_SELECT)(value & 0x1);

				if (mode != mode_select) // switching modes
				{
					if (mode == MODE_ROM_BANK)
					{
						mbc::memory_external_ram = ram_banks[0x0];
					}
					else
					{
						// in RAM mode only ROM banks 0x0 - 0x1F can be used
						mbc::memory_external_ram = ram_banks[ram_bank_idx];
					}

					mode_select = mode;
				}

				handled = true;
			}

			return handled;
		}

		int get_rom_bank_idx()
		{
			return rom_bank_idx;
		}
	};
}