#pragma once

#include "defines.h"

#include "cpu_memory_module.h"

#define DEBUG_ASSERT_INSTR_TIMINGS

namespace nes
{
	namespace cpu
	{
		enum MICRO_OP_TYPE
		{
			NOP,
			FETCH_OP,
			READ_BUS_EXECUTE_ALU,
			READ_MODIFY_WRITE,
			EXECUTE_IMPLIED
		};

		struct MicroOp
		{			
			MICRO_OP_TYPE micro_op_type;
			void(*alu_funct)(u8);
			u8(*mod_funct)(u8);
			void(*implied_funct)();
		};

		struct Registers
		{
			u8 a;
			u8 x;
			u8 y;
			u16 pc;
			u8 sp;
			u8 p;
		} R;

		bool running = true;
		u16 pc = 0x0;
		bool is_opcode_complete;

		std::deque<MicroOp> micro_op_queue;

		// flag macros and functions
		enum FLAGS
		{
			FLAG_CARRY = 0,
			FLAG_ZERO,
			FLAG_INTERRUPT,
			FLAG_DECIMAL,
			FLAG_BREAK,
			FLAG_UNUSED,
			FLAG_OVERFLOW,
			FLAG_NEGATIVE
		};

		// set and get flag helpers
		inline void set_flag(u8 flag)
		{
			R.p = (1 << flag);
		}

		inline void clear_flag(u8 flag)
		{
			flag = (1 << flag);
			R.p = ~flag;
		}

		inline u8 get_flag(u8 flag)
		{
			return (R.p >> flag) & 0x1;
		}

		inline void clear_all_flags()
		{
			R.p = 0x0;
		}

		// ALU function pointers
		inline void alu_ora(u8 data)
		{
			// bitwise or
		}

		inline void alu_and(u8 data)
		{
			// bitwise and
		}

		inline void alu_eor(u8 data)
		{
			// bitwise xor
		}

		inline void alu_adc(u8 data)
		{
			// add with carry
		}

		inline void alu_lda(u8 data)
		{
			// ld accumulator
		}

		inline void alu_cmp(u8 data)
		{
			// compare
		}

		inline void alu_sbc(u8 data)
		{
			// subtract with carry
			alu_adc(data); // invert bits of data and pass to add
		}

		inline void alu_bit(u8 data)
		{
			// bit test
		}

		inline void alu_ldy(u8 data)
		{
			// load y reg
		}

		inline void alu_cpy(u8 data)
		{
			// compare y reg
		}

		inline void alu_cpx(u8 data)
		{
			// compare x reg
		}

		void(*alu_function_group_0[])(u8) = { nullptr, nullptr, nullptr, nullptr, alu_bit, alu_ldy, alu_cpy, alu_cpx };
		void(*alu_function_group_1[])(u8) = { alu_ora, alu_and, alu_eor, alu_adc, nullptr, alu_lda, alu_cmp, alu_sbc };

		// modify function pointers
		inline u8 mod_asl(u8 data)
		{
			// shift left
			return 0;
		}

		inline u8 mod_rol(u8 data)
		{
			// rotate left
			return 0;
		}

		inline u8 mod_lsr(u8 data)
		{
			// logical shift right
			return 0;
		}

		inline u8 mod_ror(u8 data)
		{
			// rotate right
			return 0;
		}

		inline u8 mod_dec(u8 data)
		{
			// dec mem
			return 0;
		}

		inline u8 mod_inc(u8 data)
		{
			// inc mem
			return 0;
		}

		u8(*mod_functions_group_2[])(u8) = { mod_asl, mod_rol, mod_lsr, mod_ror, nullptr, nullptr, mod_dec, mod_inc };

		// addr mode function pointers
		inline void addr_immediate()
		{
			// immediate addr mode. load temp addr from pc
		}

		inline void addr_zeropage()
		{
			// zero page. micro op to read zero page addr
		}

		inline void addr_zeropage_x()
		{
			// zero page fetch. add x and wrap to 0xFF
		}

		inline void addr_zeropage_y()
		{
			// zero page fetch. add y and wrap to 0xFF
		}

		inline void addr_absolute()
		{
			// absolute addr. two micro ops to read low and high to temp values
		}

		inline void addr_absolute_x()
		{
			// feth abosulte add low and high to temp, add x register to temp conditional check page cross
		}

