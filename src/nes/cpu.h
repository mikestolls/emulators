#pragma once

#include "defines.h"

//#include "memory_module.h"

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
		u16 current_pc = 0x0;
		bool is_opcode_complete;

		std::deque<MicroOp> micro_op_queue;
		
		int reset()
		{
			running = true;
			is_opcode_complete = false;

			micro_op_queue.clear();

			return 0;
		}

		int initialize()
		{			
			reset();

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