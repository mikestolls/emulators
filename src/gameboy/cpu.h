#pragma once

#include "defines.h"

#include "gameboy\memory_module.h"

//Opcode  Z80				GMB
//---------------------------------------------
//08      EX   AF, AF		LD(nn), SP				done
//10      DJNZ PC + dd      STOP					done
//22      LD(nn), HL		LDI(HL), A				done
//2A      LD   HL, (nn)		LDI  A, (HL)			done
//32      LD(nn), A			LDD(HL), A				done
//3A      LD   A, (nn)		LDD  A, (HL)			done
//D3      OUT(n), A			-						done
//D9      EXX				RETI					done
//DB      IN   A, (n)		-						done
//DD      <IX>				-						done
//E0      RET  PO			LD(FF00 + n), A			done
//E2      JP   PO, nn		LD(FF00 + C), A			done
//E3      EX(SP), HL		-						done
//E4      CALL P0, nn		-						done
//E8      RET  PE			ADD  SP, d				done
//EA      JP   PE, nn		LD(nn), A				done
//EB      EX   DE, HL		-						done
//EC      CALL PE, nn		-						done
//ED      <pref>			-						done
//F0      RET  P			LD   A, (FF00 + n)		done
//F2      JP   P, nn		LD   A, (FF00 + C)		done
//F4      CALL P, nn		-						done
//F8      RET  M			LD   HL, SP + d			done
//FA      JP   M, nn		LD   A, (nn)			done
//FC      CALL M, nn		-						done
//FD      <IY>				-						done
//CB3X    SLL  r / (HL)		SWAP r / (HL)			done

namespace gameboy
{
	namespace cpu
	{
		bool running = true;
		bool halt = false;
		bool debugging = false;
		bool debugging_step = false;

		bool interrupt_master;
		u8* interrupt_enable_flag;
		u8* interrupt_request_flag;

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
		
		// stack functions
		inline void push_sp_to_stack(u16 addr)
		{
			u8 low = (addr & 0x00FF);
			u8 high = (addr >> 8);

			R.sp -= 2;
			memory_module->write_memory(R.sp, &low, 1);
			memory_module->write_memory(R.sp + 1, &high, 1);
		}

		inline u16 pop_from_stack()
		{
			u8 low = memory_module->read_memory(R.sp++) & 0xFF;
			u8 high = memory_module->read_memory(R.sp++) & 0xFF;

			return (high << 8) | low;
		}

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
			return ((R.f & (1 << flag)) >> flag);
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
			return get_flag(FLAG_CARRY) != 0;
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
			u16 res = R.a + *r;

			// set flags
			clear_all_flags();

			// check for carry
			if (res & 0xFF00)
			{
				set_flag(FLAG_CARRY);
			}

			// check for the half carry
			if ((R.a ^ *r ^ res) & 0x10)
			{
				set_flag(FLAG_HALFCARRY);
			}

			// set new value
			R.a = (u8)(res & 0xFF);

			if (R.a == 0)
			{
				set_flag(FLAG_ZERO);
			}
		}

		inline void alu_add_carry(u8* r)
		{
			u16 value = *r + get_flag(FLAG_CARRY);
			u16 res = R.a + value;
			
			// set flags
			clear_all_flags();

			// check for carry
			if (res & 0xFF00)
			{
				set_flag(FLAG_CARRY);
			}

			// check for the half carry
			if ((R.a ^ *r ^ res) & 0x10)
			{
				set_flag(FLAG_HALFCARRY);
			}

			// set new value
			R.a = (u8)(res & 0xFF);

			if (R.a == 0)
			{
				set_flag(FLAG_ZERO);
			}
		}

		inline void alu_sub(u8* r)
		{
			u16 res = R.a - *r;

			// set flags
			clear_all_flags();
			set_flag(FLAG_SUBTRACTION);
		
			// check for carry
			if (*r > R.a)
			{
				set_flag(FLAG_CARRY);
			}

			// check for the half carry
			if ((R.a ^ *r ^ res) & 0x10)
			{
				set_flag(FLAG_HALFCARRY);
			}

			// set new value
			R.a = (u8)(res & 0xFF);

			if (R.a == 0)
			{
				set_flag(FLAG_ZERO);
			}
		}

