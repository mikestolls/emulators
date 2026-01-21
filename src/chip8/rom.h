#pragma once

#include "defines.h"

namespace chip8
{
	struct rom
	{
		u8* romdata;
		u64 romsize;
		std::string filename;

		rom()
		{

		}

		rom(std::string& filename)
		{
			load(filename);
		}

		int load(std::string& filename)
		{
			romsize = 0;
			romdata = nullptr;

			FILE* file = 0;
			fopen_s(&file, filename.c_str(), "rb");

			if (!file)
			{
				printf("Error - Failed to open ROM file: %s\n", filename.c_str());
				return -1;
			}

			// get size
			fseek(file, 0, SEEK_END);
			romsize = ftell(file);
			fseek(file, 0, SEEK_SET);

			// read header
			romdata = new u8[romsize];
			fread(romdata, 1, romsize, file);

			fclose(file);

			return 0;
		}

		~rom()
		{
			delete[] romdata;
		}
	};
}