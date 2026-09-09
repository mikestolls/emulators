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
			READ_MODIFY_WRITE
		};

		struct MicroOp
		{			
			MICRO_OP_TYPE micro_op_type;
			void(*alu_funct)(u8);
			u8(*mod_funct)(u8);
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

		// temp placing here but move up later
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

		int decode_opcode(u8 opcode)
		{
			// decode by spliting opcode into aaabbbcc format
			u8 aaa = (opcode >> 5) & 0x7;
			u8 bbb = (opcode >> 2) & 0x7;
			u8 cc = (opcode & 0x3);

			switch (cc)
			{
			case 0x0: // op group for control and misc alu ops
			{
				if (aaa == 0x4)
				{
					// STY
				}
				else if (aaa == 0x2)
				{
					// JMP absolute
				}
				else if (aaa == 0x3)
				{
					// JMP indirect
				}
				else
				{
					// other alu ops (bit, ldy, cpy, cpx)
					MicroOp read_bus_exec;
					read_bus_exec.micro_op_type = READ_BUS_EXECUTE_ALU;
					read_bus_exec.alu_funct = alu_function_group_0[aaa];
				}
				break;
			}
			case 0x1: // op group for alu, load and store
			{
				if (aaa == 0x4)
				{
					// STA
				}
				else
				{
					// other ALU ops (ora, and, eor, adc, lda, cmp, sbc)
					MicroOp read_bus_exec;
					read_bus_exec.micro_op_type = READ_BUS_EXECUTE_ALU;
					read_bus_exec.alu_funct = alu_function_group_1[aaa];
				}
				break;
			}
			case 0x2: // op group for bitwise ops and x registry
			{
				if (aaa == 0x4) 
				{
					// STX
				}
				else if (aaa == 0x5) 
				{
					// LDX
				}
				else
				{
					// other mod ops (asl, rol, lsr, ror, dec, inc)
					MicroOp read_modify_write;
					read_modify_write.micro_op_type = READ_MODIFY_WRITE;
					read_modify_write.mod_funct = mod_functions_group_2[aaa];
				}
				break;
			}
			case 0x3: // op group 3 - undocumnted illegal opcodes but some roms may use them
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