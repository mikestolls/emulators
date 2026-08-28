#pragma once

#include "defines.h"

#include "memory_module.h"
#include "input.h"

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
	//int update_peripherals(u8 cycles);

	namespace gpu
	{
		int update(u8 cycles);
	}

	namespace cpu
	{
		// used for micro op processing
		enum MICRO_OP_TYPE
		{
			NOP,
			FETCH_OP,
			FETCH_PC,
			READ_REG_8,
			READ_REG_16,
			ASSIGN_REG_8,
			ASSIGN_REG_16,
			ADD_8,
			ADD_16,
			ADD_8_TO_16,
			READ_ADDR_8,
			WRITE_ADDR_8,
			JUMP,
			ROTATE_ACCUMULATOR,
			DAA,
			CPL,
			SCF,
			CCF,
			HALT,
			ALU,
			ROTATE_SHIFT,
			CONDITION,
			IME,
			TEST_BIT,
			RESET_BIT,
			SET_BIT
		};

		enum ROTATE_TYPE
		{
			LEFT_CIRCULAR,
			LEFT_THROUGH_CARRY,
			RIGHT_CIRCULAR,
			RIGHT_THROUGH_CARRY,
		};

		enum IME_MODE
		{
			DISABLE,
			ENABLE,
			ENABLE_DELAYED
		};

		struct MicroOp
		{			
			MICRO_OP_TYPE micro_op_type;
			u8* src_ptr = nullptr;
			u8* dest_ptr = nullptr;
			u16 dest_mask = 0xFFFF;
			u16 addr = 0;
			u16 addr_offset = 0x0;
			s16 value = 0;
			u8* value_ptr = nullptr;
			s8 src_modify = 0;
			s8 dest_modify = 0;
			u8 alu_rot_index = 0;
			u8 condition_index = 0;
			u8 condition_fail_pop_count = 0;
			u8 set_flags = 0x0;
			u8 reset_flags = 0x0;
			bool is_use_value = false;
			bool is_use_addr = false;
			bool is_signed = false;
			ROTATE_TYPE rotate_type = ROTATE_TYPE::LEFT_CIRCULAR;
		};

		bool is_opcode_complete = false;
		u8 last_opcode = 0x0;
		s32 last_temp_value = 0x0;
		u16 last_read_reg = 0x0;

		const u32 cycles_per_sec = 4194304;
		const u32 cycles_per_line = 456;
		const u32 lines_per_frame = 154;
		const u32 cycles_per_frame = cycles_per_line * lines_per_frame;  // = 70224
		const u32 fps = 60;

		bool running = true;
		u8 ei_occcurred = 0;
		bool halt = false;
		bool halt_bug = false;
		bool paused = false;
		bool is_double_speed = false;

		std::vector<u16> breakpoints;
		std::vector<u16> soft_breakpoints;
		std::vector<u16> memory_breakpoints;
		bool breakpoint_hit;
		bool breakpoint_disable_one_instr;
		s32 memory_breakpoint_last_addr;
		u16 memory_breakpoint_last_pc;

		bool interrupt_master;
		u8* interrupt_enable_flag;
		u8* interrupt_request_flag;

		s32 internal_divider;
		bool skip_next_tima_increment; // Skip next TIMA increment after write

		u8* timer_value;
		u8* timer_controller;
		u8* timer_modulator;
		u8 timer_last_bit;


		u8 timer_overflow_state = 0;

		u8* divide_value;

		u16 current_pc = 0x0;

		std::deque<MicroOp> micro_op_queue;
		
		// debug instruction timings
		static const int instruction_times_nocondition[] = {
			1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1,
			1, 3, 2, 2, 1, 1, 2, 1, 3, 2, 2, 2, 1, 1, 2, 1,
			2, 3, 2, 2, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 2, 1,
			2, 3, 2, 2, 3, 3, 3, 1, 2, 2, 2, 2, 1, 1, 2, 1,
			1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
			1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
			1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
			2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1,
			1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
			1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
			1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
			1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
			2, 3, 3, 4, 3, 4, 2, 4, 2, 4, 3, 0, 3, 6, 2, 4,
			2, 3, 3, 0, 3, 4, 2, 4, 2, 4, 3, 0, 3, 0, 2, 4,
			3, 3, 2, 0, 0, 4, 2, 4, 4, 1, 4, 0, 0, 0, 2, 4,
			3, 3, 2, 1, 0, 4, 2, 4, 3, 2, 4, 1, 0, 0, 2, 4,
		};

		static const int instruction_times_condition[] = {
			1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1,
			0, 3, 2, 2, 1, 1, 2, 1, 3, 2, 2, 2, 1, 1, 2, 1,
			3, 3, 2, 2, 1, 1, 2, 1, 3, 2, 2, 2, 1, 1, 2, 1,
			3, 3, 2, 2, 3, 3, 3, 1, 3, 2, 2, 2, 1, 1, 2, 1,
			1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
			1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
			1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
			2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1,
			1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
			1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
			1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
			1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
			5, 3, 4, 4, 6, 4, 2, 4, 5, 4, 4, 0, 6, 6, 2, 4,
			5, 3, 4, 0, 6, 4, 2, 4, 5, 4, 4, 0, 6, 0, 2, 4,
			3, 3, 2, 0, 0, 4, 2, 4, 4, 1, 4, 0, 0, 0, 2, 4,
			3, 3, 2, 1, 0, 4, 2, 4, 3, 2, 4, 1, 0, 0, 2, 4,
		};

		static const int instruction_times_cb[] = {
			2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
			2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
			2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
			2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
			2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
			2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
			2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
			2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
			2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
			2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
			2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
			2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
			2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
			2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
			2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
			2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
		};

		// cpus register structure
		struct Registers
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
		u8* register_single[] = { &R.b, &R.c, &R.d, &R.e, &R.h, &R.l, 0, &R.a };
		
		// set and get flag helpers
		inline void set_flag(u8& reg, u8 flag)
		{
			flag = (1 << flag);
			reg |= flag;
		}

		inline void clear_flag(u8& reg, u8 flag)
		{
			flag = (1 << flag);
			reg &= ~flag; // clear the bit
		}

		inline u8 get_flag(u8& reg, u8 flag)
		{
			return ((reg & (1 << flag)) >> flag);
		}

		// flag helpers to the register
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
			u8 val = memory_module::read_memory(R.pc++);

			return val;
		}

		inline u16 readpc_u16()
		{
			// lsb is first in memory
			u16 val = memory_module::read_memory(R.pc++);
			val |= (memory_module::read_memory(R.pc++) << 8);

			return val;
		}

		inline u16 get_current_pc()
		{
			return current_pc;
		}

		inline void set_double_speed(bool double_speed)
		{
			is_double_speed = false;
		}

		// interrupt functionality
		enum INTERRUPT_FLAG
		{
			INTERRUPT_VBLANK = 0,
			INTERRUPT_LCD,
			INTERRUPT_TIMER,
			INTERRUPT_SERIAL_IO_END,
			INTERRUPT_JOYPAD,
			INTERRUPT_COUNT
		};

		inline void set_request_interrupt_flag(u8 flag)
		{
			u8 flag_mask = (1 << flag);
			*interrupt_request_flag |= flag_mask;
			*interrupt_request_flag |= 0xE0;
		}

		inline void clear_request_interrupt_flag(u8 flag)
		{
			u8 flag_mask = (1 << flag);
			*interrupt_request_flag &= ~flag_mask; // clear the bit
			*interrupt_request_flag |= 0xE0;
		}

		inline u8 get_request_interrupt_flag(u8 flag)
		{
			return ((*interrupt_request_flag & (1 << flag)) >> flag);
		}

		inline void clear_all_request_interrupt_flags()
		{
			*interrupt_request_flag = 0xE0;
		}

		void request_joypad_interrupt()
		{
			set_request_interrupt_flag(cpu::INTERRUPT_JOYPAD);
		}

		// interrupt enable function
		inline void set_enabled_interrupt_flag(u8 flag)
		{
			u8 flag_mask = (1 << flag);
			*interrupt_enable_flag |= flag_mask;
		}

		inline void clear_enabled_interrupt_flag(u8 flag)
		{
			u8 flag_mask = (1 << flag);
			*interrupt_enable_flag &= ~flag_mask; // clear the bit
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
			interrupt_master = false; // servicing an interrupt will disable the master

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
			case INTERRUPT_SERIAL_IO_END:
				addr = 0x58;
				break;
			case INTERRUPT_JOYPAD:
				addr = 0x60;
				break;
			default:
				printf("Error - Trying to service an invalid interrupt: %d\n", interrupt);
				assert(0);
				break;
			}

			last_temp_value = addr;

			// STACK PUSH
			MicroOp write_high;
			write_high.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
			write_high.value_ptr = ((u8*)&R.pc) + 1; // high byte of PC
			write_high.dest_ptr = (u8*)&R.sp;
			write_high.addr_offset = -1;  // write to (sp-1)
			write_high.dest_modify = -1;  // decrement sp
			micro_op_queue.push_back(write_high);

			MicroOp write_low;
			write_low.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
			write_low.value_ptr = (u8*)&R.pc; // low byte of PC
			write_low.dest_ptr = (u8*)&R.sp;
			write_low.addr_offset = -1;  // write to (sp-1)
			write_low.dest_modify = -1;  // decrement sp
			micro_op_queue.push_back(write_low);

			MicroOp assign_pc;
			assign_pc.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_16;
			assign_pc.src_ptr = (u8*)&last_temp_value;
			assign_pc.dest_ptr = (u8*)&R.pc;
			micro_op_queue.push_back(assign_pc);
		}

		int check_interrupts()
		{
			if (interrupt_master == false)
			{
				return 0;
			}

			for (u8 i = 0; i < INTERRUPT_COUNT; i++)
			{
				if (get_request_interrupt_flag(i) && get_enabled_interrupt_flag(i))
				{
					if (interrupt_master)
					{
						u8 cycles = 20;

						if (halt)
						{
							cycles += 4;
							R.pc++;
						}

						//gameboy::update_peripherals(cycles);

						service_interrupt(i);
						clear_request_interrupt_flag(i);

						halt = false;

						return cycles;
					}
					else if (halt)
					{
						R.pc++;
						halt = false;
						return 0;
					}
					else
					{
						return 0;
					}
				}
			}

			return 0;
		}

		// functions for the timer
		void reset_divider()
		{
			// Check if the timer bit goes from 1->0 due to this reset
			// This can cause TIMA to increment immediately (falling edge detection)
			if (is_timer_enabled(*timer_controller))
			{
				u8 bit_index = get_timer_frequency_bit(*timer_controller);
				bool old_bit = (internal_divider >> bit_index) & 1;

				if (old_bit)  // If bit was 1, resetting to 0 causes falling edge
				{
					increment_tima();
				}
			}

			internal_divider = 0;
			*divide_value = 0x0;
			update_last_timer_bit();
		}

		s32 get_internal_divider()
		{
			return internal_divider;
		}
		bool is_timer_enabled(u8 timer_control)
		{
			return (timer_control & 0x4 ? true : false); // bit 2 is on/off flag
		}

		u32 get_timer_frequency_bit(u8 timer_control)
		{
			switch (timer_control & 0x3) // bit 0 and 1 are the frequency flags. cycles_per_sec / frequency
			{
			case 0: 
				return 9;   // 1024 T-cycles
			case 1: 
				return 3;   // 16 T-cycles
			case 2: 
				return 5;   // 64 T-cycles
			case 3: 
				return 7;   // 256 T-cycles
			}
		}

		bool is_tima_overflow_pending()
		{
			return timer_overflow_state > 0;
		}

		void cancel_tima_overflow()
		{
			timer_overflow_state = 0;
		}

		void increment_tima()
		{
			u8 old_value = *timer_value;
			(*timer_value)++;

			if (*timer_value == 0x00) // Overflow from 0xFF to 0x00
			{
				timer_overflow_state = 1;
			}
		}

		void update_last_timer_bit()
		{
			u8 old_bit = timer_last_bit;

			if (is_timer_enabled(*timer_controller))
			{
				u8 bit_index = get_timer_frequency_bit(*timer_controller);
				timer_last_bit = (internal_divider >> bit_index) & 0x1;
			}
			else
			{
				// When timer is disabled, the effective bit is always 0
				timer_last_bit = 0;
			}
		}

		void set_timer_last_bit(u8 value)
		{
			timer_last_bit = value;
		}

		void set_skip_next_tima_increment(bool value)
		{
			skip_next_tima_increment = value;
		}

		int update_timer(u8 cycles)
		{
			for (u8 i = 0; i < cycles; i++) // iterate oer t cycle
			{
				// handle pending reload from previous cycle
				if (i % 4 && timer_overflow_state == 1) // check on first t cycle
				{
					*timer_value = *timer_modulator;
					set_request_interrupt_flag(INTERRUPT_TIMER);
					timer_overflow_state = 0;
				}

				u8 old_tima = *timer_value;

				internal_divider++;

				// DIV is bits 15-8
				*divide_value = (internal_divider >> 8) & 0xFF;

				// only update timer_last_bit and check for edges if timer is enabled
				if (is_timer_enabled(*timer_controller))
				{
					u8 bit_index = get_timer_frequency_bit(*timer_controller);
					u8 current_bit = (internal_divider >> bit_index) & 0x1;

					// Falling edge detection (1→0)
					if (timer_last_bit == 1 && current_bit == 0)
					{
						increment_tima();
					}

					// Update timer_last_bit only when timer is enabled
					timer_last_bit = current_bit;
				}
			}

			return 0;
		}

		/*

		u32 get_timer_frequency()
		{
			u32 cycles = 0;

			switch (*timer_controller & 0x3) // bit 0 and 1 are the frequency flags. cycles_per_sec / frequency
			{
			case 0x0: // 4096 hz
				cycles = 1024;
				break;
			case 0x1: // 262144 hz
				cycles = 16;
				break;
			case 0x2: // 65536 hz
				cycles = 64;
				break;
			case 0x3: // 16384 hz
				cycles = 256;
				break;
			}

			return cycles;
		}

		static bool log_to_file = false;
		static bool timer_debug = false;
		static int total_cycles = 0;
		static int timer_overflow_delay = -1;
		static FILE* fptr = fopen("output.txt", "w");

		bool is_reset_timer = false;

		void reset_timer_counter()
		{
			//timer_counter = get_timer_frequency();
			//*timer_value = *timer_modulator;
			is_reset_timer = true;

			if (log_to_file)
			{
				fprintf(fptr, "reset_timer_counter: freq=%d TIMA=0x%02X TMA=0x%02X TAC=0x%02X PC=0x%04X\n",
					timer_counter, *timer_value, *timer_modulator, *timer_controller, R.pc);
			}
		}

		int update_timer(u8 cycles)
		{
			// Start debug when TAC is set to 0x05
			if (!timer_debug && *timer_controller == 0x05 && timer_enabled())
			{
				timer_debug = true;

				if (log_to_file)
				{
					fprintf(fptr, "\n=== TIMER DEBUG START ===\n");
					fprintf(fptr, "Initial: TIMA=0x%02X TMA=0x%02X TAC=0x%02X freq=%d counter=%d\n",
						*timer_value, *timer_modulator, *timer_controller, get_timer_frequency(), timer_counter);
				}
			}

			if (timer_debug)
			{
				total_cycles += cycles;
			}

			// Handle overflow delay first (if pending)
			if (timer_overflow_delay >= 0)
			{
				timer_overflow_delay -= cycles;
				if (timer_overflow_delay < 0)
				{
					// Reload TIMA with TMA and set interrupt
					*timer_value = *timer_modulator;
					set_request_interrupt_flag(INTERRUPT_TIMER);

					if (timer_debug && log_to_file)
					{
						fprintf(fptr, "[%d] OVERFLOW RELOAD: TIMA->0x%02X (TMA) IF=0x%02X\n",
							total_cycles, *timer_value, *interrupt_request_flag);
					}
					timer_overflow_delay = -1;  // Clear overflow state
				}
			}

			// update divide register first
			divide_counter -= cycles;

			while (divide_counter <= 0) // divide register is 16382 hz
			{
				(*divide_value)++;
				divide_counter += 256;
			}

			if (!timer_enabled())
			{
				return 0;
			}

			if (!is_reset_timer)
			{
				timer_counter -= cycles;

				while (timer_counter <= 0)
				{
					// check if overflow. set timer_counter to modulator. increase timer
					if (*timer_value == 0xFF)
					{
						*timer_value = *timer_modulator;

						// interrupt
						set_request_interrupt_flag(INTERRUPT_TIMER);

						*timer_value = 0x00;
						timer_overflow_delay = 4;  // 4 T-cycles until TMA reload

						if (timer_debug)
						{
							if (log_to_file)
							{
								fprintf(fptr, "[%d] OVERFLOW: TIMA 0xFF->0x%02X counter=%d IF=0x%02X\n",
									total_cycles, *timer_value, timer_counter, *interrupt_request_flag);
							}
						}
					}
					else
					{
						(*timer_value)++;
					}

					// set counter back to frequency
					timer_counter += get_timer_frequency();

					if (timer_debug && log_to_file)
					{
						u8 old_val = *timer_value - 1;
						fprintf(fptr, "[%d] TIMA: 0x%02X->0x%02X counter=%d\n",
							total_cycles, old_val, *timer_value, timer_counter);
					}
				}
			}

			if (timer_debug && log_to_file && total_cycles > 400)
			{
				timer_debug = false;
				fprintf(fptr, "=== TIMER DEBUG END ===\n\n");
			}

			if (is_reset_timer)
			{
				is_reset_timer = false;
				timer_counter = get_timer_frequency() * 2;
			}

			return 0;
		}
		*/

		int reset()
		{
			memset(&R, 0x0, sizeof(R)); // init registers to 0
			
			R.af = 0x0000;
			R.bc = 0x0000;
			R.de = 0x0000;
			R.hl = 0x0000;
			R.pc = 0x0000;
			R.sp = 0x0000;

			if (!memory_module::boot_ptr)
			{
				bool is_cgb_mode = (memory_module::rom_ptr->rom_header.cgb_flag == 0x80 ||
					memory_module::rom_ptr->rom_header.cgb_flag == 0xC0);

				if (is_cgb_mode)
				{
					// CGB initial register values
					R.af = 0x1180;  // A = 0x11 (CGB identifier), F = 0x80
					R.bc = 0x0000;
					R.de = 0xFF56;
					R.hl = 0x000D;
				}
				else
				{
					// DMG initial register values
					R.af = 0x01B0;
					R.bc = 0x0013;
					R.de = 0x00D8;
					R.hl = 0x014D;
				}

				R.pc = 0x0100; // starting entry point of the ROM
				R.sp = 0xFFFE;
			}

			memory_module::reset();

			interrupt_master = false;
			interrupt_enable_flag = memory_module::get_memory(0xFFFF);
			interrupt_request_flag = memory_module::get_memory(0xFF0F);

			timer_value = memory_module::get_memory(0xFF05);
			timer_modulator = memory_module::get_memory(0xFF06);
			timer_controller = memory_module::get_memory(0xFF07);
			
			divide_value = memory_module::get_memory(0xFF04);
			internal_divider = 0;
			timer_last_bit = 0;
			skip_next_tima_increment = false;
			timer_overflow_state = 0;

			is_opcode_complete = false;
			last_opcode = 0x0;
			last_temp_value = 0x0;
			last_read_reg = 0x0;

			paused = false;
			running = true;
			ei_occcurred = 0;
			halt = false;
			halt_bug = false;
			is_double_speed = false;
			breakpoint_hit = false;
			breakpoint_disable_one_instr = false;
			memory_breakpoint_last_addr = -1;
			memory_breakpoint_last_pc = -1;

			micro_op_queue.clear();

			return 0;
		}

		int initialize()
		{			
			reset();

			return 0;
		}

		bool check_memory_breakpoint(u16 pc, u16 addr)
		{
			if (addr == memory_breakpoint_last_addr)
			{
				memory_breakpoint_last_addr = -1;
				memory_breakpoint_last_pc = -1;
				return false;
			}

			if (memory_breakpoints.size() > 0)
			{
				auto memory_breakpoint_itr = std::find(memory_breakpoints.begin(), memory_breakpoints.end(), addr);
				if (memory_breakpoint_itr != memory_breakpoints.end())
				{
					paused = true;
					breakpoint_hit = true;
					memory_breakpoint_last_addr = addr;
					memory_breakpoint_last_pc = pc;

					R.pc = pc; // assuming we back 1 byte to previous opcode. assuming non prefix dont write memory
					soft_breakpoints.push_back(R.pc);
					return true;
				}
			}

			return false;
		}

		u8 opcode_state = 0;

		int decode_nonprefixed(u8 opcode)
		{
			u8 x = (opcode >> 6);
			u8 y = (opcode >> 3) & 0x7;
			u8 z = (opcode & 0x7);
			u8 p = (opcode >> 4) & 0x3;
			u8 q = (opcode >> 3) & 0x1;

			u8 cycles = 0;
			bool is_condition = false;

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
					{
						// NOP
						MicroOp op;
						op.micro_op_type = MICRO_OP_TYPE::NOP;
						micro_op_queue.push_back(op);

						break;
					}
					case 0x1:
					{
						// LD mem NN with SP
						MicroOp fetch_low;
						fetch_low.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch_low.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(fetch_low);

						MicroOp fetch_high;
						fetch_high.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch_high.dest_ptr = ((u8*)&last_temp_value) + 1;
						micro_op_queue.push_back(fetch_high);

						MicroOp write_low;
						write_low.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
						write_low.value_ptr = (u8*)&R.sp;
						write_low.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(write_low);

						MicroOp write_high;
						write_high.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
						write_high.value_ptr = ((u8*)&R.sp) + 1;
						write_high.dest_ptr = (u8*)&last_temp_value;
						write_high.addr_offset = 0x1; // writing second byte of sp
						micro_op_queue.push_back(write_high);

						break;
					}
					case 0x2:
					{
						// STOP
						// fetch and discard. next byte which should be 0x0
						MicroOp fetch;
						fetch.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(fetch);

						u8 key1 = memory_module::read_memory(0xFF4D);

						// CGB speed swap case
						if (key1 & 0x01)  // Bit 0 set = prepare speed switch
						{
							// CGB speed switch. toggle bit 7 (current speed). clear bit 0 - speed switch flag
							key1 ^= 0x80;
							key1 &= ~0x01;

							MicroOp write;
							write.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
							write.addr = 0xFF4D;
							write.value = key1;							
							write.is_use_value = true;
							micro_op_queue.push_back(write);
						}

						break;
					}
					case 0x3:
					{
						// JR d
						MicroOp fetch;
						fetch.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch.dest_ptr = (u8*)&last_temp_value;
						fetch.is_signed = true;
						micro_op_queue.push_back(fetch);

						MicroOp jump;
						jump.micro_op_type = MICRO_OP_TYPE::JUMP;
						jump.value_ptr = (u8*)&last_temp_value;
						jump.is_signed = true;
						micro_op_queue.push_back(jump);

						break;
					}
					case 0x4:
					case 0x5:
					case 0x6:
					case 0x7:
					{
						// JR conditions[y - 4], d - relative jump
						MicroOp fetch;
						fetch.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch.dest_ptr = (u8*)&last_temp_value;
						fetch.is_signed = true;
						micro_op_queue.push_back(fetch);

						MicroOp cond;
						cond.micro_op_type = MICRO_OP_TYPE::CONDITION;
						cond.condition_index = y - 4;
						cond.condition_fail_pop_count = 1; // pop jump if condition fails
						micro_op_queue.push_back(cond);

						if (condition_funct[y - 4]())
						{
							MicroOp jump;
							jump.micro_op_type = MICRO_OP_TYPE::JUMP;
							jump.value_ptr = (u8*)&last_temp_value;
							jump.is_signed = true;
							micro_op_queue.push_back(jump);
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
					{
						// LD register_pairs[p] with nn
						MicroOp fetch_low;
						fetch_low.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch_low.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(fetch_low);

						MicroOp fetch_high;
						fetch_high.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch_high.dest_ptr = ((u8*)&last_temp_value) + 1;
						micro_op_queue.push_back(fetch_high);

						MicroOp assign;
						assign.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_16;
						assign.src_ptr = (u8*)&last_temp_value;
						assign.dest_ptr = (u8*)register_pairs[p];
						micro_op_queue.push_back(assign);

						break;
					}
					case 0x1:
					{
						// ADD HL with register_pairs[p]
						MicroOp read;
						read.micro_op_type = MICRO_OP_TYPE::READ_REG_16;
						read.src_ptr = (u8*)register_pairs[p];
						read.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(read);

						u8 flag = 0x0;
						set_flag(flag, FLAG_HALFCARRY);
						set_flag(flag, FLAG_CARRY);

						u8 reset_flag = 0x0;
						set_flag(reset_flag, FLAG_SUBTRACTION);

						MicroOp add;
						add.micro_op_type = MICRO_OP_TYPE::ADD_16;
						add.src_ptr = (u8*)&R.hl;
						add.value_ptr = (u8*)&last_temp_value;
						add.dest_ptr = (u8*)&R.hl;
						add.set_flags = flag;
						add.reset_flags = reset_flag;
						micro_op_queue.push_back(add);

						break;
					}
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
						{
							// LD (BC) with A
							MicroOp read;
							read.micro_op_type = MICRO_OP_TYPE::READ_REG_8;
							read.src_ptr = (u8*)&R.a;
							read.dest_ptr = (u8*)&last_temp_value;
							micro_op_queue.push_back(read);

							MicroOp write;
							write.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
							write.value_ptr = (u8*)&last_temp_value;
							write.dest_ptr = (u8*)&R.bc;
							micro_op_queue.push_back(write);

							break;
						}
						case 0x1:
						{
							// LD (DE) with A
							MicroOp read;
							read.micro_op_type = MICRO_OP_TYPE::READ_REG_8;
							read.src_ptr = (u8*)&R.a;
							read.dest_ptr = (u8*)&last_temp_value;
							micro_op_queue.push_back(read);

							MicroOp write;
							write.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
							write.value_ptr = (u8*)&last_temp_value;
							write.dest_ptr = (u8*)&R.de;
							micro_op_queue.push_back(write);

							break;
						}
						case 0x2:
						{
							// LDI (HL) with A. inc HL
							MicroOp read;
							read.micro_op_type = MICRO_OP_TYPE::READ_REG_8;
							read.src_ptr = (u8*)&R.a;
							read.dest_ptr = (u8*)&last_temp_value;
							micro_op_queue.push_back(read);

							MicroOp write;
							write.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
							write.value_ptr = (u8*)&last_temp_value;
							write.dest_ptr = (u8*)&R.hl;
							write.dest_modify = 1;
							micro_op_queue.push_back(write);

							break;
						}
						case 0x3:
						{
							// LDD (HL) with A. decr HL
							MicroOp read;
							read.micro_op_type = MICRO_OP_TYPE::READ_REG_8;
							read.src_ptr = (u8*)&R.a;
							read.dest_ptr = (u8*)&last_temp_value;
							micro_op_queue.push_back(read);

							MicroOp write;
							write.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
							write.value_ptr = (u8*)&last_temp_value;
							write.dest_ptr = (u8*)&R.hl;
							write.dest_modify = -1;
							micro_op_queue.push_back(write);

							break;
						}
						}
						break;
					}
					case 0x1:
					{
						switch (p)
						{
						case 0x0:
						{
							// LD A with (BC)
							MicroOp read;
							read.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
							read.src_ptr = (u8*)&R.bc;
							read.dest_ptr = (u8*)&last_temp_value;
							micro_op_queue.push_back(read);

							MicroOp write;
							write.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_8;
							write.src_ptr = (u8*)&last_temp_value;
							write.dest_ptr = (u8*)&R.a;
							micro_op_queue.push_back(write);

							break;
						}
						case 0x1:
						{
							// LD A with (DE)
							MicroOp read;
							read.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
							read.src_ptr = (u8*)&R.de;
							read.dest_ptr = (u8*)&last_temp_value;
							micro_op_queue.push_back(read);

							MicroOp write;
							write.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_8;
							write.src_ptr = (u8*)&last_temp_value;
							write.dest_ptr = (u8*)&R.a;
							micro_op_queue.push_back(write);

							break;
						}
						case 0x2:
						{
							// LDI A with (HL). inc HL
							MicroOp read;
							read.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
							read.src_ptr = (u8*)&R.hl;
							read.dest_ptr = (u8*)&last_temp_value;
							read.src_modify = 1;
							micro_op_queue.push_back(read);

							MicroOp write;
							write.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_8;
							write.src_ptr = (u8*)&last_temp_value;
							write.dest_ptr = (u8*)&R.a;
							micro_op_queue.push_back(write);

							break;
						}
						case 0x3:
						{
							// LDD A with (HL). decr HL
							MicroOp read;
							read.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
							read.src_ptr = (u8*)&R.hl;
							read.dest_ptr = (u8*)&last_temp_value;
							read.src_modify = -1;
							micro_op_queue.push_back(read);

							MicroOp write;
							write.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_8;
							write.src_ptr = (u8*)&last_temp_value;
							write.dest_ptr = (u8*)&R.a;
							micro_op_queue.push_back(write);

							break;
						}
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
					{
						// INC register_pairs[p]
						MicroOp read;
						read.micro_op_type = MICRO_OP_TYPE::READ_REG_16;
						read.src_ptr = (u8*)register_pairs[p];
						read.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(read);

						MicroOp add;
						add.micro_op_type = MICRO_OP_TYPE::ADD_16;
						add.value = 1;
						add.is_use_value = true;
						add.src_ptr = (u8*)&last_temp_value;
						add.dest_ptr = (u8*)register_pairs[p];
						add.is_signed = true;
						micro_op_queue.push_back(add);

						break;
					}
					case 0x1:
					{
						// DEC register_pairs[p]
						MicroOp read;
						read.micro_op_type = MICRO_OP_TYPE::READ_REG_16;
						read.src_ptr = (u8*)register_pairs[p];
						read.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(read);

						MicroOp add;
						add.micro_op_type = MICRO_OP_TYPE::ADD_16;
						add.value = -1;
						add.is_use_value = true;
						add.src_ptr = (u8*)&last_temp_value;
						add.dest_ptr = (u8*)register_pairs[p];
						add.is_signed = true;
						micro_op_queue.push_back(add);

						break;
					}
					}
					break;
				}
				case 0x4: // z = 4
				{
					// INC register_single[y]
					if (y == 6) // using (HL) register
					{
						MicroOp read;
						read.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
						read.src_ptr = (u8*)&R.hl;
						read.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(read);

						u8 flag = 0x0;
						set_flag(flag, FLAG_ZERO);
						set_flag(flag, FLAG_SUBTRACTION);
						set_flag(flag, FLAG_HALFCARRY);

						u8 reset_flag = 0x0;
						set_flag(reset_flag, FLAG_SUBTRACTION);

						MicroOp inc;
						inc.micro_op_type = MICRO_OP_TYPE::ADD_8;
						inc.value = 1;
						inc.is_use_value = true;
						inc.src_ptr = (u8*)&last_temp_value; 
						inc.dest_ptr = (u8*)&last_temp_value;
						inc.is_signed = true;
						inc.set_flags = flag;
						inc.reset_flags = reset_flag;
						micro_op_queue.push_back(inc);

						MicroOp write;
						write.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
						write.value_ptr = (u8*)&last_temp_value;
						write.dest_ptr = (u8*)&R.hl;
						micro_op_queue.push_back(write);
					}
					else
					{
						MicroOp read;
						read.micro_op_type = MICRO_OP_TYPE::READ_REG_8;
						read.src_ptr = (u8*)register_single[y];
						read.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(read);

						u8 flag = 0x0;
						set_flag(flag, FLAG_ZERO);
						set_flag(flag, FLAG_SUBTRACTION);
						set_flag(flag, FLAG_HALFCARRY);

						u8 reset_flag = 0x0;
						set_flag(reset_flag, FLAG_SUBTRACTION);

						MicroOp inc;
						inc.micro_op_type = MICRO_OP_TYPE::ADD_8;
						inc.value = 1;
						inc.is_use_value = true;
						inc.src_ptr = (u8*)&last_temp_value;
						inc.dest_ptr = (u8*)register_single[y];
						inc.is_signed = true;
						inc.set_flags = flag;
						inc.reset_flags = reset_flag;
						micro_op_queue.push_back(inc);
					}

					break;
				}
				case 0x5: // z = 5
				{
					// DEC register_single[y]
					if (y == 6) // using (HL) register
					{
						MicroOp read;
						read.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
						read.src_ptr = (u8*)&R.hl;
						read.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(read);

						u8 flag = 0x0;
						set_flag(flag, FLAG_ZERO);
						set_flag(flag, FLAG_SUBTRACTION);
						set_flag(flag, FLAG_HALFCARRY);

						MicroOp inc;
						inc.micro_op_type = MICRO_OP_TYPE::ADD_8;
						inc.value = -1;
						inc.is_use_value = true;
						inc.src_ptr = (u8*)&last_temp_value;
						inc.dest_ptr = (u8*)&last_temp_value;
						inc.is_signed = true;
						inc.set_flags = flag;
						micro_op_queue.push_back(inc);

						MicroOp write;
						write.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
						write.value_ptr = (u8*)&last_temp_value;
						write.dest_ptr = (u8*)&R.hl;
						micro_op_queue.push_back(write);
					}
					else
					{
						MicroOp read;
						read.micro_op_type = MICRO_OP_TYPE::READ_REG_8;
						read.src_ptr = (u8*)register_single[y];
						read.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(read);

						u8 flag = 0x0;
						set_flag(flag, FLAG_ZERO);
						set_flag(flag, FLAG_SUBTRACTION);
						set_flag(flag, FLAG_HALFCARRY);

						MicroOp inc;
						inc.micro_op_type = MICRO_OP_TYPE::ADD_8;
						inc.value = -1;
						inc.is_use_value = true;
						inc.src_ptr = (u8*)&last_temp_value;
						inc.dest_ptr = (u8*)register_single[y];
						inc.is_signed = true;
						inc.set_flags = flag;
						micro_op_queue.push_back(inc);
					}

					break;
				}
				case 0x6: // z = 6
				{
					// LD register_single[y] with n
					MicroOp fetch;
					fetch.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
					fetch.dest_ptr = (u8*)&last_temp_value;
					micro_op_queue.push_back(fetch);

					if (y == 6) // register is (HL)
					{
						MicroOp write;
						write.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
						write.value_ptr = (u8*)&last_temp_value;
						write.dest_ptr = (u8*)&R.hl;
						micro_op_queue.push_back(write);
					}
					else
					{
						MicroOp write;
						write.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_8;
						write.src_ptr = (u8*)&last_temp_value;
						write.dest_ptr = (u8*)register_single[y];
						micro_op_queue.push_back(write);
					}

					break;
				}
				case 0x7: // z = 7
				{
					switch (y)
					{
					case 0x0:
					{
						// RLC A
						MicroOp rot;
						rot.micro_op_type = MICRO_OP_TYPE::ROTATE_ACCUMULATOR;
						rot.rotate_type = ROTATE_TYPE::LEFT_CIRCULAR;
						micro_op_queue.push_back(rot);

						break;
					}
					case 0x1:
					{
						// RRC A
						MicroOp rot;
						rot.micro_op_type = MICRO_OP_TYPE::ROTATE_ACCUMULATOR;
						rot.rotate_type = ROTATE_TYPE::RIGHT_CIRCULAR;
						micro_op_queue.push_back(rot);

						break;
					}
					case 0x2:
					{
						// RL A
						MicroOp rot;
						rot.micro_op_type = MICRO_OP_TYPE::ROTATE_ACCUMULATOR;
						rot.rotate_type = ROTATE_TYPE::LEFT_THROUGH_CARRY;
						micro_op_queue.push_back(rot);

						break;
					}
					case 0x3:
					{
						// RR A
						MicroOp rot;
						rot.micro_op_type = MICRO_OP_TYPE::ROTATE_ACCUMULATOR;
						rot.rotate_type = ROTATE_TYPE::RIGHT_THROUGH_CARRY;
						micro_op_queue.push_back(rot);

						break;
					}
					case 0x4:
					{
						// DAA
						MicroOp op;
						op.micro_op_type = MICRO_OP_TYPE::DAA;
						micro_op_queue.push_back(op);

						break;
					}
					case 0x5:
					{
						// CPL
						MicroOp op;
						op.micro_op_type = MICRO_OP_TYPE::CPL;
						micro_op_queue.push_back(op);

						break;
					}
					case 0x6:
					{
						// SCF
						MicroOp op;
						op.micro_op_type = MICRO_OP_TYPE::SCF;
						micro_op_queue.push_back(op);

						break;
					}
					case 0x7:
					{
						// CCF
						MicroOp op;
						op.micro_op_type = MICRO_OP_TYPE::CCF;
						micro_op_queue.push_back(op);

						break;
					}
					}
					break;
				}
				}
				break;
			} // end x = 0
			case 0x1: // x = 1
			{
				if (z == 6 && y == 6)
				{
					// HALT
					MicroOp op;
					op.micro_op_type = MICRO_OP_TYPE::HALT;
					micro_op_queue.push_back(op);
				}
				else if (y == 6)
				{
					// LD (HL), r
					MicroOp read;
					read.micro_op_type = MICRO_OP_TYPE::READ_REG_8;
					read.src_ptr = (u8*)register_single[z];
					read.dest_ptr = (u8*)&last_temp_value;
					micro_op_queue.push_back(read);

					MicroOp write;
					write.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
					write.value_ptr = (u8*)&last_temp_value;
					write.dest_ptr = (u8*)&R.hl;
					micro_op_queue.push_back(write);
				}
				else if (z == 6)
				{
					// LD r, (HL)
					MicroOp read;
					read.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
					read.src_ptr = (u8*)&R.hl;
					read.dest_ptr = (u8*)&last_temp_value;
					micro_op_queue.push_back(read);

					MicroOp write;
					write.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_8;
					write.src_ptr = (u8*)&last_temp_value;
					write.dest_ptr = (u8*)register_single[y];
					micro_op_queue.push_back(write);
				}
				else
				{
					// LD r, r'
					MicroOp read;
					read.micro_op_type = MICRO_OP_TYPE::READ_REG_8;
					read.src_ptr = (u8*)register_single[z];
					read.dest_ptr = (u8*)&last_temp_value;
					micro_op_queue.push_back(read);

					MicroOp write;
					write.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_8;
					write.src_ptr = (u8*)&last_temp_value;
					write.dest_ptr = (u8*)register_single[y];
					micro_op_queue.push_back(write);
				}

				break;
			} // end x = 1
			case 0x2: // x = 2
			{
				if (z == 6) // using (HL) register
				{
					MicroOp read;
					read.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
					read.src_ptr = (u8*)&R.hl;
					read.dest_ptr = (u8*)&last_temp_value;
					micro_op_queue.push_back(read);
				}
				else
				{
					MicroOp read;
					read.micro_op_type = MICRO_OP_TYPE::READ_REG_8;
					read.src_ptr = (u8*)register_single[z];
					read.dest_ptr = (u8*)&last_temp_value;
					micro_op_queue.push_back(read);
				}

				// ALU operation (A is always the destination)
				MicroOp alu;
				alu.micro_op_type = MICRO_OP_TYPE::ALU;
				alu.src_ptr = (u8*)&last_temp_value;
				alu.alu_rot_index = y;
				micro_op_queue.push_back(alu);

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
					{
						// RET if condition_funct[y]
						MicroOp cond;
						cond.micro_op_type = MICRO_OP_TYPE::CONDITION;
						cond.condition_index = y;
						cond.condition_fail_pop_count = 3; // if condition fails, pop 3 
						micro_op_queue.push_back(cond);

						// adding the 3 micro ops if condition passes. will clear them when ececuting condition if it fails
						MicroOp pop_lo;
						pop_lo.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
						pop_lo.src_ptr = (u8*)&R.sp;
						pop_lo.dest_ptr = (u8*)&last_temp_value;
						pop_lo.src_modify = 1;
						micro_op_queue.push_back(pop_lo);

						MicroOp pop_hi;
						pop_hi.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
						pop_hi.src_ptr = (u8*)&R.sp; // sp already modified
						pop_hi.dest_ptr = ((u8*)&last_temp_value) + 1;
						pop_hi.src_modify = 1;
						micro_op_queue.push_back(pop_hi);

						MicroOp assign_pc;
						assign_pc.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_16;
						assign_pc.src_ptr = (u8*)&last_temp_value;
						assign_pc.dest_ptr = (u8*)&R.pc;
						micro_op_queue.push_back(assign_pc);

						break;
					}
					case 0x4:
					{
						// LD mem(FF00 + n) with A
						MicroOp fetch;
						fetch.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(fetch);

						MicroOp add;
						add.micro_op_type = MICRO_OP_TYPE::ADD_16;
						add.is_use_value = true;
						add.value = 0xFF00;
						add.src_ptr = (u8*)&last_temp_value;
						add.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(add);

						MicroOp write;
						write.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
						write.value_ptr = (u8*)&R.a;
						write.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(write);

						break;
					}
					case 0x5:
					{
						// ADD SP with (signed)n
						MicroOp fetch;
						fetch.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(fetch);

						u8 flag = 0x0;
						set_flag(flag, FLAG_HALFCARRY);
						set_flag(flag, FLAG_CARRY);

						u8 reset_flag = 0x0;
						set_flag(reset_flag, FLAG_ZERO);
						set_flag(reset_flag, FLAG_SUBTRACTION);

						MicroOp add;
						add.micro_op_type = MICRO_OP_TYPE::ADD_8_TO_16;
						add.src_ptr = ((u8*)&R.sp);
						add.value_ptr = (u8*)&last_temp_value;
						add.dest_ptr = (u8*)&last_temp_value;
						add.is_signed = true;
						add.set_flags = flag;
						add.reset_flags = reset_flag;
						micro_op_queue.push_back(add);

						MicroOp assign;
						assign.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_16;
						assign.src_ptr = (u8*)&last_temp_value;
						assign.dest_ptr = (u8*)&R.sp;
						micro_op_queue.push_back(assign);

						// adding to pad timing
						MicroOp nop0;
						nop0.micro_op_type = MICRO_OP_TYPE::NOP;
						micro_op_queue.push_back(nop0);

						break;
					}
					case 0x6:
					{
						// LD A with mem(FF00 + n)
						MicroOp fetch;
						fetch.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(fetch);

						MicroOp add;
						add.micro_op_type = MICRO_OP_TYPE::ADD_16;
						add.is_use_value = true;
						add.value = 0xFF00;
						add.src_ptr = (u8*)&last_temp_value;
						add.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(add);

						MicroOp read;
						read.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
						read.src_ptr = (u8*)&last_temp_value;
						read.dest_ptr = (u8*)&R.a;
						micro_op_queue.push_back(read);

						break;
					}
					case 0x7:
					{
						// ADD (signed)n to SP then LD HL with SP
						MicroOp fetch;
						fetch.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(fetch);

						u8 flag = 0x0;
						set_flag(flag, FLAG_HALFCARRY);
						set_flag(flag, FLAG_CARRY);

						u8 reset_flag = 0x0;
						set_flag(reset_flag, FLAG_ZERO);
						set_flag(reset_flag, FLAG_SUBTRACTION);

						MicroOp add;
						add.micro_op_type = MICRO_OP_TYPE::ADD_8_TO_16;
						add.src_ptr = (u8*)&R.sp;
						add.value_ptr = (u8*)&last_temp_value;
						add.dest_ptr = (u8*)&last_temp_value;
						add.is_signed = true;
						add.set_flags = flag;
						add.reset_flags = reset_flag;
						micro_op_queue.push_back(add);

						MicroOp assign;
						assign.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_16;
						assign.src_ptr = (u8*)&last_temp_value;
						assign.dest_ptr = (u8*)&R.hl;
						micro_op_queue.push_back(assign);
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
						MicroOp read_lo;
						read_lo.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
						read_lo.src_ptr = (u8*)&R.sp;
						read_lo.dest_ptr = (u8*)&last_temp_value;
						read_lo.src_modify = 1; // increment SP after read
						micro_op_queue.push_back(read_lo);

						MicroOp read_hi;
						read_hi.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
						read_hi.src_ptr = (u8*)&R.sp;
						read_hi.dest_ptr = ((u8*)&last_temp_value) + 1;
						read_hi.src_modify = 1; // increment SP after read
						micro_op_queue.push_back(read_hi);

						MicroOp assign;
						assign.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_16;
						assign.src_ptr = (u8*)&last_temp_value;
						assign.dest_ptr = (u8*)register_pairs2[p];
						assign.dest_mask = (p == 3 ? 0xFFF0 : 0xFFFF); // p == 3 means its writing to AF so we mask
						micro_op_queue.push_back(assign);
					}
					else
					{
						switch (p)
						{
						case 0x0:
						{
							// RET
							MicroOp read_lo;
							read_lo.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
							read_lo.src_ptr = (u8*)&R.sp;
							read_lo.dest_ptr = (u8*)&last_temp_value;
							read_lo.src_modify = 1;
							micro_op_queue.push_back(read_lo);

							MicroOp read_hi;
							read_hi.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
							read_hi.src_ptr = (u8*)&R.sp;
							read_hi.dest_ptr = ((u8*)&last_temp_value) + 1;
							read_hi.src_modify = 1;
							micro_op_queue.push_back(read_hi);

							MicroOp assign;
							assign.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_16;
							assign.src_ptr = (u8*)&last_temp_value;
							assign.dest_ptr = (u8*)&R.pc;
							micro_op_queue.push_back(assign);

							MicroOp nop;
							nop.micro_op_type = MICRO_OP_TYPE::NOP;
							micro_op_queue.push_back(nop);

							break;
						}
						case 0x1:
						{
							// RETI
							MicroOp read_lo;
							read_lo.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
							read_lo.src_ptr = (u8*)&R.sp;
							read_lo.dest_ptr = (u8*)&last_temp_value;
							read_lo.src_modify = 1;
							micro_op_queue.push_back(read_lo);

							MicroOp read_hi;
							read_hi.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
							read_hi.src_ptr = (u8*)&R.sp;
							read_hi.dest_ptr = ((u8*)&last_temp_value) + 1;
							read_hi.src_modify = 1;
							micro_op_queue.push_back(read_hi);

							MicroOp assign;
							assign.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_16;
							assign.src_ptr = (u8*)&last_temp_value;
							assign.dest_ptr = (u8*)&R.pc;
							micro_op_queue.push_back(assign);

							MicroOp ime;
							ime.micro_op_type = MICRO_OP_TYPE::IME;
							ime.value = IME_MODE::ENABLE;
							ime.is_use_value = true;
							micro_op_queue.push_back(ime);

							break;
						}
						case 0x2:
						{
							// JP (HL)
							MicroOp assign;
							assign.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_16;
							assign.src_ptr = (u8*)&R.hl;
							assign.dest_ptr = (u8*)&R.pc;
							micro_op_queue.push_back(assign);

							break;
						}
						case 0x3:
						{
							// LD SP with HL
							MicroOp assign;
							assign.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_16;
							assign.src_ptr = (u8*)&R.hl;
							assign.dest_ptr = (u8*)&R.sp;
							micro_op_queue.push_back(assign);

							break;
						}
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
						// JP to nn if condition_funct[y]
						MicroOp fetch_low;
						fetch_low.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch_low.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(fetch_low);

						MicroOp fetch_high;
						fetch_high.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch_high.dest_ptr = ((u8*)&last_temp_value) + 1;
						micro_op_queue.push_back(fetch_high);

						MicroOp cond;
						cond.micro_op_type = MICRO_OP_TYPE::CONDITION;
						cond.condition_index = y;
						cond.condition_fail_pop_count = 1;
						micro_op_queue.push_back(cond);

						MicroOp assign_pc;
						assign_pc.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_16;
						assign_pc.src_ptr = (u8*)&last_temp_value;
						assign_pc.dest_ptr = (u8*)&R.pc;
						micro_op_queue.push_back(assign_pc);

						break;
					}
					case 0x4:
					{
						// LD mem(FF00 + C) with A
						MicroOp read;
						read.micro_op_type = MICRO_OP_TYPE::READ_REG_8;
						read.src_ptr = (u8*)&R.c;
						read.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(read);

						MicroOp write;
						write.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
						write.value_ptr = (u8*)&R.a;
						write.dest_ptr = (u8*)&last_temp_value;
						write.addr_offset = 0xFF00;
						micro_op_queue.push_back(write);

						break;
					}
					case 0x5:
					{
						// LD mem(nn) with A
						MicroOp fetch_low;
						fetch_low.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch_low.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(fetch_low);

						MicroOp fetch_high;
						fetch_high.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch_high.dest_ptr = ((u8*)&last_temp_value) + 1;
						micro_op_queue.push_back(fetch_high);

						MicroOp read_a;
						read_a.micro_op_type = MICRO_OP_TYPE::READ_REG_8;
						read_a.src_ptr = (u8*)&R.a; // just reading not storing
						micro_op_queue.push_back(read_a);

						MicroOp write;
						write.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
						write.value_ptr = (u8*)&R.a; // value to write
						write.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(write);

						break;
					}
					case 0x6:
					{
						// LD A with mem(FF00 + C)
						MicroOp load;
						load.micro_op_type = MICRO_OP_TYPE::READ_REG_8;
						load.src_ptr = (u8*)&R.c;
						load.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(load);

						MicroOp read;
						read.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
						read.src_ptr = (u8*)&last_temp_value;
						read.dest_ptr = (u8*)&R.a;
						read.addr_offset = 0xFF00;
						micro_op_queue.push_back(read);

						break;
					}
					case 0x7:
					{
						// LD A with mem(nn)
						MicroOp fetch_low;
						fetch_low.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch_low.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(fetch_low);

						MicroOp fetch_high;
						fetch_high.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch_high.dest_ptr = ((u8*)&last_temp_value) + 1;
						micro_op_queue.push_back(fetch_high);

						MicroOp read;
						read.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
						read.src_ptr = (u8*)&last_temp_value;
						read.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(read);

						MicroOp assign;
						assign.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_8;
						assign.src_ptr = (u8*)&last_temp_value;
						assign.dest_ptr = (u8*)&R.a;
						micro_op_queue.push_back(assign);

						break;
					}
					}
					break;
				}
				case 0x3: // z = 3
				{
					switch (y)
					{
					case 0x0:
					{
						// JP nn
						MicroOp fetch_low;
						fetch_low.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch_low.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(fetch_low);

						MicroOp fetch_high;
						fetch_high.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch_high.dest_ptr = ((u8*)&last_temp_value) + 1;
						micro_op_queue.push_back(fetch_high);

						MicroOp nop;
						nop.micro_op_type = MICRO_OP_TYPE::NOP;
						micro_op_queue.push_back(nop);

						MicroOp assign_pc;
						assign_pc.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_16;
						assign_pc.src_ptr = (u8*)&last_temp_value;
						assign_pc.dest_ptr = (u8*)&R.pc;
						micro_op_queue.push_back(assign_pc);

						break;
					}
					case 0x1:
					{
						// CB prefix
						printf("Error - CB prefix opcode should not get here\n");
						assert(0);
						break;
					}
					case 0x2:
					case 0x3:
					case 0x4:
					case 0x5:
					{
						// unsupported by gameboy
						running = false;

						break;
					}
					case 0x6:
					{
						// DI - disable interupts
						MicroOp ime;
						ime.micro_op_type = MICRO_OP_TYPE::IME;
						ime.value = IME_MODE::DISABLE;
						ime.is_use_value = true;
						micro_op_queue.push_back(ime);

						break;
					}
					case 0x7:
					{
						// EI - enable interupts
						MicroOp ime;
						ime.micro_op_type = MICRO_OP_TYPE::IME;
						ime.value = IME_MODE::ENABLE_DELAYED;
						ime.is_use_value = true;
						micro_op_queue.push_back(ime);

						break;
					}
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
						MicroOp fetch_low;
						fetch_low.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch_low.dest_ptr = (u8*)&last_temp_value;
						micro_op_queue.push_back(fetch_low);

						MicroOp fetch_high;
						fetch_high.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
						fetch_high.dest_ptr = ((u8*)&last_temp_value) + 1;
						micro_op_queue.push_back(fetch_high);

						MicroOp cond;
						cond.micro_op_type = MICRO_OP_TYPE::CONDITION;
						cond.condition_index = y;
						cond.condition_fail_pop_count = 3;
						micro_op_queue.push_back(cond);

						// STACK PUSH
						MicroOp write_high;
						write_high.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
						write_high.value_ptr = ((u8*)&R.pc) + 1; // high byte of PC
						write_high.dest_ptr = (u8*)&R.sp;
						write_high.addr_offset = -1;  // write to (sp-1)
						write_high.dest_modify = -1;  // decrement sp
						micro_op_queue.push_back(write_high);

						MicroOp write_low;
						write_low.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
						write_low.value_ptr = (u8*)&R.pc; // low byte of PC
						write_low.dest_ptr = (u8*)&R.sp;
						write_low.addr_offset = -1;  // write to (sp-1)
						write_low.dest_modify = -1;  // decrement sp
						micro_op_queue.push_back(write_low);

						MicroOp assign_pc;
						assign_pc.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_16;
						assign_pc.src_ptr = (u8*)&last_temp_value;
						assign_pc.dest_ptr = (u8*)&R.pc;
						micro_op_queue.push_back(assign_pc);

						break;
					}
					case 0x4:
					case 0x5:
					case 0x6:
					case 0x7:
					{
						// unsupported by gameboy
						running = false;

						break;
					}
					}
					break;
				}
				case 0x5: // z = 5
				{
					if (q == 0)
					{
						// PUSH register_pairs2[p]
						// STACK PUSH
						MicroOp write_high;
						write_high.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
						write_high.value_ptr = ((u8*)register_pairs2[p]) + 1; // high byte of reg
						write_high.dest_ptr = (u8*)&R.sp;
						write_high.addr_offset = -1;  // write to (sp-1)
						write_high.dest_modify = -1;  // decrement sp
						micro_op_queue.push_back(write_high);

						MicroOp write_low;
						write_low.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
						write_low.value_ptr = (u8*)register_pairs2[p]; // low byte of reg
						write_low.dest_ptr = (u8*)&R.sp;
						write_low.addr_offset = -1;  // write to (sp-1)
						write_low.dest_modify = -1;  // decrement sp
						micro_op_queue.push_back(write_low);
					}
					else
					{
						if (p == 0)
						{
							// CALL nn
							MicroOp fetch_low;
							fetch_low.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
							fetch_low.dest_ptr = (u8*)&last_temp_value;
							micro_op_queue.push_back(fetch_low);

							MicroOp fetch_high;
							fetch_high.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
							fetch_high.dest_ptr = ((u8*)&last_temp_value) + 1;
							micro_op_queue.push_back(fetch_high);

							// STACK PUSH
							MicroOp write_high;
							write_high.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
							write_high.value_ptr = ((u8*)&R.pc) + 1; // high byte of PC
							write_high.dest_ptr = (u8*)&R.sp;
							write_high.addr_offset = -1;  // write to (sp-1)
							write_high.dest_modify = -1;  // decrement sp
							micro_op_queue.push_back(write_high);

							MicroOp write_low;
							write_low.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
							write_low.value_ptr = (u8*)&R.pc; // low byte of PC
							write_low.dest_ptr = (u8*)&R.sp;
							write_low.addr_offset = -1;  // write to (sp-1)
							write_low.dest_modify = -1;  // decrement sp
							micro_op_queue.push_back(write_low);

							MicroOp assign_pc;
							assign_pc.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_16;
							assign_pc.src_ptr = (u8*)&last_temp_value;
							assign_pc.dest_ptr = (u8*)&R.pc;
							micro_op_queue.push_back(assign_pc);
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
					MicroOp fetch;
					fetch.micro_op_type = MICRO_OP_TYPE::FETCH_PC;
					fetch.dest_ptr = (u8*)&last_temp_value;
					micro_op_queue.push_back(fetch);

					MicroOp alu;
					alu.micro_op_type = MICRO_OP_TYPE::ALU;
					alu.src_ptr = (u8*)&last_temp_value;
					alu.alu_rot_index = y;
					micro_op_queue.push_back(alu);

					break;
				}
				case 0x7: // z = 7
				{
					// Debug: print RST info
					u8 rst_opcode = 0xC7 | (y << 3);
					printf("RST instruction: opcode=0x%02X, y=%d, PC=0x%04X\n", rst_opcode, y, R.pc);

					// RST at pc 7 * 8. basically a CALL
					MicroOp nop;
					nop.micro_op_type = MICRO_OP_TYPE::NOP;
					micro_op_queue.push_back(nop);

					// STACK PUSH
					MicroOp write_high;
					write_high.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
					write_high.value_ptr = ((u8*)&R.pc) + 1; // high byte of PC
					write_high.dest_ptr = (u8*)&R.sp;
					write_high.addr_offset = -1;  // write to (sp-1)
					write_high.dest_modify = -1;  // decrement sp
					micro_op_queue.push_back(write_high);

					MicroOp write_low;
					write_low.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
					write_low.value_ptr = (u8*)&R.pc; // low byte of PC
					write_low.dest_ptr = (u8*)&R.sp;
					write_low.addr_offset = -1;  // write to (sp-1)
					write_low.dest_modify = -1;  // decrement sp
					micro_op_queue.push_back(write_low);

					MicroOp assign_pc;
					assign_pc.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_16;
					assign_pc.value = y * 8;
					assign_pc.is_use_value = true;
					assign_pc.dest_ptr = (u8*)&R.pc;
					micro_op_queue.push_back(assign_pc);

					break;
				}
				}
				break;
			}
			}

			/*if (is_condition)
			{
				assert(cycles / 4 == instruction_times_condition[opcode]);
			}
			else
			{
				assert(cycles / 4 == instruction_times_nocondition[opcode]);
			}*/

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
			{
				// rot_function[y] with register_single[z]
				u8 val;
				if (z == 6) // (HL) register
				{
					MicroOp read;
					read.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
					read.src_ptr = (u8*)&R.hl;
					read.dest_ptr = (u8*)&last_temp_value;
					micro_op_queue.push_back(read);

					MicroOp rot;
					rot.micro_op_type = MICRO_OP_TYPE::ROTATE_SHIFT;
					rot.src_ptr = (u8*)&last_temp_value;
					rot.dest_ptr = (u8*)&last_temp_value;
					rot.alu_rot_index = y;
					micro_op_queue.push_back(rot);

					MicroOp write;
					write.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
					write.value_ptr = (u8*)&last_temp_value;
					write.dest_ptr = (u8*)&R.hl;
					micro_op_queue.push_back(write);
				}
				else
				{
					MicroOp read;
					read.micro_op_type = MICRO_OP_TYPE::READ_REG_8;
					read.src_ptr = register_single[z];
					read.dest_ptr = (u8*)&last_temp_value;
					micro_op_queue.push_back(read);

					MicroOp rot;
					rot.micro_op_type = MICRO_OP_TYPE::ROTATE_SHIFT; 
					rot.src_ptr = (u8*)&last_temp_value;
					rot.dest_ptr = (u8*)&last_temp_value;
					rot.alu_rot_index = y;
					micro_op_queue.push_back(rot);

					MicroOp write;
					write.micro_op_type = MICRO_OP_TYPE::ASSIGN_REG_8;
					write.src_ptr = (u8*)&last_temp_value;
					write.dest_ptr = register_single[z];
					micro_op_queue.push_back(write);
				}

				break;
			}
			case 0x1:
			{
				// test bit y from register_single[z]
				u8 val;
				if (z == 6) // (HL) register
				{
					MicroOp read;
					read.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
					read.src_ptr = (u8*)&R.hl;
					read.dest_ptr = (u8*)&last_temp_value;
					micro_op_queue.push_back(read);

					MicroOp test_bit;
					test_bit.micro_op_type = MICRO_OP_TYPE::TEST_BIT;
					test_bit.src_ptr = (u8*)&last_temp_value;
					test_bit.value = y; // bit index
					micro_op_queue.push_back(test_bit);
				}
				else
				{
					MicroOp test_bit;
					test_bit.micro_op_type = MICRO_OP_TYPE::TEST_BIT;
					test_bit.src_ptr = register_single[z];
					test_bit.value = y; // bit index
					micro_op_queue.push_back(test_bit);
				}

				break;
			}
			case 0x2:
			{
				// reset bit y from register_single[z]
				if (z == 6) // (HL) register
				{
					MicroOp read;
					read.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
					read.src_ptr = (u8*)&R.hl;
					read.dest_ptr = (u8*)&last_temp_value;
					micro_op_queue.push_back(read);

					MicroOp reset_bit;
					reset_bit.micro_op_type = MICRO_OP_TYPE::RESET_BIT;
					reset_bit.src_ptr = (u8*)&last_temp_value;
					reset_bit.value = y; // bit index
					micro_op_queue.push_back(reset_bit);

					MicroOp write;
					write.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
					write.value_ptr = (u8*)&last_temp_value;
					write.dest_ptr = (u8*)&R.hl;
					micro_op_queue.push_back(write);
				}
				else
				{
					MicroOp reset_bit;
					reset_bit.micro_op_type = MICRO_OP_TYPE::RESET_BIT;
					reset_bit.src_ptr = register_single[z];
					reset_bit.value = y; // bit index
					micro_op_queue.push_back(reset_bit);
				}

				break;
			}
			case 0x3:
			{
				// reset bit y from register_single[z]
				if (z == 6) // (HL) register
				{
					MicroOp read;
					read.micro_op_type = MICRO_OP_TYPE::READ_ADDR_8;
					read.src_ptr = (u8*)&R.hl;
					read.dest_ptr = (u8*)&last_temp_value;
					micro_op_queue.push_back(read);

					MicroOp reset_bit;
					reset_bit.micro_op_type = MICRO_OP_TYPE::SET_BIT;
					reset_bit.src_ptr = (u8*)&last_temp_value;
					reset_bit.value = y; // bit index
					micro_op_queue.push_back(reset_bit);

					MicroOp write;
					write.micro_op_type = MICRO_OP_TYPE::WRITE_ADDR_8;
					write.value_ptr = (u8*)&last_temp_value;
					write.dest_ptr = (u8*)&R.hl;
					micro_op_queue.push_back(write);
				}
				else
				{
					MicroOp reset_bit;
					reset_bit.micro_op_type = MICRO_OP_TYPE::SET_BIT;
					reset_bit.src_ptr = register_single[z];
					reset_bit.value = y; // bit index
					micro_op_queue.push_back(reset_bit);
				}

				break;
			}
			}

			//assert(cycles / 4 == instruction_times_cb[opcode]);

			return cycles;
		}
		
		int execute_opcode()
		{
			if (!running || (paused && !breakpoint_disable_one_instr))
			{
				// processor is stopped
				return 0;
			}

			// check for hitting breakpoints to pause
			if (!breakpoint_disable_one_instr)
			{
				if (breakpoints.size() > 0)
				{
					auto breakpoint_itr = std::find(breakpoints.begin(), breakpoints.end(), R.pc);
					if (breakpoint_itr != breakpoints.end())
					{
						paused = true;
						breakpoint_hit = true;
						return 0;
					}
				}

				// soft breakpoints are used for step over. not visible
				if (soft_breakpoints.size() > 0)
				{
					auto breakpoint_itr = std::find(soft_breakpoints.begin(), soft_breakpoints.end(), R.pc);
					if (breakpoint_itr != soft_breakpoints.end())
					{
						if (memory_breakpoint_last_addr == -1) // hacky to get mem breakpoints working
						{
							paused = true;
							breakpoint_hit = true;
						}

						soft_breakpoints.erase(breakpoint_itr);
						return 0;
					}
				}
			}

			if (breakpoint_disable_one_instr)
			{
				breakpoint_hit = true;
				breakpoint_disable_one_instr = false;
			}
			
			// update the joypad register
			/*u8 joypad_register = memory_module::read_memory(0xFF00);
			joypad_register &= 0xF0; // keep upper bits

			if ((joypad_register & 0x20) == 0)
			{
				// directional keys are set
				joypad_register |= (get_button_register(false) & 0xF); // only lower 4 bits
			}
			else
			{
				joypad_register |= (get_button_register(true) & 0xF); // only lower 4 bits
			}
			memory_module::write_memory(0xFF00, joypad_register);*/

			u8 cycles = 0;

			// fetch the opcode
			current_pc = R.pc;
			u8 opcode = readpc_u8();

			if (halt_bug)
			{
				R.pc--;
				halt_bug = false;
			}

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

		int execute_micro_op(MicroOp& op)
		{
			switch (op.micro_op_type)
			{
			case MICRO_OP_TYPE::NOP:
			{
				// do nothing
				break;
			}
			case MICRO_OP_TYPE::FETCH_OP:
			{
				// decode. gameboy only has CB prefix
				if (last_opcode == 0xCB)
				{
					u8 opcode = readpc_u8();
					decode_prefixed_cb(opcode); // decode functions will push their own operations
				}
				else
				{
					current_pc = R.pc;
					u8 opcode = readpc_u8();

					if (opcode == 0xCB)
					{
						// if CB, add new fetch to the queue
						MicroOp fetch_op;
						fetch_op.micro_op_type = MICRO_OP_TYPE::FETCH_OP;
						fetch_op.src_ptr = (u8*)&R.pc;

						micro_op_queue.push_back(fetch_op);
					}
					else
					{
						if (halt_bug)
						{
							R.pc--;
							halt_bug = false;
						}

						decode_nonprefixed(opcode);
					}

					// store last op code mainly for CB fetching
					last_opcode = opcode;
				}

				// set this as we have started a new opcode
				is_opcode_complete = false;

				break;
			}
			case MICRO_OP_TYPE::FETCH_PC:
			{
				// fetch next 8 bit from PC. store in last_fetch value
				current_pc = R.pc;
				s32 value = 0x0;

				if (op.is_signed)
				{
					value = (s8)readpc_u8();
				}
				else
				{
					value = readpc_u8();
				}

				if (op.dest_ptr != nullptr)
				{
					*op.dest_ptr = (u8)value;
				}

				break;
			}
			case MICRO_OP_TYPE::READ_REG_8:
			{
				u8 value = 0x0;
				if (op.src_ptr != nullptr)
				{
					value = *op.src_ptr;
				}

				if (op.dest_ptr != nullptr)
				{
					*op.dest_ptr = value;
				}

				break;
			}
			case MICRO_OP_TYPE::READ_REG_16:
			{
				u16 value = 0x0;
				if (op.src_ptr != nullptr)
				{
					value = *((u16*)op.src_ptr);
				}

				if (op.dest_ptr != nullptr)
				{
					*((u16*)op.dest_ptr) = value;
				}

				break;
			}
			case MICRO_OP_TYPE::ASSIGN_REG_8:
			{
				u8 value = 0x0;
				if (op.is_use_value)
				{
					value = (u8)op.value;
				}
				else if (op.src_ptr != nullptr)
				{
					value = *op.src_ptr;
				}

				if (op.dest_ptr != nullptr)
				{
					*op.dest_ptr = value & op.dest_mask;
				}

				break;
			}
			case MICRO_OP_TYPE::ASSIGN_REG_16:
			{
				u16 value = op.value;
				if (op.is_use_value)
				{
					value = (u16)op.value;
				}
				else if (op.src_ptr != nullptr)
				{
					value = (u16)*((u16*)op.src_ptr);
				}
				
				if (op.dest_ptr != nullptr)
				{
					*((u16*)op.dest_ptr) = value & op.dest_mask;
				}

				break;
			}
			case MICRO_OP_TYPE::ADD_8:
			{
				if (op.dest_ptr == nullptr)
				{
					assert(false);
				}

				if (op.src_ptr == nullptr)
				{
					assert(false);
				}

				s32 original = (s8)*((u8*)op.src_ptr);
				s32 value = 0x0;

				if (op.is_use_value) // if use value we add to total value
				{
					value = (s8)op.value;
				}
				else if (op.value_ptr != nullptr) // if we passed pointer to value
				{
					if (op.is_signed) // note: is signed only works with 8 bit value ptr
					{
						value = (s8)*((s8*)op.value_ptr);
					}
					else
					{
						value = (s8)(*((u8*)op.value_ptr));
					}
				}

				// check for carry
				if (get_flag(op.set_flags, FLAG_CARRY) != 0)
				{
					clear_flag(FLAG_CARRY);

					if ((original & 0xF) + (value & 0xF) > 0xF)
					{
						set_flag(FLAG_CARRY);
					}
				}

				// check for the half carry.				
				if (get_flag(op.set_flags, FLAG_HALFCARRY) != 0)
				{
					clear_flag(FLAG_HALFCARRY);

					if (value >= 0)
					{
						if ((original & 0xF) + (value & 0xF) > 0xF)
						{
							set_flag(FLAG_HALFCARRY);
						}
					}
					else
					{
						if ((original & 0xF) == 0)
						{
							set_flag(FLAG_HALFCARRY);
						}
					}
				}

				// Subtract flag
				if (get_flag(op.set_flags, FLAG_SUBTRACTION) != 0)
				{
					if (value < 0)
					{
						set_flag(FLAG_SUBTRACTION);
					}
					else if (value < 0)
					{
						clear_flag(FLAG_SUBTRACTION);
					}
				}

				// Zero flag
				if (get_flag(op.set_flags, FLAG_ZERO) != 0)
				{
					if (((original + (u8)value) & 0xFF) == 0)
					{
						set_flag(FLAG_ZERO);
					}
					else
					{
						clear_flag(FLAG_ZERO);
					}
				}

				// check for which flags need to be reset
				for (u8 i = FLAG_CARRY; i <= FLAG_ZERO; i++)
				{
					if (get_flag(op.reset_flags, i) != 0)
					{
						clear_flag(i);
					}
				}

				// set result
				original += value;
				*op.dest_ptr = (u8)(original & 0xFF);

				break;
			}
			case MICRO_OP_TYPE::ADD_16:
			{
				if (op.dest_ptr == nullptr)
				{
					assert(false);
				}

				if (op.src_ptr == nullptr)
				{
					assert(false);
				}

				s32 original = *((u16*)op.src_ptr);
				s32 value = 0x0;
				if (op.is_use_value) // if use value we add to total value
				{
					value = (s16)op.value;
				}
				else if (op.value_ptr != nullptr) // if we passed pointer to value
				{
					if (op.is_signed) // note: is signed only works with 8 bit value ptr
					{
						value = (s8)*((s8*)op.value_ptr);
					}
					else
					{
						value = (u16)(*((u16*)op.value_ptr));
					}
				}

				// check for carry
				if (get_flag(op.set_flags, FLAG_CARRY) != 0)
				{
					clear_flag(FLAG_CARRY);

					if ((original & 0xFFFF) + (value & 0xFFFF) > 0xFFFF)
					{
						set_flag(FLAG_CARRY);
					}
				}

				// check for the half carry.
				if (get_flag(op.set_flags, FLAG_HALFCARRY) != 0)
				{
					clear_flag(FLAG_HALFCARRY);

					if ((original & 0xFFF) + (value & 0xFFF) > 0xFFF)
					{
						set_flag(FLAG_HALFCARRY);
					}
				}

				// subtract flag
				if (get_flag(op.set_flags, FLAG_SUBTRACTION) != 0)
				{
					if (value < 0)
					{
						set_flag(FLAG_SUBTRACTION);
					}
					else if (value < 0)
					{
						clear_flag(FLAG_SUBTRACTION);
					}
				}

				// Zero flag
				if (get_flag(op.set_flags, FLAG_ZERO) != 0)
				{
					if (((original + (u8)value) & 0xFFFF) == 0)
					{
						set_flag(FLAG_ZERO);
					}
					else
					{
						clear_flag(FLAG_ZERO);
					}
				}

				// check for which flags need to be reset
				for (u8 i = FLAG_CARRY; i <= FLAG_ZERO; i++)
				{
					if (get_flag(op.reset_flags, i) != 0)
					{
						clear_flag(i);
					}
				}

				// set the result
				original += value;
				*((u16*)op.dest_ptr) = (u16)(original & 0xFFFF);

				break;
			}
			case MICRO_OP_TYPE::ADD_8_TO_16:
			{
				if (op.dest_ptr == nullptr)
				{
					assert(false);
				}

				if (op.src_ptr == nullptr)
				{
					assert(false);
				}

				s32 original = *((u16*)op.src_ptr);
				s32 value = 0x0;
				if (op.is_use_value) // if use value we add to total value
				{
					value = (s16)op.value;
				}
				else if (op.value_ptr != nullptr) // if we passed pointer to value
				{
					if (op.is_signed) // note: is signed only works with 8 bit value ptr
					{
						value = (s8)*((s8*)op.value_ptr);
					}
					else
					{
						value = (u16)(*((u16*)op.value_ptr));
					}
				}

				// check for carry
				if (get_flag(op.set_flags, FLAG_CARRY) != 0)
				{
					clear_flag(FLAG_CARRY);

					if ((original & 0xFF) + (value & 0xFF) > 0xFF)
					{
						set_flag(FLAG_CARRY);
					}
				}

				// check for the half carry.
				if (get_flag(op.set_flags, FLAG_HALFCARRY) != 0)
				{
					clear_flag(FLAG_HALFCARRY);

					if ((original & 0xF) + (value & 0xF) > 0xF)
					{
						set_flag(FLAG_HALFCARRY);
					}
				}

				// subtract flag
				if (get_flag(op.set_flags, FLAG_SUBTRACTION) != 0)
				{
					if (value < 0)
					{
						set_flag(FLAG_SUBTRACTION);
					}
					else if (value < 0)
					{
						clear_flag(FLAG_SUBTRACTION);
					}
				}

				// Zero flag
				if (get_flag(op.set_flags, FLAG_ZERO) != 0)
				{
					if (((original + (u8)value) & 0xFFFF) == 0)
					{
						set_flag(FLAG_ZERO);
					}
					else
					{
						clear_flag(FLAG_ZERO);
					}
				}

				// check for which flags need to be reset
				for (u8 i = FLAG_CARRY; i <= FLAG_ZERO; i++)
				{
					if (get_flag(op.reset_flags, i) != 0)
					{
						clear_flag(i);
					}
				}

				// set the result
				original += value;
				*((u16*)op.dest_ptr) = (u16)(original & 0xFFFF);

				break;
			}
			case MICRO_OP_TYPE::READ_ADDR_8:
			{
				// handle the read from address. either from op value or last fetch value
				u32 addr = 0x0;
				if (op.is_use_addr)
				{
					addr = op.addr;
				}
				else if (op.src_ptr != nullptr)
				{
					addr = *((u16*)op.src_ptr);
				}

				addr += op.addr_offset; // add a addr offset. mostly used for 0xFF00 + n ops

				if (check_memory_breakpoint(current_pc, addr))
				{
					return 0;
				}

				if (op.dest_ptr != nullptr)
				{
					*op.dest_ptr = memory_module::read_memory(addr);
				}

				// apply modifier for LDI  and LDD
				if (op.src_ptr != nullptr)
				{
					*((u16*)op.src_ptr) += op.src_modify;
				}

				// apply modifier for LDI  and LDD
				if (op.dest_ptr != nullptr)
				{
					*((u16*)op.dest_ptr) += op.dest_modify;
				}

				break;
			}
			case MICRO_OP_TYPE::WRITE_ADDR_8:
			{
				// handle the write to an address. either from op value or last fetch value
				u32 addr = 0x0;
				if (op.is_use_addr) // prio is last value
				{
					addr = addr;
				}
				else if (op.dest_ptr != nullptr) // then we look if src ptr
				{
					addr = *((u16*)op.dest_ptr);
				}

				addr += op.addr_offset; // add a addr offset. mostly used for 0xFF00 + n ops

				if (check_memory_breakpoint(current_pc, addr))
				{
					return 0;
				}

				// write 8 bits to 16 bit addr
				u8 value = 0x0;
				if (op.is_use_value)
				{
					value = op.value;
				}
				else if (op.value_ptr != nullptr)
				{
					value = (u8)*op.value_ptr;
				}

				// write to memory
				memory_module::write_memory(addr, &value, 1);

				// apply modifier for LDI  and LDD
				if (op.src_ptr != nullptr)
				{
					*((u16*)op.src_ptr) += op.src_modify;
				}

				// apply modifier for LDI  and LDD
				if (op.dest_ptr != nullptr)
				{
					*((u16*)op.dest_ptr) += op.dest_modify;
				}

				break;
			}
			case MICRO_OP_TYPE::JUMP:
			{
				// handle PC jump offset. either from op.value or last fetch value
				s32 value = 0x0;
				if (op.is_use_value)
				{
					value = op.value;
				}
				else if (op.value_ptr != nullptr)
				{
					if (op.is_signed)
					{
						value = (s8)(*op.value_ptr);
					}
					else
					{
						value = *op.value_ptr;
					}
				}

				R.pc += value;

				break;
			}
			case MICRO_OP_TYPE::ROTATE_ACCUMULATOR:
			{
				clear_flag(FLAG_SUBTRACTION);
				clear_flag(FLAG_HALFCARRY);
				clear_flag(FLAG_ZERO);

				switch (op.rotate_type)
				{
				case ROTATE_TYPE::LEFT_CIRCULAR:
				{
					u8 carry = R.a >> 7;
					R.a = (R.a << 1) | carry;
					if (carry)
					{
						set_flag(FLAG_CARRY);
					}
					else
					{
						clear_flag(FLAG_CARRY);
					}

					break;
				}
				case ROTATE_TYPE::LEFT_THROUGH_CARRY:
				{
					u8 carry = R.a >> 7;
					R.a = (R.a << 1) | get_flag(FLAG_CARRY);
					if (carry)
					{
						set_flag(FLAG_CARRY);
					}
					else
					{
						clear_flag(FLAG_CARRY);
					}

					break;
				}
				case ROTATE_TYPE::RIGHT_CIRCULAR:
				{
					u8 carry = R.a & 0x1;
					R.a = (R.a >> 1) | (carry << 7);
					if (carry)
					{
						set_flag(FLAG_CARRY);
					}
					else
					{
						clear_flag(FLAG_CARRY);
					}

					break;
				}
				case ROTATE_TYPE::RIGHT_THROUGH_CARRY:
				{
					u8 carry = R.a & 0x1;
					R.a = (R.a >> 1) | (get_flag(FLAG_CARRY) << 7);
					if (carry)
					{
						set_flag(FLAG_CARRY);
					}
					else
					{
						clear_flag(FLAG_CARRY);
					}

					break;
				}
				}

				break;
			}
			case MICRO_OP_TYPE::DAA:
			{
				// DAA
				u16 a = R.a;

				if (get_flag(FLAG_SUBTRACTION) != 0)
				{
					if (get_flag(FLAG_HALFCARRY) != 0)
					{
						a = (a - 0x06) & 0xFF;
					}

					if (get_flag(FLAG_CARRY) != 0)
					{
						a -= 0x60;
					}
				}
				else
				{
					if (get_flag(FLAG_HALFCARRY) != 0 || (a & 0xF) > 9)
					{
						a += 0x06;
					}

					if (get_flag(FLAG_CARRY) != 0 || a > 0x9F)
					{
						a += 0x60;
					}
				}

				R.a = (u8)(a & 0xFF);
				clear_flag(FLAG_HALFCARRY);

				if (R.a)
				{
					clear_flag(FLAG_ZERO);
				}
				else
				{
					set_flag(FLAG_ZERO);
				}

				if (a >= 0x100)
				{
					set_flag(FLAG_CARRY);
				}

				break;
			}
			case MICRO_OP_TYPE::CPL:
			{
				// CPL
				R.a = ~R.a;
				set_flag(FLAG_HALFCARRY);
				set_flag(FLAG_SUBTRACTION);

				break;
			}
			case MICRO_OP_TYPE::SCF:
			{
				// SCF
				set_flag(FLAG_CARRY);
				clear_flag(FLAG_HALFCARRY);
				clear_flag(FLAG_SUBTRACTION);

				break;
			}
			case MICRO_OP_TYPE::CCF:
			{	
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
			case MICRO_OP_TYPE::HALT:
			{
				if (interrupt_master) // interrupt servicing enabled
				{
					halt = true;
					R.pc--;
				}
				else // interrupt servicing disabled
				{
					if ((*interrupt_enable_flag & *interrupt_request_flag & 0x1F) != 0x0) // halt bug if pending interrupts
					{
						halt_bug = true;
					}
					else // no pending. we halt but don't service interrupt
					{
						halt = true;
						R.pc--;
					}
				}

				break;
			}
			case MICRO_OP_TYPE::ALU:
			{
				if (op.src_ptr == nullptr)
				{
					assert(false);
				}

				u8 temp = *op.src_ptr;
				alu_function[op.alu_rot_index](&temp);

				// dest_ptr not used as alu funcs set R register directly
				break;
			}
			case MICRO_OP_TYPE::ROTATE_SHIFT:
			{
				if (op.dest_ptr == nullptr)
				{
					assert(false);
				}

				if (op.src_ptr == nullptr)
				{
					assert(false);
				}

				u8 temp = *op.src_ptr;
				rot_function[op.alu_rot_index](&temp);
				*op.dest_ptr = temp; // set back to destination

				break;
			}
			case MICRO_OP_TYPE::CONDITION:
			{
				if (!condition_funct[op.condition_index]())
				{
					// condition not met. pop number of micro ops
					for (int i = 0; i < op.condition_fail_pop_count; i++)
					{
						if (micro_op_queue.empty())
						{
							break;
						}
						
						micro_op_queue.pop_front();
					}
				}

				break;
			}
			case MICRO_OP_TYPE::IME:
			{
				if (op.is_use_value)
				{
					switch (op.value)
					{
					case IME_MODE::DISABLE:
						interrupt_master = false;
						ei_occcurred = 0;
						break;
					case IME_MODE::ENABLE:
						interrupt_master = true;
						ei_occcurred = 0;
						break;
					case IME_MODE::ENABLE_DELAYED:
						ei_occcurred = 2;
						break;
					}
				}

				break;
			}
			case MICRO_OP_TYPE::TEST_BIT:
			{
				u8 value = 0;
				if (op.src_ptr != nullptr)
				{
					value = *op.src_ptr;
				}

				u8 bit = (u8)op.value;

				if (value & (1 << bit))
				{
					clear_flag(FLAG_ZERO);
				}
				else
				{
					set_flag(FLAG_ZERO);
				}

				set_flag(FLAG_HALFCARRY);
				clear_flag(FLAG_SUBTRACTION);

				break;
			}
			case MICRO_OP_TYPE::RESET_BIT:
			{
				if (op.src_ptr != nullptr)
				{
					*op.src_ptr &= ~(1 << (u8)op.value);
				}

				break;
			}
			case MICRO_OP_TYPE::SET_BIT:
			{
				if (op.src_ptr != nullptr)
				{
					*op.src_ptr |= (1 << (u8)op.value);
				}

				break;
			}
			}

			return 0;
		}

		int update()
		{
			// if micro ops is empty. we decode next op code
			if (micro_op_queue.empty())
			{
				// clear temp values
				last_opcode = 0x0;
				last_temp_value = 0x0;

				// when pulling new op code. if ei occured on delay we enable here
				if (ei_occcurred > 0)
				{
					ei_occcurred--;

					if (ei_occcurred == 0)
					{
						ei_occcurred = false;
						interrupt_master = true;
					}
				}

				// fetch the opcode
				MicroOp fetch_op;
				fetch_op.micro_op_type = MICRO_OP_TYPE::FETCH_OP;

				micro_op_queue.push_back(fetch_op);

				// handle breakpoitns for the debugger
				if (!breakpoint_disable_one_instr)
				{
					if (breakpoints.size() > 0)
					{
						auto breakpoint_itr = std::find(breakpoints.begin(), breakpoints.end(), R.pc);
						if (breakpoint_itr != breakpoints.end())
						{
							paused = true;
							breakpoint_hit = true;
							return 0;
						}
					}

					// soft breakpoints are used for step over. not visible
					if (soft_breakpoints.size() > 0)
					{
						auto breakpoint_itr = std::find(soft_breakpoints.begin(), soft_breakpoints.end(), R.pc);
						if (breakpoint_itr != soft_breakpoints.end())
						{
							if (memory_breakpoint_last_addr == -1) // hacky to get mem breakpoints working
							{
								paused = true;
								breakpoint_hit = true;
							}

							soft_breakpoints.erase(breakpoint_itr);
							return 0;
						}
					}
				}

				if (breakpoint_disable_one_instr)
				{
					breakpoint_hit = true;
					breakpoint_disable_one_instr = false;
				}
			}

			if (paused && !breakpoint_disable_one_instr)
			{
				return 0;
			}

			// now we can process a micro op at a time
			MicroOp op = micro_op_queue.front();
			micro_op_queue.pop_front();

			execute_micro_op(op);

			is_opcode_complete = micro_op_queue.empty(); // if its empty then we completed opcode

			return 4;
		}
	}
}