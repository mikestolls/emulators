#pragma once

#include "defines.h"

#include "gameboy\memory_module.h"
#include "gameboy\cpu_instructions.h"

//Opcode  Z80				GMB
//---------------------------------------------
//08      EX   AF, AF		LD(nn), SP				done
//10      DJNZ PC + dd      STOP					done
//22      LD(nn), HL		LDI(HL), A				done
//2A      LD   HL, (nn)		LDI  A, (HL)			done
//32      LD(nn), A			LDD(HL), A				done
//3A      LD   A, (nn)		LDD  A, (HL)			done
//D3      OUT(n), A			-						done
//D9      EXX				RETI					not implemented
//DB      IN   A, (n)		-						done
//DD      <IX>				-						done
//E0      RET  PO			LD(FF00 + n), A			not implemented
//E2      JP   PO, nn		LD(FF00 + C), A			not implemented
//E3      EX(SP), HL		-						done
//E4      CALL P0, nn		-						done
//E8      RET  PE			ADD  SP, dd				not implemented - check if 8 or 16 bit operand
//EA      JP   PE, nn		LD(nn), A				not implemented
//EB      EX   DE, HL		-						done
//EC      CALL PE, nn		-						done
//ED      <pref>			-						done
//F0      RET  P			LD   A, (FF00 + n)		not implemented.
//F2      JP   P, nn		LD   A, (FF00 + C)		not implemented. check if supported
//F4      CALL P, nn		-						done
//F8      RET  M			LD   HL, SP + dd		not implemented - check if 8 or 16 bit operand
//FA      JP   M, nn		LD   A, (nn)			not implemented
//FC      CALL M, nn		-						done
//FD      <IY>				-						done
//CB3X    SLL  r / (HL)		SWAP r / (HL)			not implemented - rot[6] should be a swap

namespace gameboy
{
	namespace cpu
	{
		bool running = true;
		bool halt = false;

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
		u8* register_single[] = { &R.b, &R.c, &R.d, &R.e, &R.h, &R.l, /*memory_module->get_memory(R.hl)*/ 0, &R.a };
		
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

		// condition functions for instructions
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

		// alu functions for instructions
		inline void alu_add(u8* r)
		{
			R.a += *r;
		}

		inline void alu_add_carry(u8* r)
		{
			R.a += *r;
			R.a += get_flag(FLAG_CARRY);
		}

		inline void alu_sub(u8* r)
		{
			R.a -= *r;
		}

		inline void alu_sub_carry(u8* r)
		{
			R.a -= *r;
			R.a -= get_flag(FLAG_CARRY);
		}
		
		inline void alu_and(u8* r)
		{
			R.a &= *r;
		}

		inline void alu_xor(u8* r)
		{
			R.a ^= *r;
		}

		inline void alu_or(u8* r)
		{
			R.a |= *r;
		}

		inline void alu_cp(u8* r)
		{
			// do a sub without changing value of A
			u8 temp = R.a;
			alu_sub(r);
			R.a = temp;
		}

		void(*alu_function[])(u8*) = { alu_add, alu_add_carry, alu_sub, alu_sub_carry, alu_and, alu_xor, alu_or, alu_cp };

		// rotation and shift operations
		inline void rot_rlc(u8* r)
		{
			warning_assert("rot rlc function not implemented");
		}

		inline void rot_rrc(u8* r)
		{
			warning_assert("rot rrc function not implemented");
		}

		inline void rot_rl(u8* r)
		{
			warning_assert("rot rl function not implemented");
		}

		inline void rot_rr(u8* r)
		{
			warning_assert("rot rr function not implemented");
		}

		inline void rot_sla(u8* r)
		{
			warning_assert("rot sla function not implemented");
		}

		inline void rot_sra(u8* r)
		{
			warning_assert("rot sra function not implemented");
		}

		inline void rot_swap(u8* r)
		{
			warning_assert("rot swap function not implemented");
		}

