#pragma once

#include "defines.h"

namespace gameboy
{
	namespace boot_rom
	{
		u8* rom_data;
		u64 rom_size;
		std::string filename;

		int load(const char* path)
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

			return 0;
		}

		int reset()
		{
			filename = "";
			rom_size = 0x0;

			if (rom_data)
			{
				delete[] rom_data;
				rom_data = nullptr;
			}

			return 0;
		}
	};
}