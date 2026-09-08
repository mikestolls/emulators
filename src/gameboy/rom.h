#pragma once

#include "defines.h"

namespace gameboy
{
	namespace rom
	{
		std::string rom_extension = ".gb";

		enum CATRIDGE_TYPE
		{
			ROM_ONLY = 0,
			ROM_MBC1,
			ROM_MBC1_RAM,
			ROM_MBC1_RAM_BATTERY,
			ROM_MBC2,
			ROM_MBC2_BATTERY,
		};

		enum ROM_SIZE
		{
			ROM_32KB = 0,
			ROM_64KB,
			ROM_128KB,
			ROM_256KB,
			ROM_512KB,
			ROM_1MB,
			ROM_2MB,
			ROM_4MB,
		};

		enum RAM_SIZE
		{
			RAM_NONE = 0,
			RAM_2KB,
			RAM_8KB,
			RAM_32KB,
		};

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

		int reset ()
		{
			filename = "";
			rom_size = 0x0;
			memset(&rom_header, 0x0, sizeof(rom_header));

			if (rom_data)
			{
				delete[] rom_data;
				rom_data = nullptr;
			}

			return 0;
		}

		int load(const std::string& filename)
		{
			reset();

			FILE* file = 0;
			fopen_s(&file, filename.c_str(), "rb");

			if (!file)
			{
				printf("Error - Failed to open ROM file: %s\n", filename.c_str());
				return -1;
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
		}
	};
}