		inline void addr_absolute_y()
		{
			// feth abosulte add low and high to temp, add y register to temp conditional check page cross
		}

		inline void addr_indirect_x()
		{
			// read base pointer. offset by x and warp 0xFF. read vector high and low
		}

		inline void addr_indirect_y()
		{
			// read base pointer. offset by y and wrap 0xFF. read vector high and low
		}

		void(*addr_mode_function_group_0[])() = { addr_immediate, addr_zeropage, addr_immediate, addr_absolute, addr_indirect_x, addr_zeropage_x, addr_indirect_y, addr_absolute_x };
		void(*addr_mode_function_group_1[])() = { addr_immediate, addr_zeropage, nullptr, addr_absolute, nullptr, addr_zeropage_y, nullptr, addr_absolute_y };
		void(*addr_mode_function_group_2[])() = { addr_immediate, addr_zeropage, nullptr, addr_absolute, nullptr, addr_zeropage_x, nullptr, addr_absolute_x };
		
		// implied execute functions
		inline void implied_clc()
		{
			// clear carry
			clear_flag(FLAG_CARRY);
		}

		inline void implied_sec()
		{
			// set carry
			set_flag(FLAG_CARRY);
		}

		inline void implied_cli()
		{
			// clear interrupt disable
			clear_flag(FLAG_INTERRUPT);
		}

		inline void implied_sei()
		{
			// set interrupt disable
			set_flag(FLAG_INTERRUPT);
		}

		inline void implied_clv()
		{
			// clear overflow
			clear_flag(FLAG_OVERFLOW);
		}

		inline void implied_cld()
		{
			// clear deimal
			clear_flag(FLAG_DECIMAL);
		}

		inline void implied_sed()
		{
			// set decimal
			set_flag(FLAG_DECIMAL);
		}

		// read 8 and 16 bit at PC. increment PC
		inline u8 readpc_u8()
		{
			u8 val = cpu_memory_module::read_memory(pc++);

			return val;
		}

		inline u16 readpc_u16()
		{
			// lsb is first in memory
			u16 val = cpu_memory_module::read_memory(pc++);
			val |= (cpu_memory_module::read_memory(pc++) << 8);

			return val;
		}

		inline u16 get_current_pc()
		{
			return pc;
		}

		int reset()
		{
			running = true;
			pc = 0x0;
			is_opcode_complete = false;

			micro_op_queue.clear();

			// initial values
			R.a = 0x0;
			R.x = 0x0;
			R.y = 0x0;
			R.pc = 0x0;
			R.sp = 0x0;
			R.p = 0x0;

			return 0;
		}

		int initialize()
		{			
			reset();

			// need to first read the reset vector
			u8 reset_low = cpu_memory_module::read_memory(0xFFFC);
			u16 reset_high = cpu_memory_module::read_memory(0xFFFD);

			pc = cpu_memory_module::read_memory(0xFFFC);
			pc |= cpu_memory_module::read_memory(0xFFFD) << 8;

			return 0;
		}

		int decode_implied_instruction(u8 opcode)
		{
			void(*implied_funct)() = nullptr;

			switch (opcode)
			{
			case 0x18:
			{
				// clear carry
				implied_funct = implied_clc;
				break;
			}
			case 0x38:
			{
				// set carry
				implied_funct = implied_sec;
				break;
			}
			case 0x58:
			{
				// clear interrupt disable
				implied_funct = implied_cli;
				break;
			}
			case 0x78:
			{
				// set interrupt
				implied_funct = implied_sei;
				break;
			}
			case 0xB8:
			{
				// clear overflow
				implied_funct = implied_clv;
				break;
			}
			case 0xD8:
			{
				// clear decimal
				implied_funct = implied_cld;
				break;
			}
			case 0xF8:
			{
				// set decimal
				implied_funct = implied_sed;
				break;
			}
			case 0x8A:
			{
				// transfer x to a
				assert(false);
				break;
			}
			case 0x9A:
			{
				// transfer x to stack pointer
				assert(false);
				break;
			}
			case 0xAA:
			{
				// transfer a to x
				assert(false);
				break;
			}
			case 0xBA:
			{
				// transfer stack pointer to x
				assert(false);
				break;
			}
			case 0xCA:
			{
				// dec x
				assert(false);
				break;
			}
			case 0xEA:
			{
				// no op
				assert(false);
				break;
			}
			case 0x98:
			{
				// transfer y to a
				assert(false);
				break;
			}
			case 0xA8:
			{
				// transfer a to y
				assert(false);
				break;
			}
			case 0xC8:
			{
				// inc y
				assert(false);
				break;
			}
			case 0xE8:
			{
				// inc y
				assert(false);
				break;
			}
			default:
			{
				// not implemented
				assert(false);
				break;
			}
			}

			MicroOp execute_implied;
			execute_implied.micro_op_type = EXECUTE_IMPLIED;
			execute_implied.implied_funct = implied_funct;

			micro_op_queue.push_back(execute_implied);

			return 0;
		}

