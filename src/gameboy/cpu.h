#pragma once

#include "defines.h"

#include "gameboy\memory_module.h"
#include "gameboy\cpu_instructions.h"

//Opcode  Z80				GMB
//---------------------------------------------
//08      EX   AF, AF		LD(nn), SP
//10      DJNZ PC + dd      STOP
//22      LD(nn), HL		LDI(HL), A
//2A      LD   HL, (nn)		LDI  A, (HL)
//32      LD(nn), A			LDD(HL), A
//3A      LD   A, (nn)		LDD  A, (HL)
//D3      OUT(n), A			-
//D9      EXX				RETI
//DB      IN   A, (n)		-
//DD      <IX>				-
//E0      RET  PO			LD(FF00 + n), A
//E2      JP   PO, nn		LD(FF00 + C), A
//E3      EX(SP), HL		-
//E4      CALL P0, nn		-
//E8      RET  PE			ADD  SP, dd
//EA      JP   PE, nn		LD(nn), A
//EB      EX   DE, HL		-
//EC      CALL PE, nn		-
//ED      <pref>			-
//F0      RET  P			LD   A, (FF00 + n)
//F2      JP   P, nn		LD   A, (FF00 + C)
//F4      CALL P, nn		-
//F8      RET  M			LD   HL, SP + dd
//FA      JP   M, nn		LD   A, (nn)
//FC      CALL M, nn		-
//FD      <IY>				-
//CB3X    SLL  r / (HL)		SWAP r / (HL)

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
		u16* register_pairs2[] = { &R.bc, &R.de, &R.hl, &R.af };
		u8* register_single[] = { &R.b, &R.c, &R.d, &R.e, &R.h, &R.l, memory_module->get_memory(R.hl), &R.a };
		
		// set and get flag helpers
		inline void set_flag(u8 flag)
		{
			flag = (1 << flag);
			R.f |= flag;
		}

		inline void clear_flag(u8 flag)
		{
			flag = (1 << flag);
			R.f &= ~flag; // clear the bit
		}

		inline u8 get_flag(u8 flag)
		{
			flag = (1 << flag);
			return ((R.f & flag) >> flag);
		}

		inline void clear_all_flags()
		{
			R.f = 0x0;
		}

		enum FLAGS
		{
			FLAG_CARRY = 4,
			FLAG_HALFCARRY = 5,
			FLAG_SUBTRACTION = 6,
			FLAG_ZERO = 7,
		};

		inline bool condition_notzero()
		{
			return get_flag(FLAG_ZERO) == 0;
		}

		inline bool condition_zero()
		{
			return get_flag(FLAG_ZERO) != 0;
		}

		inline bool condition_notcarry()
		{
			return get_flag(FLAG_CARRY) == 0;
		}

		inline bool condition_carry()
		{
			return get_flag(FLAG_CARRY);
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
						// STOP
						running = false;
						break;
					case 0x3:
						// JR d
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
				case 0x3: // z = 3
				{
					switch (q)
					{
					case 0x0:
						// INC register_pairs[p]
						*register_pairs[p]++;
						break;
					case 0x1:
						// DEC register_pairs[p]
						*register_pairs[p]--;
						break;
					}
					break;
				}
				case 0x4: // z = 4
					// INC register_single[y]
					*register_single[y]++;
					break;
				case 0x5: // z = 5
					// DEC register_single[y]
					*register_single[y]--;
					break;
				case 0x6: // z = 6
					// LD register_single[y] with n
					*register_single[y] = readpc_u8();
					break;
				case 0x7: // z = 7
				{
					switch (y)
					{
					case 0x0:
					{
						// RLC A
						u8 carry = R.a >> 7;
						R.a = (R.a << 1) | carry;
						clear_all_flags(); // reset flags
						if (carry) { set_flag(FLAG_CARRY); }
						break;
					}
					case 0x1:
					{
						// RRC A
						u8 carry = R.a & 0x1;
						R.a = (R.a >> 1) | (carry << 7);
						clear_all_flags(); // reset flags
						if (carry) { set_flag(FLAG_CARRY); }
						break;
					}
					case 0x2:
					{
						// RL A
						u8 carry = R.a >> 7;
						R.a = (R.a << 1) | get_flag(FLAG_CARRY); // rotate with carry flag
						clear_all_flags(); // reset flags
						if (carry) { set_flag(FLAG_CARRY); }
						break;
					}
					case 0x3:
					{
						// RR A
						u8 carry = R.a & 0x1;
						R.a = (R.a >> 1) | (get_flag(FLAG_CARRY) << 7); // rotate with carry flag
						clear_all_flags(); // reset flags
						if (carry) { set_flag(FLAG_CARRY); }
						break;
					}
					case 0x4:
						// DA A
						printf("Error - Not implemented\n");
						break;
					case 0x5:
						// CPL
						R.a = ~R.a;
						set_flag(FLAG_HALFCARRY);
						set_flag(FLAG_SUBTRACTION);
						break;
					case 0x6:
						// SCF
						set_flag(FLAG_CARRY);
						clear_flag(FLAG_HALFCARRY);
						clear_flag(FLAG_SUBTRACTION);
						break;
					case 0x7:
						// CCF
						if (get_flag(FLAG_CARRY))
						{
							clear_flag(FLAG_CARRY);
						}
						else
						{
							set_flag(FLAG_CARRY);
						}
						clear_flag(FLAG_HALFCARRY);
						clear_flag(FLAG_SUBTRACTION);
						break;
					}
					break;
				}
				}
				break;
				} // end x = 0
			}

			return 0;
		}
	}
}