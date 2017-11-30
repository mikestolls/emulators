#pragma once

#include "defines.h"

namespace gameboy
{
	struct boot_rom
	{
		u8* romdata;
		u64 romsize;
		std::string filename;

		void open(const char* path)
		{
			filename = path;

			FILE* file = 0;
			fopen_s(&file, filename.c_str(), "rb");

			// get size
			fseek(file, 0, SEEK_END);
			romsize = ftell(file);
			fseek(file, 0, SEEK_SET);

			// read header
			romdata = new u8[romsize];
			size_t size = fread(romdata, 1, romsize, file);

			assert(size == 0x100);

			fclose(file);
		}

		boot_rom()
		{
			filename = "";
			romsize = 0x0;
			romdata = nullptr;
		}

		boot_rom(const char* path)
		{
			open(path);
		}

		~boot_rom()
		{
			if (romdata)
			{
				delete[] romdata;
			}
		}
	};
}