		inline void alu_sub_carry(u8* r)
		{
			u16 value = *r + get_flag(FLAG_CARRY);
			u16 res = R.a - value;

			// set flags
			clear_all_flags();
			set_flag(FLAG_SUBTRACTION);

			// check for carry
			if (value > R.a)
			{
				set_flag(FLAG_CARRY);
			}

			// check for the half carry
			if ((R.a ^ *r ^ res) & 0x10)
			{
				set_flag(FLAG_HALFCARRY);
			}

			// set new value
			R.a = (u8)(res & 0xFF);

			if (R.a == 0)
			{
				set_flag(FLAG_ZERO);
			}
		}
		
		inline void alu_and(u8* r)
		{
			R.a &= *r;

			// set flags
			clear_all_flags();
			set_flag(FLAG_HALFCARRY);

			if (R.a == 0)
			{
				set_flag(FLAG_ZERO);
			}
		}

		inline void alu_xor(u8* r)
		{
			R.a ^= *r;

			// set flags
			clear_all_flags();

			if (R.a == 0)
			{
				set_flag(FLAG_ZERO);
			}
		}

		inline void alu_or(u8* r)
		{
			R.a |= *r;

			// set flags
			clear_all_flags();

			if (R.a == 0)
			{
				set_flag(FLAG_ZERO);
			}
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
			u8 carry = (*r & 0x80) >> 7;
			*r = (*r << 1) | carry;

			clear_all_flags();
			if (carry)
			{
				set_flag(FLAG_CARRY);
			}

			if (*r == 0)
			{
				set_flag(FLAG_ZERO);
			}
		}

		inline void rot_rrc(u8* r)
		{
			u8 carry = (*r & 0x1);
			*r = (*r >> 1) | (carry << 7);

			clear_all_flags();
			if (carry)
			{
				set_flag(FLAG_CARRY);
			}

			if (*r == 0)
			{
				set_flag(FLAG_ZERO);
			}
		}

		inline void rot_rl(u8* r)
		{
			u8 carry = get_flag(FLAG_CARRY);

			clear_all_flags();
			if ((*r >> 7))
			{
				set_flag(FLAG_CARRY);
			}

			*r = (*r << 1) | carry;

			if (*r == 0)
			{
				set_flag(FLAG_ZERO);
			}
		}

		inline void rot_rr(u8* r)
		{
			u8 carry = get_flag(FLAG_CARRY);

			clear_all_flags();
			if (*r & 0x1)
			{
				set_flag(FLAG_CARRY);
			}

			*r = (*r >> 1) | (carry << 7);

			if (*r == 0)
			{
				set_flag(FLAG_ZERO);
			}
		}

		inline void rot_sla(u8* r)
		{
			clear_all_flags();
			if (*r & 0x80)
			{
				set_flag(FLAG_CARRY);
			}

			*r <<= 1;

			// if set zero flag
			if (*r == 0)
			{
				set_flag(FLAG_ZERO);
			}
		}

		inline void rot_sra(u8* r)
		{
			clear_all_flags();
			if (*r & 0x1)
			{
				set_flag(FLAG_CARRY);
			}

			*r = (*r & 0x80) | (*r >> 1); // high bit stays

			// set zero flag
			if (*r == 0)
			{
				set_flag(FLAG_ZERO);
			}
		}

		inline void rot_swap(u8* r)
		{
			clear_all_flags();
			*r = ((*r & 0x0F) << 4) | ((*r & 0xF0) >> 4);

			// set zero flag
			if (*r == 0)
			{
				set_flag(FLAG_ZERO);
			}
		}

