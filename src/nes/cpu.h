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
			FETCH_OP
		};

		struct MicroOp
		{			
			MICRO_OP_TYPE micro_op_type;
		};

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