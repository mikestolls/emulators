#pragma once

#include "defines.h"

namespace chip8
{
	class rom
	{
	public:

		u8* romdata;
		u64 romsize;
		std::string filename;

		rom(const char* path)
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
			fread(romdata, 1, romsize, file);

			fclose(file);
		}

		~rom()
		{
			delete[] romdata;
		}
	};
}