		inline void rot_srl(u8* r)
		{
			clear_all_flags();
			if (*r & 0x1)
			{
				set_flag(FLAG_CARRY);
			}

			*r >>= 1; // high bit 0

			// set zero flag
			if (*r == 0)
			{
				set_flag(FLAG_ZERO);
			}
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

		// interrupt functionality
		enum INTERRUPT_FLAG
		{
			INTERRUPT_VBLANK = 0,
			INTERRUPT_LCD,
			INTERRUPT_TIMER,
			INTERRUPT_JOYPAD,
		};

		inline void disable_interrupts()
		{
			interrupt_master = false;
		}

		inline void enable_interrupts()
		{
			interrupt_master = true;
		}

		inline void set_request_interrupt_flag(u8 flag)
		{
			flag = (1 << flag);
			*interrupt_request_flag |= flag;
		}

		inline void clear_request_interrupt_flag(u8 flag)
		{
			flag = (1 << flag);
			*interrupt_request_flag &= ~flag; // clear the bit
		}

		inline u8 get_request_interrupt_flag(u8 flag)
		{
			return ((*interrupt_request_flag & (1 << flag)) >> flag);
		}

		inline void clear_all_request_interrupt_flags()
		{
			*interrupt_request_flag = 0x0;
		}

		// interrupt enable function
		inline void set_enabled_interrupt_flag(u8 flag)
		{
			flag = (1 << flag);
			*interrupt_enable_flag |= flag;
		}

		inline void clear_enabled_interrupt_flag(u8 flag)
		{
			flag = (1 << flag);
			*interrupt_enable_flag &= ~flag; // clear the bit
		}

		inline u8 get_enabled_interrupt_flag(u8 flag)
		{
			return ((*interrupt_enable_flag & (1 << flag)) >> flag);
		}

		inline void clear_all_enabled_interrupt_flags()
		{
			*interrupt_enable_flag = 0x0;
		}

		void service_interrupt(u8 interrupt)
		{
			interrupt_master = false;
			clear_request_interrupt_flag(interrupt);

			push_sp_to_stack(R.pc);

			u16 addr = 0;
			switch (interrupt)
			{
			case INTERRUPT_VBLANK:
				addr = 0x40;
				break;
			case INTERRUPT_LCD:
				addr = 0x48;
				break;
			case INTERRUPT_TIMER:
				addr = 0x50;
				break;
			case INTERRUPT_JOYPAD:
				addr = 0x60;
				break;
			default:
				printf("Error - Trying to service an invalid interrupt: %d\n", interrupt);
				assert(0);
				break;
			}

			R.pc = addr;
		}

		int check_interrupts()
		{
			if (!interrupt_master)
			{
				return 0;
			}

			for (u8 i = 0; i <= INTERRUPT_JOYPAD; i++)
			{
				if (get_request_interrupt_flag(i) && get_enabled_interrupt_flag(i))
				{
					// service the intterupt
					service_interrupt(i);
				}
			}

			return 0;
		}

		int reset()
		{
			memset(&R, 0x0, sizeof(R)); // init registers to 0

			R.af = 0x01B0;
			R.bc = 0x0013;
			R.de = 0x00D8;
			R.hl = 0x014D;

			// NOTE: these match the bgb init values
			R.af = 0x1180;
			R.bc = 0x0000;
			R.de = 0xFF56;
			R.hl = 0x000D;

			R.pc = 0x0100; // starting entry point of the ROM
			R.sp = 0xFFFE;

			memory_module->reset();

			interrupt_master = true;
			interrupt_enable_flag = memory_module->get_memory(0xFFFF);
			interrupt_request_flag = memory_module->get_memory(0xFF0F);

			return 0;
		}

		int initialize(gameboy::memory_module* memory)
		{
			memory_module = memory;
			
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

			u8 cycles = 0;

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
						cycles = 4;
						break;
					case 0x1:
					{
						// LD mem NN with SP
						u16 addr = readpc_u16();
						memory_module->write_memory(addr, (const u8*)&R.sp, 2);
						cycles = 20;
						break;
					}
					case 0x2:
						// STOP
						running = false;
						cycles = 4;
						break;
					case 0x3:
						// JR d
						R.pc += (s8)readpc_u8(); // relative jump is singed offset
						cycles = 12;
						break;
					case 0x4:
					case 0x5:
					case 0x6:
					case 0x7:
					{
						cycles = 8; // if condition true 12, if false 8

						s8 val = (s8)readpc_u8();

						// JR conditions[y - 4], d - relative jump
						if (condition_funct[y - 4]())
						{
							R.pc += val; // relative jump is singed offset
							cycles += 4;
						}
						break;
					}
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
						cycles = 12;
						break;
					case 0x1:
						// ADD HL with register_pairs[p]
						u32 res = R.hl + *register_pairs[p];

						// check for carry
						if (res & 0xFFFF0000)
						{
							set_flag(FLAG_CARRY);
						}
						else
						{
							clear_flag(FLAG_CARRY);
						}

						// check for the half carry.
						if ((R.hl & 0xFFF) + (*register_pairs[p] & 0xFFF) > 0xFFF)
						{
							set_flag(FLAG_HALFCARRY);
						}
						else
						{
							clear_flag(FLAG_HALFCARRY);
						}

						clear_flag(FLAG_SUBTRACTION);

						R.hl = (u16)(res & 0xFFFF);

						cycles = 8;
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
							cycles = 8;
							break;
						case 0x1:
							// LD (DE) with A
							memory_module->write_memory(R.de, &R.a, 1);
							cycles = 8;
							break;
						case 0x2:
							// LDI (HL) with A. inc HL
							memory_module->write_memory(R.hl, &R.a, 1);
							R.hl++;
							cycles = 8;
							break;
						case 0x3:
							// LDD (HL) with A. decr HL
							memory_module->write_memory(R.hl, &R.a, 1);
							R.hl--;
							cycles = 8;
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
							cycles = 8;
							break;
						case 0x1:
							// LD A with (DE)
							R.a = memory_module->read_memory(R.de);
							cycles = 8;
							break;
						case 0x2:
							// LDI A with (HL). inc HL
							R.a = memory_module->read_memory(R.hl);
							R.hl++;
							cycles = 8;
							break;
						case 0x3:
							// LDD A with (HL). decr HL
							R.a = memory_module->read_memory(R.hl);
							R.hl--;
							cycles = 8;
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
						(*register_pairs[p])++;
						cycles = 8;
						break;
					case 0x1:
						// DEC register_pairs[p]
						(*register_pairs[p])--;
						cycles = 8;
						break;
					}
					break;
				}
				case 0x4: // z = 4
					// INC register_single[y]
					// check for the half carry only
					if ((*register_single[y] & 0xF) == 0x0F)
					{
						set_flag(FLAG_HALFCARRY);
					}
					else
					{
						clear_flag(FLAG_HALFCARRY);
					}