		int decode_opcode(u8 opcode)
		{
			// check if opcode is condition branch
			if ((opcode & 0x1F) == 0x10)
			{
				assert(false);
				return 0;
			}

			// check all single byte implied instructions
			if ((opcode & 0x0F) == 0x08 || (opcode & 0x0F) == 0x0A)
			{
				return decode_implied_instruction(opcode);
			}

			// decode by spliting opcode into aaabbbcc format
			u8 aaa = (opcode >> 5) & 0x7;
			u8 bbb = (opcode >> 2) & 0x7;
			u8 cc = (opcode & 0x3);

			switch (cc)
			{
			case 0x0: // op group for control and misc alu ops
			{
				if (opcode == 0x00)
				{
					// BRK
					assert(false);
				}
				else if (opcode == 0x20)
				{
					// JSR
					assert(false);
				}
				else if (opcode == 0x40)
				{
					// RTI
					assert(false);
				}
				else if (opcode == 0x60)
				{
					// RTS
					assert(false);
				}
				else if (aaa == 0x2)
				{
					// JMP absolute
					assert(false);
				}
				else if (aaa == 0x3)
				{
					// JMP indirect
					assert(false);
				}
				else if (aaa == 0x4)
				{
					// STY
					assert(false);
				}
				else
				{
					// other alu ops (bit, ldy, cpy, cpx)
					assert(false);
					MicroOp read_bus_exec;
					read_bus_exec.micro_op_type = READ_BUS_EXECUTE_ALU;
					read_bus_exec.alu_funct = alu_function_group_0[aaa];

					micro_op_queue.push_back(read_bus_exec);
				}
				break;
			}
			case 0x1: // op group for alu, load and store
			{
				if (aaa == 0x4)
				{
					// STA
					assert(false);
				}
				else
				{
					// other ALU ops (ora, and, eor, adc, lda, cmp, sbc)
					assert(false);
					MicroOp read_bus_exec;
					read_bus_exec.micro_op_type = READ_BUS_EXECUTE_ALU;
					read_bus_exec.alu_funct = alu_function_group_1[aaa];

					micro_op_queue.push_back(read_bus_exec);
				}
				break;
			}
			case 0x2: // op group for bitwise ops and x registry
			{
				if (aaa == 0x4) 
				{
					// STX
					assert(false);
				}
				else if (aaa == 0x5) 
				{
					// LDX
					assert(false);
				}
				else
				{
					// other mod ops (asl, rol, lsr, ror, dec, inc)
					assert(false);
					MicroOp read_modify_write;
					read_modify_write.micro_op_type = READ_MODIFY_WRITE;
					read_modify_write.mod_funct = mod_functions_group_2[aaa];

					micro_op_queue.push_back(read_modify_write);
				}
				break;
			}
			case 0x3: // op group 3 - undocumnted illegal opcodes but some roms may use them
			{
				assert(false);
				break;
			}
			default:
			{
				assert(false);
				break;
			}
			}

			return 0;
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
				u8 opcode = readpc_u8();

				decode_opcode(opcode);

				// set this as we have started a new opcode
				is_opcode_complete = false;
				break;
			}
			case READ_BUS_EXECUTE_ALU:
			{
				break;
			}
			case READ_MODIFY_WRITE:
			{
				break;
			}
			case EXECUTE_IMPLIED:
			{
				// we can execute the function pointer from the micro op
				op.implied_funct();
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
				// fetch the opcode
				MicroOp fetch_op;
				fetch_op.micro_op_type = MICRO_OP_TYPE::FETCH_OP;

				micro_op_queue.push_back(fetch_op);
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