		inline void rot_srl(u8* r)
		{
			warning_assert("rot srl function not implemented");
		}

		void(*rot_function[])(u8*) = { rot_rlc , rot_rrc, rot_rl, rot_rr, rot_sla, rot_sra, rot_swap, rot_srl };

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

			register_single[6] = memory_module->get_memory(R.hl); // need to point this to mem
			
			reset();

			return 0;
		}

		int decode_nonprefixed(u8 opcode)
		{
			u8 x = (opcode >> 6);
			u8 y = (opcode >> 3) & 0x7;
			u8 z = (opcode & 0x7);
			u8 p = (opcode >> 4) & 0x3;
			u8 q = (opcode >> 3) & 0x1;

			switch (x)
			{
			case 0x0: // x = 0
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
					{
						// LD mem NN with SP
						u16 addr = readpc_u16();
						memory_module->write_memory(addr, (const u8*)&R.sp, 2);
						break;
					}
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
						if (carry)
						{
							set_flag(FLAG_CARRY);
						}
						break;
					}
					case 0x1:
					{
						// RRC A
						u8 carry = R.a & 0x1;
						R.a = (R.a >> 1) | (carry << 7);
						clear_all_flags(); // reset flags
						if (carry)
						{
							set_flag(FLAG_CARRY);
						}
						break;
					}
					case 0x2:
					{
						// RL A
						u8 carry = R.a >> 7;
						R.a = (R.a << 1) | get_flag(FLAG_CARRY); // rotate with carry flag
						clear_all_flags(); // reset flags
						if (carry)
						{
							set_flag(FLAG_CARRY);
						}
						break;
					}
					case 0x3:
					{
						// RR A
						u8 carry = R.a & 0x1;
						R.a = (R.a >> 1) | (get_flag(FLAG_CARRY) << 7); // rotate with carry flag
						clear_all_flags(); // reset flags
						if (carry)
						{
							set_flag(FLAG_CARRY);
						}
						break;
					}
					case 0x4:
						// DA A
						warning_assert("DA A");
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
			case 0x1: // x = 1
			{
				if (z == 6)
				{
					// HALT
					halt = true;
				}
				else
				{
					// LD register_single[y] with register_single[z]
					*register_single[y] = *register_single[z];
				}
				break;
			} // end x = 1
			case 0x2: // x = 2
			{
				// alu[y] with register_single[z]
				alu_function[y](register_single[z]);
				break;
			}
			case 0x3: // x = 3
			{
				switch (z)
				{
				case 0x0: // z = 0
				{
					switch (y)
					{
					case 0x0:
					case 0x1:
					case 0x2:
					case 0x3:
						// RET if condition_funct[y]
						if (condition_funct[y])
						{
							warning_assert("RET if condition_funct[y]");
						}
						break;
					case 0x4:
						// LD mem(FF00 + n) with A
						warning_assert("LD mem(FF00 + n) with A");
						break;
					case 0x5:
						// ADD SP with (signed)nn
						warning_assert("ADD SP with (signed)nn");
						break;
					case 0x6:
						// LD A with mem(FF00 + n)
						warning_assert("LD A with mem(FF00 + n)");
						break;
					case 0x7:
						// ADD (signed)nn to SP then LD HL with SP
						warning_assert("ADD (signed)nn to SP then LD HL with SP");
						break;
					}
					break;
				}
				case 0x1: // z = 1
				{
					if (q == 0)
					{
						// POP stack ptr to register_pairs2[p]
						warning_assert("POP stack ptr to register_pairs2[p]");
					}
					else
					{
						switch (p)
						{
						case 0x0:
							// RET
							warning_assert("RET");
							break;
						case 0x1:
							// RETI
							warning_assert("RETI");
							break;
						case 0x2:
							// JP (HL)
							R.pc = R.hl;
							break;
						case 0x3:
							// LD SP with HL
							R.sp = R.hl;
							break;
						}
					}
					break;
				}
				case 0x2: // z = 2
				{
					switch (y)
					{
					case 0x0:
					case 0x1:
					case 0x2:
					case 0x3:
						// JP to nn if condition_funct[y]
						if (condition_funct[y])
						{
							R.pc = readpc_u16();
						}
						break;
					case 0x4:
						// LD mem(FF00 + C) with A
						warning_assert("LD mem(FF00 + C) with A");
						break;
					case 0x5:
						// LD mem(nn) with A
						warning_assert("LD mem(nn) with A");
						break;
					case 0x6:
						// LD A with mem(FF00 + C)
						warning_assert("LD A with mem(FF00 + C)");
						break;
					case 0x7:
						// LD A with mem(nn)
						warning_assert("LD A with mem(nn)");
						break;
					}
					break;
				}
				case 0x3: // z = 3
				{
					switch (y)
					{
					case 0x0:
						// JP nn
						warning_assert("JP nn");
						break;
					case 0x1:
						// CB prefix
						warning_assert("CB prefix");
						break;
					case 0x2:
					case 0x3:
					case 0x4:
					case 0x5:
						// unsupported by gameboy
						running = false;
						break;
					case 0x6:
						// DI - disable interupts
						warning_assert("DI - disable interupts");
						break;
					case 0x7:
						// EI - enable interupts
						warning_assert("EI - enable interupts");
						break;
					}
					break;
				}
				case 0x4: // z = 4
				{
					switch (y)
					{
					case 0x0:
					case 0x1:
					case 0x2:
					case 0x3:
						// CALL nn if condition_funct[y]
						warning_assert("CALL nn if condition_funct[y]");
						break;
					case 0x4:
					case 0x5:
					case 0x6:
					case 0x7:
						// unsupported by gameboy
						running = false;
						break;
					}
					break;
				}
				case 0x5: // z = 5
				{
					if (q == 0)
					{
						// PUSH register_pairs2[p]
						warning_assert("PUSH register_pairs2[p]");
					}
					else
					{
						if (p == 0)
						{
							// CALL nn
							warning_assert("CALL nn");
						}
						else
						{
							// unsupported by gameboy
							running = false;
						}
					}
					break;
				}
				case 0x6: // z = 6
				{
					// alu[y] with n
					u8 value = readpc_u8();
					alu_function[y](&value);
					break;
				}
				case 0x7: // z = 7
						  // RST at pc 7 * 8
					reset();
					R.pc = y * 8;
					break;
				}
				break;
			}
			}

