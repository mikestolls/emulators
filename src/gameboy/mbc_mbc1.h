#pragma once

#include "defines.h"

namespace gameboy
{	
	namespace memory_module
	{
		extern void enable_external_ram(bool enable);
	}

	class mbc_mbc1 : public mbc_base
	{
	public:

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
			memset(memory, 0x0, sizeof(memory));

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
			rom_bank_idx = 0x0;
			memory_rom = rom_banks[0];
			memory_switchable_rom = rom_banks[rom_bank_idx];

			// based on the ram setting. create external ram banks
			switch (ramsize)
			{
			case RAM_NONE:
				ram_bank_idx = 0x0;
				memory_external_ram = nullptr;
				break;
			case RAM_2KB:
			case RAM_8KB:
				// one bank, can just point to 0xA000
				ram_bank_idx = 0x0;
				memory_external_ram = &memory[0xA000];
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
				memory_external_ram = ram_banks[ram_bank_idx];
				break;
			}

			return 0;
		}

		int write_memory(u16 addr, u8 value)
		{
			if (addr < 0x2000)
			{
				// enable external ram
				memory_module::enable_external_ram(true);
			}
			else if (addr < 0x4000)
			{
				// set the lower 5 bits of rom bank
				rom_bank_idx &= 0xE0; // clear lower 5 bits
				rom_bank_idx |= (value & 0x1F); // and in the new lower 5 bits

				if (rom_bank_idx == 0x0)
				{
					rom_bank_idx = 0x1; // if set to 0, force it to 1
				}

				memory_switchable_rom = rom_banks[rom_bank_idx];
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

					if (rom_bank_idx == 0x0)
					{
						rom_bank_idx = 0x1; // if set to 0, force it to 1
					}

					memory_switchable_rom = rom_banks[rom_bank_idx];
				}
				else
				{
					// two bits are the ram bank
					ram_bank_idx = bits;

					memory_external_ram = ram_banks[ram_bank_idx];
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
						ram_bank_idx = 0x0; // only use ram bank 0 during this mode
					}
					else
					{
						// in RAM mode only ROM banks 0x0 - 0x1F can be used
					}

					mode_select = mode;
				}
			}

			return 0;
		}

		mbc_mbc1() : mbc_base()
		{

		}

		~mbc_mbc1()
		{
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
		}
	};
}