#pragma once

#include "defines.h"

#include "mbc.h"
#include "mbc_mbc1.h"

namespace gameboy
{
	std::string rom_extension = ".gb";

	struct rom
	{
		struct header
		{
			u8 entry_point[4];
			u8 nintendo_character_area[48];
			u8 game_title[16];
			CATRIDGE_TYPE cartridge_type;
			ROM_SIZE rom_size;
			RAM_SIZE ram_size;
			u8 version;
			u8 cgb_flag;
		};

		u8* rom_data;
		u64 rom_size;
		std::string filename;
		header rom_header;

		rom()
		{
			filename = "";
			rom_size = 0x0;
			rom_data = nullptr;
			memset(&rom_header, 0x0, sizeof(rom_header));
		}

		rom(const std::string& filename)
		{
			load(filename);
		}

		void load(const std::string& filename)
		{
			FILE* file = 0;
			fopen_s(&file, filename.c_str(), "rb");

			if (!file)
			{
				printf("Error - Failed to open ROM file: %s\n", filename.c_str());
				return;
			}

			// get size
			fseek(file, 0, SEEK_END);
			rom_size = ftell(file);
			fseek(file, 0, SEEK_SET);

			// read header
			rom_data = new u8[rom_size];
			size_t size = fread(rom_data, 1, rom_size, file);

			assert(size == rom_size);

			fclose(file);

			// copy to header for reference. header starts at 0x100 of the ROM
			memset(&rom_header, 0x0, sizeof(rom_header));
			memcpy(rom_header.entry_point, &rom_data[0x100], sizeof(rom_header.entry_point));
			memcpy(rom_header.nintendo_character_area, &rom_data[0x104], sizeof(rom_header.nintendo_character_area));
			memcpy(rom_header.game_title, &rom_data[0x134], sizeof(rom_header.game_title));
			rom_header.cartridge_type = (CATRIDGE_TYPE)rom_data[0x147];
			rom_header.rom_size = (ROM_SIZE)rom_data[0x148];
			rom_header.ram_size = (RAM_SIZE)rom_data[0x149];
			rom_header.version = rom_data[0x14C];
			rom_header.cgb_flag = rom_data[0x143];

			switch (rom_header.cartridge_type)
			{
			case ROM_ONLY:
				break;
			case ROM_MBC1:
			case ROM_MBC1_RAM:
			case ROM_MBC1_RAM_BATTERY:
				mbc::mbc_initialize = &mbc_mbc1::initialize;
				mbc::mbc_get_rom_bank_idx = &mbc_mbc1::get_rom_bank_idx;
				mbc::mbc_reset = &mbc_mbc1::reset;
				mbc::mbc_write_memory = &mbc_mbc1::write_memory;
				break;
			default:
				warning_assert("memory bank controller not supported yet");
				break;
			}
		}

		~rom()
		{
			if (rom_data)
			{
				delete[] rom_data;
			}
		}
	};
}