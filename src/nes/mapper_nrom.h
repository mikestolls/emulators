#pragma once

#include "defines.h"
#include "mapper.h"

#include "cpu_memory_module.h"

namespace nes
{	
	namespace mapper_nrom
	{
		int initialize()
		{
			// copy in the rom data
			assert(rom::prg_size <= 0x8000);

			memcpy(nes::cpu_memory_module::memory_map[nes::cpu_memory_module::MEMORY_PRG_ROM].memory_ptr, rom::prg_data, rom::prg_size); // copy the rom to the memory map

			// need to move this into a mapper 0 struct. which is no mem mapping

			u32 count = 0;
			while (count < 0x8000)
			{
				u32 size = (count + rom::prg_size) < 0x8000 ? rom::prg_size : 0x8000 - rom::prg_size;
				memcpy(&nes::cpu_memory_module::memory_map[nes::cpu_memory_module::MEMORY_PRG_ROM].memory_ptr[count], rom::prg_data, size);
				count += size;
			}

			return 0;
		}

		int reset()
		{
			return 0;
		}
	};
}