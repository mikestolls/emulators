#pragma once

#include "defines.h"

namespace nes
{
	std::string rom_extension = ".nes";

	struct rom
	{
		struct header
		{
			u8 constant[4];
			u8 prg_rom;
			u8 chr_rom;
			u8 flag_6;
			u8 flag_7;
			u8 padding[8];
		};

		u8* rom_data;
		u64 rom_size;
		std::string filename;
		header rom_header;

		u8* prg_data;
		u32 prg_size;
		u8* chr_data;
		u32 chr_size;

		rom()
		{
			filename = "";
			rom_size = 0x0;
			rom_data = nullptr;
			memset(&rom_header, 0x0, sizeof(rom_header));
			prg_data = nullptr;
			chr_data = nullptr;
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

			// copy to header for reference
			memset(&rom_header, 0x0, sizeof(rom_header));
			memcpy(&rom_header.constant, &rom_data[0x0], sizeof(rom_header.constant));
			rom_header.prg_rom = rom_data[0x4];
			rom_header.chr_rom = rom_data[0x5];
			rom_header.flag_6 = rom_data[0x6];
			rom_header.flag_7 = rom_data[0x7];
			memcpy(rom_header.padding, &rom_data[0x8], sizeof(rom_header.padding));

			// check the first magic chars
			assert(strncmp((char*)&rom_header.constant, "NES\x1a", 4) == 0);

			// check for trainer bit
			u8* data = &rom_data[0x10];
			if (rom_header.flag_6 & 0x4)
			{
				data += 512; // trainer is 512 bytes
			}

			// save ptr to the prg data
			prg_size = rom_header.prg_rom * 16384;
			prg_data = data;

			data += prg_size;

			// save ptr to the chr data if present
			chr_size = rom_header.chr_rom * 8192;
			chr_data = nullptr;
			if (rom_header.chr_rom > 0)
			{
				chr_data = data;
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