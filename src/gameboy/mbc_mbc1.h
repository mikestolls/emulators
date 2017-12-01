#pragma once

#include "defines.h"

namespace gameboy
{	
	class mbc_mbc1 : public mbc_base
	{
	public:

		std::vector<u8*> rom_banks;

		int initialize(ROM_SIZE romsize, RAM_SIZE ramsize, u8* romdata, u64 datasize)
		{
			memset(memory, 0x0, sizeof(memory));

			u32 banksize = 0x4000;

			// copy in the static rom bank
			u8* rom_ptr = romdata;
			memcpy(memory, romdata, banksize);

			rom_ptr += banksize;

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
			memory_switchable_rom = rom_banks[0];
			
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
		}
	};
}