					// set new value
					*register_single[y] += 1;

					if (*register_single[y] == 0)
					{
						set_flag(FLAG_ZERO);
					}
					else
					{
						clear_flag(FLAG_ZERO);
					}

					clear_flag(FLAG_SUBTRACTION);

					if (y == 6) // register (HL)
					{
						cycles = 12;
					}
					else
					{
						cycles = 4;
					}
					break;
				case 0x5: // z = 5
					// DEC register_single[y]
					// check for the half carry only
					if (*register_single[y] & 0x0F)
					{
						clear_flag(FLAG_HALFCARRY);
					}
					else
					{
						set_flag(FLAG_HALFCARRY);
					}

					// set new value
					*register_single[y] -= 1;

					if (*register_single[y] == 0)
					{
						set_flag(FLAG_ZERO);
					}
					else
					{
						clear_flag(FLAG_ZERO);
					}

					set_flag(FLAG_SUBTRACTION);

					if (y == 6) // register (HL)
					{
						cycles = 12;
					}
					else
					{
						cycles = 4;
					}
					break;
				case 0x6: // z = 6
					// LD register_single[y] with n
					*register_single[y] = readpc_u8();

					if (y == 6) // register is (HL)
					{
						cycles = 12;
					}
					else
					{
						cycles = 8;
					}
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

						cycles = 4;
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

						cycles = 4;
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

						cycles = 4;
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

