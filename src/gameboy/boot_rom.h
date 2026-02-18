#pragma once

#include "defines.h"

namespace gameboy
{
	struct boot_rom
	{
		u8* rom_data;
		u64 rom_size;
		std::string filename;

		void open(const char* path)
		{
			filename = path;

			FILE* file = 0;
			fopen_s(&file, filename.c_str(), "rb");

			// get size
			fseek(file, 0, SEEK_END);
			rom_size = ftell(file);
			fseek(file, 0, SEEK_SET);

			// read header
			rom_data = new u8[rom_size];
			size_t size = fread(rom_data, 1, rom_size, file);

			assert(size == 0x100);

			fclose(file);
		}

		boot_rom()
		{
			filename = "";
			rom_size = 0x0;
			rom_data = nullptr;
		}

		boot_rom(const char* path)
		{
			open(path);
		}

		~boot_rom()
		{
			if (rom_data)
			{
				delete[] rom_data;
			}
		}
	};
}