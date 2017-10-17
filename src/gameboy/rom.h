#pragma once

#include "defines.h"

namespace gameboy
{
	struct rom
	{
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
			ROM_256KB = 0,
			ROM_512KB,
			ROM_1MB,
			ROM_2MB,
			ROM_4MB,
		};

		enum RAM_SIZE
		{
			RAM_NONE = 0,
			RAM_16KB,
			RAM_64KB,
			RAM_256KB,
		};


		struct rom_header
		{
			u8 entryPoint[4];
			u8 nintendoCharacterArea[48];
			u8 gameTitle[16];
			CATRIDGE_TYPE cartridgeType;
			ROM_SIZE romSize;
			RAM_SIZE ramSize;
			u8 makerCode[2];
			u8 version;
			u8 complementCheck;
			u8 checksum[2];
		};


		u8* romdata;
		u64 romsize;
		std::string filename;
		rom_header romheader;

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

			// copy to header for reference. header starts at 0x100 of the ROM
			memset(&romheader, 0x0, sizeof(rom_header));
			memcpy(romheader.entryPoint, &romdata[0x100], sizeof(romheader.entryPoint));
			memcpy(romheader.nintendoCharacterArea, &romdata[0x104], sizeof(romheader.nintendoCharacterArea));
			memcpy(romheader.gameTitle, &romdata[0x134], sizeof(romheader.gameTitle));
			romheader.cartridgeType = (CATRIDGE_TYPE)romdata[0x144];
			romheader.romSize = (ROM_SIZE)romdata[0x147];
			romheader.ramSize = (RAM_SIZE)romdata[0x148];
			memcpy(romheader.makerCode, &romdata[0x149], sizeof(romheader.makerCode));
			romheader.version = romdata[0x14A];
			romheader.complementCheck = romdata[0x14C];
			memcpy(romheader.checksum, &romdata[0x14D], sizeof(romheader.checksum));
		}

		~rom()
		{
			delete[] romdata;
		}
	};
}