						cycles = 4;
						break;
					}
					case 0x4:
					{

						/*void DAA(CPU* cpu)
						{
							byte a = cpu->registers.a;
							if ((cpu->registers.f & 0x20) || ((cpu->registers.a & 15)>9))
								cpu->registers.a += 6; 

							cpu->registers.f &= 0xEF;

							if ((cpu->registers.f & 0x20) || (a>0x99)) 
							{
								cpu->registers.a += 0x60;
								cpu->registers.f |= 0x10; 
							} 
							cpu->registers.m = 1; 
						};*/

						// DA A
						u8 result = R.a;
						u8 incr = 0;
						bool carry = get_flag(FLAG_CARRY) != 0;

						if (get_flag(FLAG_HALFCARRY) || ((result & 0x0f) > 0x09))
						{
							incr |= 0x06;
						}

						if (carry || (result > 0x9f) || ((result > 0x8f) && ((result & 0x0f) > 0x09)))
						{
							incr |= 0x60;
						}

						if (result > 0x99)
						{
							carry = true;
						}

						bool subtract = get_flag(FLAG_SUBTRACTION) != 0;

						// these will clear flags and set half carry, zero, and subtraction flags
						if (subtract)
						{
							alu_sub(&incr);
						}
						else
						{
							alu_add(&incr);
						}

						// reset flags
						if (carry)
						{
							set_flag(FLAG_CARRY);
						}
						else
						{
							clear_flag(FLAG_CARRY);
						}

						if (subtract)
						{
							set_flag(FLAG_SUBTRACTION);
						}
						else
						{
							clear_flag(FLAG_SUBTRACTION);
						}

						clear_flag(FLAG_HALFCARRY);

						cycles = 4;
						break;
					}
					case 0x5:
						// CPL
						R.a = ~R.a;
						set_flag(FLAG_HALFCARRY);
						set_flag(FLAG_SUBTRACTION);

						cycles = 4;
						break;
					case 0x6:
						// SCF
						set_flag(FLAG_CARRY);
						clear_flag(FLAG_HALFCARRY);
						clear_flag(FLAG_SUBTRACTION);

						cycles = 4;
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

						cycles = 4;
						break;
					}
					break;
				}
				}
				break;
			} // end x = 0
			case 0x1: // x = 1
			{
				if (z == 6 && y== 6)
				{
					// HALT
					halt = true;
					cycles = 4;
				}
				else
				{
					// LD register_single[y] with register_single[z]
					*register_single[y] = *register_single[z];

					if (y == 6 || z == 6) // register is (HL)
					{
						cycles = 8;
					}
					else
					{
						cycles = 4;
					}
				}
				break;
			} // end x = 1
			case 0x2: // x = 2
			{
				// alu[y] with register_single[z]
				alu_function[y](register_single[z]);

				if (z == 6) // using (HL) register
				{
					cycles = 8;
				}
				else
				{
					cycles = 4;
				}
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
						cycles = 8;
						if (condition_funct[y]())
						{
							R.pc = pop_from_stack();

							cycles += 12;
						}
						break;
					case 0x4:
						// LD mem(FF00 + n) with A
						memory_module->write_memory(0xFF00 + readpc_u8(), &R.a, 1);
						cycles = 12;
						break;
					case 0x5:
					{
						// ADD SP with (signed)n
						u8 val = readpc_u8();
						u32 res = R.sp + val;

						// check for carry
						clear_all_flags();
						if (res & 0xFF00)
						{
							set_flag(FLAG_CARRY);
						}

						// check for the half carry.
						if ((R.sp ^ val ^ res) & 0x10)
						{
							set_flag(FLAG_HALFCARRY);
						}

						R.sp += (s8)val;
						
						cycles = 16;
						break;
					}
					case 0x6:
						// LD A with mem(FF00 + n)
						R.a = memory_module->read_memory(0xFF00 + readpc_u8());
						cycles = 12;
						break;
					case 0x7:
					{
						// ADD (signed)n to SP then LD HL with SP
						clear_all_flags();
						u8 operand = readpc_u8();
						u32 res = R.sp + (s8)operand;

						// check for carry
						if (res & 0xFFFF0000)
						{
							set_flag(FLAG_CARRY);
						}

						// check for the half carry.
						if ((R.sp & 0x0F) + (operand & 0x0F) > 0x0F)
						{
							set_flag(FLAG_HALFCARRY);
						}

						R.hl = (u16)(res & 0xFFFF); 
						cycles = 12;
						break;
					}
					}
					break;
				}
				case 0x1: // z = 1
				{
					if (q == 0)
					{
						// POP stack ptr to register_pairs2[p]
						u16 addr = pop_from_stack();

						if (p == 3) // R.af has special case. cant set lower 4 bits of f register
						{
							addr &= 0xFFF0;
						}

						*(register_pairs2[p]) = addr;
						cycles = 12;
					}
					else
					{
						switch (p)
						{
						case 0x0:
							// RET
							R.pc = pop_from_stack();

							cycles = 16;
							break;
						case 0x1:
							// RETI
							R.pc = pop_from_stack();
							interrupt_master = true;

							cycles = 16;
							break;
						case 0x2:
							// JP (HL)
							R.pc = R.hl;

							cycles = 4;
							break;
						case 0x3:
							// LD SP with HL
							R.sp = R.hl;
							cycles = 8;
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
					{
						cycles = 12; // 16 if true, 12 is false

						// JP to nn if condition_funct[y]
						u16 val = readpc_u16();
						if (condition_funct[y]())
						{
							R.pc = val;

							cycles += 4;
						}
						break;
					}
					case 0x4:
						// LD mem(FF00 + C) with A
						memory_module->write_memory(0xFF00 + R.c, &R.a, 1);
						cycles = 8;
						break;
					case 0x5:
						// LD mem(nn) with A
						memory_module->write_memory(readpc_u16(), &R.a, 1);
						cycles = 16;
						break;
					case 0x6:
						// LD A with mem(FF00 + C)
						R.a = memory_module->read_memory(0xFF00 + R.c);
						cycles = 8;
						break;
					case 0x7:
						// LD A with mem(nn)
						R.a = memory_module->read_memory(readpc_u16());
						cycles = 16;
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
						R.pc = readpc_u16();

						cycles = 16;
						break;
					case 0x1:
						// CB prefix
						printf("Error - CB prefix opcode should not get here\n");
						assert(0);
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
						disable_interrupts();
						cycles = 4;
						break;
					case 0x7:
						// EI - enable interupts
						enable_interrupts();
						cycles = 4;
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
					{
						cycles = 12;

						// CALL nn if condition_funct[y]
						u16 val = readpc_u16();
						if (condition_funct[y]())
						{
							push_sp_to_stack(R.pc);

							R.pc = val;

							cycles += 12;
						}
						break;
					}
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
						push_sp_to_stack(*register_pairs2[p]);

						cycles = 16;
					}
					else
					{
						if (p == 0)
						{
							// CALL nn
							u16 val = readpc_u16();
							push_sp_to_stack(R.pc);
							R.pc = val;

							cycles = 24;
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
					cycles = 8;
					break;
				}
				case 0x7: // z = 7
				{
					// RST at pc 7 * 8. basically a CALL
					push_sp_to_stack(R.pc);
					R.pc = y * 8;

					cycles = 16;
					break;
				}
				}
				break;
			}
			}

			return cycles;
		}

		int decode_prefixed_cb(u8 opcode)
		{
			u8 x = (opcode >> 6);
			u8 y = (opcode >> 3) & 0x7;
			u8 z = (opcode & 0x7);
			u8 p = (opcode >> 4) & 0x3;
			u8 q = (opcode >> 3) & 0x1;

			u8 cycles = 0;

			switch (x)
			{
			case 0x0:
				// rot_function[y] with register_single[z]
				rot_function[y](register_single[z]);

				if (z == 6) // (HL) register
				{
					cycles = 16;
				}
				else
				{
					cycles = 8;
				}
				break;
			case 0x1:
				// test bit y from register_single[z]
				if (*register_single[z] & (1 << y))
				{
					clear_flag(FLAG_ZERO);
				}
				else
				{
					set_flag(FLAG_ZERO);
				}

				set_flag(FLAG_HALFCARRY);
				clear_flag(FLAG_SUBTRACTION);

				if (z == 6) // (HL) register
				{
					cycles = 12;
				}
				else
				{
					cycles = 8;
				}
				break;
			case 0x2:
				// reset bit y from register_single[z]
				*register_single[z] &= ~(1 << y);

				if (z == 6) // (HL) register
				{
					cycles = 16;
				}
				else
				{
					cycles = 8;
				}
				break;
			case 0x3:
				// set bit y from register_single[z]
				*register_single[z] |= (1 << y);

				if (z == 6) // (HL) register
				{
					cycles = 16;
				}
				else
				{
					cycles = 8;
				}
				break;
			}

			return cycles;
		}

		int execute_opcode()
		{
			if (!running || halt)
			{
				// processor is stopped
				return 0;
			}

			// need to point this to mem. small hack for the (HL) register instructons
			register_single[6] = memory_module->get_memory(R.hl); 

			u8 cycles = 0;

			// fetch the opcode
			u8 opcode = readpc_u8();

			// decode. gameboy only has CB prefix
			if (opcode == 0xCB)
			{
				opcode = readpc_u8();
				cycles = decode_prefixed_cb(opcode);
			}
			else
			{
				cycles = decode_nonprefixed(opcode);
			}

			if (cycles == 0)
			{
				printf("Error - 0 cycles returned from opcode\n");
			}

			return cycles;
		}
	}
}