			return 0;
		}

		int decode_prefixed_cb(u8 opcode)
		{
			u8 x = (opcode >> 6);
			u8 y = (opcode >> 3) & 0x7;
			u8 z = (opcode & 0x7);
			u8 p = (opcode >> 4) & 0x3;
			u8 q = (opcode >> 3) & 0x1;

			switch (x)
			{
			case 0x0:
				// rot_function[y] with register_single[z]
				warning_assert("rot[y] with register_single[z]");
				break;
			case 0x1:
				// test bit y from register_single[z]
				warning_assert("test bit y from register_single[z]");
				break;
			case 0x2:
				// reset bit y from register_single[z]
				warning_assert("reset bit y from register_single[z]");
				break;
			case 0x3:
				// set bit y from register_single[z]
				warning_assert("set bit y from register_single[z]");
				break;
			}

			return 0;
		}

		int update_cycle()
		{
			if (!running || halt)
			{
				// processor is stopped
				return 0;
			}

			// fetch the opcode
			u8 opcode = readpc_u8();

			// decode. gameboy only has CB prefix
			if (opcode == 0xCB)
			{
				opcode = readpc_u8();
				decode_prefixed_cb(opcode);
			}
			else
			{
				decode_nonprefixed(opcode);
			}

			return 0;
		}
	}
}