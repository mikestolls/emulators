#pragma once

#include "defines.h"

namespace nes
{
	namespace mapper
	{
		int initialize()
		{
			return 0;
		}

		int reset()
		{
			return 0;
		}

		// function pointers for mbc
		int(*mapper_initialize)() = &initialize;
		int(*mapper_reset)() = &reset;
	};
}