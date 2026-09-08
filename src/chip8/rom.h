#pragma once

#include "defines.h"

namespace chip8
{
	namespace rom
	{
		std::string rom_extension = ".ch8";

		u8* rom_data;
		u64 rom_size;
		std::string filename;

		int reset()
		{
			filename = "";
			rom_size = 0x0;

			if (rom_data != nullptr)
			{
				delete[] rom_data;
				rom_data = nullptr;
			}

			return 0;
		}

		int load(const std::string& filename)
		{
			reset();

			rom_size = 0;
			rom_data = nullptr;

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
			fread(rom_data, 1, rom_size, file);

			fclose(file);

			return 0;
		}
	};
}