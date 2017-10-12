#pragma once

#include "defines.h"

namespace z80
{
	class cpu
	{
	public:
		u8 a, b, c, d, e, h, l, f;
		u16 PC, SP;
	};
}