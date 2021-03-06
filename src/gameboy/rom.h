#pragma once

#include "defines.h"

#include "mbc.h"
#include "mbc_mbc1.h"

namespace gameboy
{
	struct rom
	{
		struct rom_header
		{
			u8 entryPoint[4];
			u8 nintendoCharacterArea[48];
			u8 gameTitle[16];
			CATRIDGE_TYPE cartridgeType;
			ROM_SIZE romSize;
			RAM_SIZE ramSize;
			u8 version;
			u8 cgbFlag;
		};


		u8* romdata;
		u64 romsize;
		std::string filename;
		rom_header romheader;

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

			assert(size == romsize);

			fclose(file);

			// copy to header for reference. header starts at 0x100 of the ROM
			memset(&romheader, 0x0, sizeof(rom_header));
			memcpy(romheader.entryPoint, &romdata[0x100], sizeof(romheader.entryPoint));
			memcpy(romheader.nintendoCharacterArea, &romdata[0x104], sizeof(romheader.nintendoCharacterArea));
			memcpy(romheader.gameTitle, &romdata[0x134], sizeof(romheader.gameTitle));
			romheader.cartridgeType = (CATRIDGE_TYPE)romdata[0x147];
			romheader.romSize = (ROM_SIZE)romdata[0x148];
			romheader.ramSize = (RAM_SIZE)romdata[0x149];
			romheader.version = romdata[0x14C];
			romheader.cgbFlag = romdata[0x143];

			switch (romheader.cartridgeType)
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

		rom()
		{
			filename = "";
			romsize = 0x0;
			romdata = nullptr;
			memset(&romheader, 0x0, sizeof(rom_header));
		}

		rom(const char* path)
		{
			open(path);
		}

		~rom()
		{
			if (romdata)
			{
				delete[] romdata;
			}
		}
	};
}