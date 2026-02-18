#pragma once

#include "defines.h"

#include "memory_module.h"

namespace gameboy
{
	namespace apu
	{
		bool is_initialized = false;

		u32 sound_length_timer = 0;

		// channel 1
		u8 channel1_length = 0;
		u8* nr11;
		u8* nr14;

		// channel 2
		u8 channel2_length = 0;
		u8* nr21;
		u8* nr24;

		// common
		u8* nr52;

		int reset()
		{
			is_initialized = true;

			sound_length_timer = 0;
			channel1_length = 0;
			channel2_length = 0;

			// channel 1
			nr11 = memory_module::get_memory(0xFF11);
			nr14 = memory_module::get_memory(0xFF14);

			// channel 2
			nr21 = memory_module::get_memory(0xFF16);
			nr24 = memory_module::get_memory(0xFF19);

			// status
			nr52 = memory_module::get_memory(0xFF26);

			return 0;
		}

		int initialize()
		{
			reset();

			return 0;
		}

		// currently only impementing apu to pass interrupt time test
		int update(u8 cycles)
		{
			sound_length_timer += cycles;

			// sound length is clocked at 256 Hz (every 16384 cycles)
			while (sound_length_timer >= 16384)
			{
				sound_length_timer -= 16384;

				// update channel 1 length
				if ((*nr14 & 0x40) && (*nr52 & 0x01)) // length enabled and channel 1 active
				{
					if (channel1_length > 0)
					{
						channel1_length--;

						if (channel1_length == 0)
						{
							// disable channel 1
							*nr52 = (*nr52) & ~0x01;
						}
					}
				}

				// update channel 2 length
				if ((*nr24 & 0x40) && (*nr52 & 0x02)) // length enabled and channel 2 active
				{
					if (channel2_length > 0)
					{
						channel2_length--;

						if (channel2_length == 0)
						{
							// disable channel 2
							*nr52 = (*nr52) & ~0x02;
						}
					}
				}
			}

			return 0;
		}

		int trigger_channel1()
		{
			if (!is_initialized)
			{
				return 0;
			}

			channel1_length = 64 - (*nr11 & 0x3F);  // Load length counter

			// If length is 0, it wraps to 64
			if (channel1_length == 0)
			{
				channel1_length = 64;
			}

			return 0;
		}

		int trigger_channel2()
		{
			if (!is_initialized)
			{
				return 0;
			}

			channel2_length = 64 - (*nr21 & 0x3F);  // Load length counter

			// If length is 0, it wraps to 64
			if (channel2_length == 0)
			{
				channel2_length = 64;
			}

			return 0;
		}
	}
}
