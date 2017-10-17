#pragma once

#include "defines.h"

#include "gameboy\memory_module.h"
#include "gameboy\cpu_instructions.h"

namespace gameboy
{
	namespace cpu
	{
		bool running = true;

		// main memory module pointer
		gameboy::memory_module* memory_module;

		// cpus register structure
		struct registers
		{
			struct
			{
				union
				{
					struct
					{
						u8 f;
						u8 a;
					};
					u16 af;
				};
			};
			struct
			{
				union
				{
					struct
					{
						u8 c;
						u8 b;
					};
					u16 bc;
				};
			};
			struct
			{
				union
				{
					struct
					{
						u8 e;
						u8 d;
					};
					u16 de;
				};
			};
			struct
			{
				union
				{
					struct
					{
						u8 l;
						u8 h;
					};
					u16 hl;
				};
			};

			u16 sp;
			u16 pc;
		} R;

		// register pointers used by decoder
		u16* register_pairs[] = { &R.bc, &R.de, &R.hl, &R.sp };
		
		// set and get flag helpers
		inline void set_flags(u8 flags)
		{
			R.f |= flags;
		}

		inline void clear_flags()
		{
			R.f = 0x0;
		}

		inline u8 compare_flags(u8 flags)
		{
			return R.f & flags;
		}

		inline bool check_flag(u8 flag)
		{
			return ((R.f & flag) == flag);
		}

		enum FLAGS
		{
			FLAG_ZERO = (1 << 7),
			FLAG_SUBTRACTION = (1 << 6),
			FLAG_HALFCARRY = (1 << 5),
			FLAG_CARRY = (1 << 4)
		};

		inline bool condition_notzero()
		{
			return !check_flag(FLAG_ZERO);
		}

		inline bool condition_zero()
		{
			return check_flag(FLAG_ZERO);
		}

		inline bool condition_notcarry()
		{
			return !check_flag(FLAG_CARRY);
		}

		inline bool condition_carry()
		{
			return check_flag(FLAG_CARRY);
		}

		inline bool condition_invalid()
		{
			printf("Error - A condition was decoded that is not valid for cpu");
			return false;
		}

		bool (*condition_funct[])(void) = { condition_notzero, condition_zero, condition_notcarry, condition_carry, condition_invalid, condition_invalid, condition_invalid, condition_invalid };

		// read 8 and 16 bit at PC. increment PC
		inline u8 readpc_u8()
		{
			u8 val = memory_module->read_memory(R.pc++);

			return val;
		}

		inline u16 readpc_u16()
		{
			// lsb is first in memory
			u16 val = memory_module->read_memory(R.pc++);
			val |= (memory_module->read_memory(R.pc++) << 8);

			return val;
		}

		int reset()
		{
			memset(&R, 0x0, sizeof(R)); // init registers to 0

			R.af = 0x01B0;
			R.bc = 0x0013;
			R.de = 0x00D8;
			R.hl = 0x014D;

			R.pc = 0x0100; // starting entry point of the ROM
			R.sp = 0xFFFE;

			return 0;
		}

		int initialize(gameboy::memory_module* memory)
		{
			memory_module = memory;
			
			reset();

			return 0;
		}

		int update_cycle()
		{
			if (!running)
			{
				// processor is stopped
				return 0;
			}

			// fetch the opcode
			u8 opcode = readpc_u8();

			// check for 0xCB prefix

			u8 x = (opcode >> 6);
			u8 y = (opcode >> 3) & 0x7;
			u8 z = (opcode & 0x7);
			u8 p = (opcode >> 4) & 0x3;
			u8 q = (opcode >> 3) & 0x1;
			switch (x)
			{
			case 0x0:
			{
				switch (z)
				{
				case 0x0: // z = 0
				{
					switch (y)
					{
					case 0x0:
						// NOP
						break;
					case 0x1:
						// LD sp with nn
						R.sp = readpc_u16();
						break;
					case 0x2:
						// STOP - diff from z80
						running = false;
						break;
					case 0x3:
						R.pc += (s8)readpc_u8(); // relative jump is singed offset
						break;
					case 0x4:
					case 0x5:
					case 0x6:
					case 0x7:
						// JR conditions[y - 4], d - relative jump
						if (condition_funct[y - 4])
						{
							R.pc += (s8)readpc_u8(); // relative jump is singed offset
						}
						break;
					}
					break;
				}
				case 0x1: // z = 1
				{
					switch (q)
					{
					case 0x0:
						// LD register_pairs[p] with nn
						*register_pairs[p] = readpc_u16();
						break;
					case 0x1:
						// ADD HL with register_pairs[p]
						R.hl += *register_pairs[p];
						break;
					}
					break;
				}
				case 0x2: // z = 2
				{
					switch (q)
					{
					case 0x0:
					{
						switch (p)
						{
						case 0x0:
							// LD (BC) with A
							memory_module->write_memory(R.bc, &R.a, 1);
							break;
						case 0x1:
							// LD (DE) with A
							memory_module->write_memory(R.de, &R.a, 1);
							break;
						case 0x2:
							// LDI (HL) with A. inc HL
							memory_module->write_memory(R.hl, &R.a, 1);
							R.hl++;
							break;
						case 0x3:
							// LDD (HL) with A. decr HL
							memory_module->write_memory(R.hl, &R.a, 1);
							R.hl--;
							break;
						}
						break;
					}
					case 0x1:
					{
						switch (p)
						{
						case 0x0:
							// LD A with (BC)
							R.a = memory_module->read_memory(R.bc);
							break;
						case 0x1:
							// LD A with (DE)
							R.a = memory_module->read_memory(R.de);
							break;
						case 0x2:
							// LDI A with (HL). inc HL
							R.a = memory_module->read_memory(R.hl);
							R.hl++;
							break;
						case 0x3:
							// LDD A with (HL). decr HL
							R.a = memory_module->read_memory(R.hl);
							R.hl--;
							break;
						}
						break;
					}
					}
					break;
				}
				}
				break;
			}
			}

			return 0;
		}
	}
}