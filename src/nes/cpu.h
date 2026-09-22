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
			FETCH_IMMEDIATE,
			FETCH_IMMEDIATE_EXECUTE_ALU,
			READ_ADDR_BUS_EXECUTE_ALU,
			READ_INDIRECT_ADDR,
			READ_ADDR_BUS,
			READ_VALUE_FROM_STACK,
			WRITE_VALUE_TO_STACK,
			WRITE_DATA_BUS_TO_ADDR_BUS,
			MODIFY_WRITE_DATA_BUS_TO_ADDR_BUS,
			EXECUTE_IMPLIED_FUNCTION,
			DATA_BUS_EXECUTE_ALU,
			TRANSFER_VALUES,
			CONDITIONAL_BRANCH,
			ADD_INDEX_TO_ADDR_BUS,
		};

		struct MicroOp
		{			
			MICRO_OP_TYPE micro_op_type = NOP;
			u8 opcode = 0x0;;
			void(*alu_funct)(u8) = nullptr;
			u8(*mod_funct)(u8) = nullptr;
			void(*implied_funct)() = nullptr;
			bool is_high_byte = false;
			u8* transfer_src = nullptr;
			u8* transfer_dest = nullptr;
			bool is_update_flags = false;
			u8 conditional_value = 0x0;
			u8 conditional_flag = 0x0;
			bool is_wrap_low = false;
			bool is_transfer_addr_to_pc = false;
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

		// NTSC NES Clock & Frame Timing
		const u32 master_clock_hz = 21477272; // NTSC Master Clock frequency (~21.48 MHz)
		const u32 cpu_cycles_per_sec = 1789773;  // Master Clock / 12 (~1.79 MHz)
		const u32 ppu_cycles_per_sec = 5369318;  // Master Clock / 4  (~5.37 MHz)

		const u32 cycles_per_line = 341;      // PPU cycles per scanline (113.66 CPU cycles)
		const u32 lines_per_frame = 262;      // Total scanlines (240 visible + 1 dummy + 21 VBlank)
		const u32 ppu_cycles_per_frame = cycles_per_line * lines_per_frame; // = 89342 PPU dots
		const u32 cpu_cycles_per_frame = ppu_cycles_per_frame / 3;          // ~29780.67 CPU cycles

		bool running = true;
		u8 current_opcode = 0x0;
		bool is_opcode_complete;

		std::deque<MicroOp> micro_op_queue;
		u16 micro_op_addr_bus;
		u8 micro_op_data_bus;
		u8 micro_op_zeropage;

		// read 8 and 16 bit at PC. increment PC
		inline u8 readpc_u8()
		{
			u8 val = cpu_memory_module::read_memory(R.pc++);

			return val;
		}

		inline u16 get_current_pc()
		{
			return R.pc;
		}

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

		inline void update_flags_nz(u8 value)
		{
			// set the zero flag
			if (value == 0x0)
			{
				set_flag(FLAG_ZERO);
			}
			else
			{
				clear_flag(FLAG_ZERO);
			}

			// set the negative flag
			if (value & 0x80)
			{
				set_flag(FLAG_NEGATIVE);
			}
			else
			{
				clear_flag(FLAG_NEGATIVE);
			}
		}

		// ALU function pointers
		inline void alu_ora(u8 value)
		{
			// bitwise or
			R.a |= value;

			update_flags_nz(R.a);
		}

		inline void alu_and(u8 value)
		{
			// bitwise and
			R.a &= value;

			update_flags_nz(R.a);
		}

		inline void alu_eor(u8 value)
		{
			// bitwise xor
			R.a ^= value;

			update_flags_nz(R.a);
		}

		inline void alu_adc(u8 value)
		{
			// add with carry
			assert(false);
		}

		inline void alu_lda(u8 value)
		{
			// ld accumulator
			R.a = value;

			update_flags_nz(R.a);
		}

		inline void alu_cmp(u8 value)
		{
			// compare
			u16 result = (u16)R.a - (u16)value;

			if (R.a >= value)
			{
				set_flag(FLAG_CARRY);
			}
			else 
			{
				clear_flag(FLAG_CARRY);
			}

			update_flags_nz((u8)result);
		}

		inline void alu_sbc(u8 value)
		{
			// subtract with carry
			alu_adc(value); // invert bits of data and pass to add
		}

		inline void alu_bit(u8 value)
		{
			// bit test
			assert(false);
		}

		inline void alu_ldy(u8 value)
		{
			// load y reg
			R.y = value;

			update_flags_nz(R.x);
		}

		inline void alu_ldx(u8 value)
		{
			// load y reg
			R.x = value;

			update_flags_nz(R.x);
		}

		inline void alu_cpy(u8 value)
		{
			// compare y reg
			u16 result = (u16)R.y - (u16)value;

			if (R.y >= value)
			{
				set_flag(FLAG_CARRY);
			}
			else
			{
				clear_flag(FLAG_CARRY);
			}

			update_flags_nz((u8)result);
		}

		inline void alu_cpx(u8 value)
		{
			// compare x reg
			u16 result = (u16)R.x - (u16)value;

			if (R.x >= value)
			{
				set_flag(FLAG_CARRY);
			}
			else
			{
				clear_flag(FLAG_CARRY);
			}

			update_flags_nz((u8)result);
		}

		void(*alu_function_group_0[])(u8) = { nullptr, nullptr, nullptr, nullptr, alu_bit, alu_ldy, alu_cpy, alu_cpx };
		void(*alu_function_group_1[])(u8) = { alu_ora, alu_and, alu_eor, alu_adc, nullptr, alu_lda, alu_cmp, alu_sbc };

		// modify function pointers
		inline u8 mod_asl(u8 data)
		{
			// shift left
			assert(false);
			return 0;
		}

		inline u8 mod_rol(u8 data)
		{
			// rotate left
			assert(false);
			return 0;
		}

		inline u8 mod_lsr(u8 data)
		{
			// logical shift right
			assert(false);
			return 0;
		}

		inline u8 mod_ror(u8 data)
		{
			// rotate right
			assert(false);
			return 0;
		}

		inline u8 mod_dec(u8 data)
		{
			// dec mem
			data--;

			update_flags_nz(data);
			return data;
		}

		inline u8 mod_inc(u8 data)
		{
			// inc mem
			data++;

			update_flags_nz(data);
			return 0;
		}

		u8(*mod_functions_group_2[])(u8) = { mod_asl, mod_rol, mod_lsr, mod_ror, nullptr, nullptr, mod_dec, mod_inc };

		// addr mode function pointers
		inline void addr_immediate()
		{
			// immediate addr mode. load temp addr from pc
			micro_op_data_bus = readpc_u8();
		}

		inline void addr_zeropage()
		{
			// zero page. micro op to read zero page addr
			micro_op_addr_bus = 0x0;

			MicroOp fetch_low;
			fetch_low.opcode = current_opcode;
			fetch_low.micro_op_type = FETCH_IMMEDIATE;
			fetch_low.transfer_dest = (u8*)&micro_op_addr_bus;
			fetch_low.is_high_byte = false;
			micro_op_queue.push_back(fetch_low);
		}

		inline void addr_zeropage_x()
		{
			// zero page fetch. add x and wrap to 0xFF
			assert(false);
		}

		inline void addr_zeropage_y()
		{
			// zero page fetch. add y and wrap to 0xFF
			assert(false);
		}

		inline void addr_absolute()
		{
			// absolute addr. two micro ops to read low and high to temp values
			MicroOp fetch_low;
			fetch_low.opcode = current_opcode;
			fetch_low.micro_op_type = FETCH_IMMEDIATE;
			fetch_low.transfer_dest = (u8*)&micro_op_addr_bus;
			fetch_low.is_high_byte = false;
			micro_op_queue.push_back(fetch_low);

			MicroOp fetch_high;
			fetch_high.opcode = current_opcode;
			fetch_high.micro_op_type = FETCH_IMMEDIATE;
			fetch_high.transfer_dest = (u8*)&micro_op_addr_bus;
			fetch_high.is_high_byte = true;
			micro_op_queue.push_back(fetch_high);
		}

		inline void addr_absolute_x()
		{
			// feth abosulte add low and high to temp, add x register to temp conditional check page cross
			assert(false);
		}

		inline void addr_absolute_y()
		{
			// feth abosulte add low and high to temp, add y register to temp conditional check page cross
			assert(false);
		}

		inline void addr_indirect_x()
		{
			// read base pointer. offset by x and warp 0xFF. read vector high and low
			
			// zero page read
			micro_op_addr_bus = 0x0;

			MicroOp fetch_low;
			fetch_low.opcode = current_opcode;
			fetch_low.micro_op_type = FETCH_IMMEDIATE;
			fetch_low.transfer_dest = (u8*)&micro_op_zeropage;
			fetch_low.is_high_byte = false;
			micro_op_queue.push_back(fetch_low);

			// add x to addr and wrap 0xFF
			MicroOp add_index;
			add_index.opcode = current_opcode;
			add_index.micro_op_type = ADD_INDEX_TO_ADDR_BUS;
			add_index.transfer_src = &R.x;
			add_index.transfer_dest = (u8*)&micro_op_zeropage;
			add_index.is_wrap_low = true;
			micro_op_queue.push_back(add_index);

			MicroOp read_low;
			read_low.opcode = current_opcode;
			read_low.micro_op_type = READ_INDIRECT_ADDR;
			read_low.transfer_dest = (u8*)&micro_op_addr_bus;
			read_low.is_high_byte = false;
			micro_op_queue.push_back(read_low);

			MicroOp read_high;
			read_high.opcode = current_opcode;
			read_high.micro_op_type = READ_INDIRECT_ADDR;
			read_high.transfer_dest = (u8*)&micro_op_addr_bus;
			read_high.is_high_byte = true;
			micro_op_queue.push_back(read_high);
		}

		inline void addr_indirect_y()
		{
			// read base pointer. offset by y and wrap 0xFF. read vector high and low

			// zero page. micro op to read zero page addr
			micro_op_addr_bus = 0x0;

			MicroOp fetch_low;
			fetch_low.opcode = current_opcode;
			fetch_low.micro_op_type = FETCH_IMMEDIATE;
			fetch_low.transfer_dest = (u8*)&micro_op_zeropage;
			fetch_low.is_high_byte = false;
			micro_op_queue.push_back(fetch_low);

			MicroOp read_low;
			read_low.opcode = current_opcode;
			read_low.micro_op_type = READ_INDIRECT_ADDR;
			read_low.transfer_dest = (u8*)&micro_op_addr_bus;
			read_low.is_high_byte = false;
			micro_op_queue.push_back(read_low);

			MicroOp read_high;
			read_high.opcode = current_opcode;
			read_high.micro_op_type = READ_INDIRECT_ADDR;
			read_high.transfer_dest = (u8*)&micro_op_addr_bus;
			read_high.is_high_byte = true;
			micro_op_queue.push_back(read_high);

			MicroOp add_index;
			add_index.opcode = current_opcode;
			add_index.micro_op_type = ADD_INDEX_TO_ADDR_BUS;
			add_index.transfer_src = &R.y;
			add_index.transfer_dest = (u8*)&micro_op_addr_bus;
			micro_op_queue.push_back(add_index);
		}

		void(*addr_mode_function_group_0[])() = { addr_immediate, addr_zeropage, nullptr, addr_absolute, nullptr, addr_zeropage_x, nullptr, addr_absolute_x };
		void(*addr_mode_function_group_1[])() = { addr_indirect_x, addr_zeropage, addr_immediate, addr_absolute, addr_indirect_y, addr_zeropage_x, addr_absolute_y, addr_absolute_x };
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

		inline void implied_dey()
		{
			// dec y and set flags
			R.y--;

			update_flags_nz(R.y);
		}

		inline void implied_iny()
		{
			// inc y and set flags
			R.y++;

			update_flags_nz(R.y);
		}

		inline void implied_dex()
		{
			// dec x and set flags
			R.x--;

			update_flags_nz(R.x);
		}

		inline void implied_inx()
		{
			// inc x and set flags
			R.x++;

			update_flags_nz(R.x);
		}

		int reset()
		{
			running = true;
			is_opcode_complete = false;

			micro_op_queue.clear();
			micro_op_addr_bus = 0x0;
			micro_op_data_bus = 0x0;
			micro_op_zeropage = 0x0;

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

			R.pc = cpu_memory_module::read_memory(0xFFFC);
			R.pc |= cpu_memory_module::read_memory(0xFFFD) << 8;

			return 0;
		}

		int decode_branch_instruction(u8 opcode)
		{
			// opcode can be split up. bit 5 is whether we are comparing to 0 or 1. bit 6, 7 are the flag we are checking
			MicroOp condition;
			condition.opcode = current_opcode;
			condition.micro_op_type = CONDITIONAL_BRANCH;
			condition.conditional_value = (opcode >> 5) & 0x1;

			switch ((opcode >> 6) & 0x3) // bit 6 and 7 is the flag to check
			{
			case 0x0:
			{
				// negative
				condition.conditional_flag = get_flag(FLAG_NEGATIVE);
				break;
			}
			case 0x1:
			{
				// overflow
				condition.conditional_flag = get_flag(FLAG_OVERFLOW);
				break;
			}
			case 0x2:
			{
				// carry
				condition.conditional_flag = get_flag(FLAG_CARRY);
				break;
			}
			case 0x3:
			{
				// zero
				condition.conditional_flag = get_flag(FLAG_ZERO);
				break;
			}
			}

			// push back condition check. it will fetch signed offset
			micro_op_queue.push_back(condition);

			return 0;
		}

		int decode_implied_instruction(u8 opcode)
		{
			switch (opcode)
			{
			case 0x18:
			{
				// clear carry
				MicroOp execute_implied;
				execute_implied.opcode = current_opcode;
				execute_implied.micro_op_type = EXECUTE_IMPLIED_FUNCTION;
				execute_implied.implied_funct = implied_clc;
				micro_op_queue.push_back(execute_implied);
				break;
			}
			case 0x38:
			{
				// set carry
				MicroOp execute_implied;
				execute_implied.opcode = current_opcode;
				execute_implied.micro_op_type = EXECUTE_IMPLIED_FUNCTION;
				execute_implied.implied_funct = implied_sec;
				micro_op_queue.push_back(execute_implied);
				break;
			}
			case 0x58:
			{
				// clear interrupt disable
				MicroOp execute_implied;
				execute_implied.opcode = current_opcode;
				execute_implied.micro_op_type = EXECUTE_IMPLIED_FUNCTION;
				execute_implied.implied_funct = implied_cli;
				micro_op_queue.push_back(execute_implied);
				break;
			}
			case 0x78:
			{
				// set interrupt
				MicroOp execute_implied;
				execute_implied.opcode = current_opcode;
				execute_implied.micro_op_type = EXECUTE_IMPLIED_FUNCTION;
				execute_implied.implied_funct = implied_sei;
				micro_op_queue.push_back(execute_implied);
				break;
			}
			case 0xB8:
			{
				// clear overflow
				MicroOp execute_implied;
				execute_implied.opcode = current_opcode;
				execute_implied.micro_op_type = EXECUTE_IMPLIED_FUNCTION;
				execute_implied.implied_funct = implied_clv;
				micro_op_queue.push_back(execute_implied);
				break;
			}
			case 0xD8:
			{
				// clear decimal
				MicroOp execute_implied;
				execute_implied.opcode = current_opcode;
				execute_implied.micro_op_type = EXECUTE_IMPLIED_FUNCTION;
				execute_implied.implied_funct = implied_cld;
				micro_op_queue.push_back(execute_implied);
				break;
			}
			case 0xF8:
			{
				// set decimal
				MicroOp execute_implied;
				execute_implied.opcode = current_opcode;
				execute_implied.micro_op_type = EXECUTE_IMPLIED_FUNCTION;
				execute_implied.implied_funct = implied_sed;
				micro_op_queue.push_back(execute_implied);
				break;
			}
			case 0x88:
			{
				// dec y
				MicroOp execute_implied;
				execute_implied.opcode = current_opcode;
				execute_implied.micro_op_type = EXECUTE_IMPLIED_FUNCTION;
				execute_implied.implied_funct = implied_dey;
				micro_op_queue.push_back(execute_implied);
				break;
			}
			case 0xC8:
			{
				// inc y
				MicroOp execute_implied;
				execute_implied.opcode = current_opcode;
				execute_implied.micro_op_type = EXECUTE_IMPLIED_FUNCTION;
				execute_implied.implied_funct = implied_iny;
				micro_op_queue.push_back(execute_implied);
				break;
			}
			case 0xCA:
			{
				// dec x
				MicroOp execute_implied;
				execute_implied.opcode = current_opcode;
				execute_implied.micro_op_type = EXECUTE_IMPLIED_FUNCTION;
				execute_implied.implied_funct = implied_dex;
				micro_op_queue.push_back(execute_implied);
				break;
			}
			case 0xE8:
			{
				// inc x
				MicroOp execute_implied;
				execute_implied.opcode = current_opcode;
				execute_implied.micro_op_type = EXECUTE_IMPLIED_FUNCTION;
				execute_implied.implied_funct = implied_inx;
				micro_op_queue.push_back(execute_implied);
				break;
			}
			case 0x8A:
			{
				// transfer x to a
				MicroOp transfer;
				transfer.opcode = current_opcode;
				transfer.micro_op_type = TRANSFER_VALUES;
				transfer.transfer_src = &R.x;
				transfer.transfer_dest = &R.a;
				transfer.is_update_flags = true;
				micro_op_queue.push_back(transfer);
				break;
			}
			case 0x98:
			{
				// transfer y to a
				MicroOp transfer;
				transfer.opcode = current_opcode;
				transfer.micro_op_type = TRANSFER_VALUES;
				transfer.transfer_src = &R.y;
				transfer.transfer_dest = &R.a;
				transfer.is_update_flags = true;
				micro_op_queue.push_back(transfer);
				break;
			}
			case 0x9A:
			{
				// transfer x to stack pointer
				MicroOp transfer;
				transfer.opcode = current_opcode;
				transfer.micro_op_type = TRANSFER_VALUES;
				transfer.transfer_src = &R.x;
				transfer.transfer_dest = &R.sp;
				transfer.is_update_flags = false;
				micro_op_queue.push_back(transfer);
				break;
			}
			case 0xA8:
			{
				// transfer a to y
				MicroOp transfer;
				transfer.opcode = current_opcode;
				transfer.micro_op_type = TRANSFER_VALUES;
				transfer.transfer_src = &R.a;
				transfer.transfer_dest = &R.y;
				transfer.is_update_flags = true;
				micro_op_queue.push_back(transfer);
				break;
			}
			case 0xAA:
			{
				// transfer a to x
				MicroOp transfer;
				transfer.opcode = current_opcode;
				transfer.micro_op_type = TRANSFER_VALUES;
				transfer.transfer_src = &R.a;
				transfer.transfer_dest = &R.x;
				transfer.is_update_flags = true;
				micro_op_queue.push_back(transfer);
				break;
			}
			case 0xBA:
			{
				// transfer stack pointer to x
				MicroOp transfer;
				transfer.opcode = current_opcode;
				transfer.micro_op_type = TRANSFER_VALUES;
				transfer.transfer_src = &R.sp;
				transfer.transfer_dest = &R.x;
				transfer.is_update_flags = true;
				micro_op_queue.push_back(transfer);
				break;
			}
			case 0xEA:
			{
				// no op
				assert(false);
				break;
			}
			case 0x48:
			{
				// push accumulator
				MicroOp delay;
				delay.opcode = current_opcode;
				delay.micro_op_type = NOP;
				micro_op_queue.push_back(delay); // dont need to dummy read. just cycle delay

				MicroOp stack;
				stack.opcode = opcode;
				stack.micro_op_type = WRITE_VALUE_TO_STACK;
				stack.transfer_src = &R.a;
				micro_op_queue.push_back(stack);

				break;
			}
			case 0x68:
			{		
				// pull accumulator
				MicroOp delay;
				delay.opcode = current_opcode;
				delay.micro_op_type = NOP;
				micro_op_queue.push_back(delay); // dont need to dummy read. just cycle delay

				// this read just increases stack pointer
				MicroOp stack_inc;
				stack_inc.opcode = opcode;
				stack_inc.micro_op_type = READ_VALUE_FROM_STACK;
				stack_inc.transfer_dest = &micro_op_data_bus;
				micro_op_queue.push_back(stack_inc);

				MicroOp stack;
				stack.opcode = opcode;
				stack.micro_op_type = READ_VALUE_FROM_STACK;
				stack.transfer_dest = &R.a;
				stack.is_update_flags = true;
				micro_op_queue.push_back(stack);

				break;
			}
			case 0x08:
			{
				// push processor status
				assert(false);
				break;
			}
			case 0x28:
			{
				// pull processor status
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

			return 0;
		}

		int decode_opcode(u8 opcode)
		{
			//printf("opcode: 0x%02hX R.y: 0x%02hX\n", opcode, R.y);

			// check if opcode is condition branch
			if ((opcode & 0x1F) == 0x10)
			{
				return decode_branch_instruction(opcode);
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
					micro_op_addr_bus = 0x0;

					// fetch low byte
					MicroOp fetch_low;
					fetch_low.opcode = current_opcode;
					fetch_low.micro_op_type = FETCH_IMMEDIATE;
					fetch_low.transfer_dest = (u8*)&micro_op_addr_bus;
					fetch_low.is_high_byte = false;
					micro_op_queue.push_back(fetch_low);

					// internal delay
					MicroOp internal_delay;
					internal_delay.opcode = current_opcode;
					internal_delay.micro_op_type = NOP;
					micro_op_queue.push_back(internal_delay);

					// write PC high byte to stack
					MicroOp stack_high;
					stack_high.opcode = opcode;
					stack_high.micro_op_type = WRITE_VALUE_TO_STACK;
					stack_high.transfer_src = ((u8*)&R.pc) + 1;
					micro_op_queue.push_back(stack_high);

					// write PC low byte to stack
					MicroOp stack_low;
					stack_low.opcode = opcode;
					stack_low.micro_op_type = WRITE_VALUE_TO_STACK;
					stack_low.transfer_src = (u8*)&R.pc;
					micro_op_queue.push_back(stack_low);

					// fetch high byte and transfer to PC.
					MicroOp fetch_high;
					fetch_high.opcode = current_opcode;
					fetch_high.micro_op_type = FETCH_IMMEDIATE;
					fetch_high.transfer_dest = (u8*)&micro_op_addr_bus;
					fetch_high.is_high_byte = true;
					fetch_high.is_transfer_addr_to_pc = true;
					micro_op_queue.push_back(fetch_high);
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
					micro_op_addr_bus = 0x0;

					// fetch low byte
					MicroOp fetch_low;
					fetch_low.opcode = current_opcode;
					fetch_low.micro_op_type = FETCH_IMMEDIATE;
					fetch_low.transfer_dest = (u8*)&micro_op_addr_bus;
					fetch_low.is_high_byte = false;
					micro_op_queue.push_back(fetch_low);

					// fetch high byte and transfer to PC.
					MicroOp fetch_high;
					fetch_high.opcode = current_opcode;
					fetch_high.micro_op_type = FETCH_IMMEDIATE;
					fetch_high.transfer_dest = (u8*)&micro_op_addr_bus;
					fetch_high.is_high_byte = true;
					fetch_high.is_transfer_addr_to_pc = true;
					micro_op_queue.push_back(fetch_high);
				}
				else if (aaa == 0x3)
				{
					// JMP indirect
					assert(false);
				}
				else if (aaa == 0x4)
				{
					// STY
					addr_mode_function_group_0[bbb]();

					micro_op_data_bus = R.y; // will put reg y on data bus to reuse micro op

					MicroOp write_bus;
					write_bus.opcode = current_opcode;
					write_bus.micro_op_type = WRITE_DATA_BUS_TO_ADDR_BUS;
					micro_op_queue.push_back(write_bus);
				}
				else
				{
					// other alu ops (bit, ldy, cpy, cpx)
					addr_mode_function_group_0[bbb]();

					MicroOp read_bus_exec;
					read_bus_exec.opcode = current_opcode;
					read_bus_exec.micro_op_type = READ_ADDR_BUS_EXECUTE_ALU;
					read_bus_exec.alu_funct = alu_function_group_0[aaa];

					if (bbb == 0x0) // immeidate addr mode
					{
						read_bus_exec.micro_op_type = DATA_BUS_EXECUTE_ALU;
					}

					micro_op_queue.push_back(read_bus_exec);
				}
				break;
			}
			case 0x1: // op group for alu, load and store
			{
				addr_mode_function_group_1[bbb]();

				if (aaa == 0x4)
				{
					// STA					
					micro_op_data_bus = R.a; // put R.a to bus

					// then a micro op to write bus to temp_add. 
					MicroOp write_bus;
					write_bus.opcode = current_opcode;
					write_bus.micro_op_type = WRITE_DATA_BUS_TO_ADDR_BUS;
					micro_op_queue.push_back(write_bus);
				}
				else
				{
					// other ALU ops (ora, and, eor, adc, lda, cmp, sbc)
					MicroOp read_bus_exec;
					read_bus_exec.opcode = current_opcode;
					read_bus_exec.micro_op_type = READ_ADDR_BUS_EXECUTE_ALU;
					read_bus_exec.alu_funct = alu_function_group_1[aaa];

					if (bbb == 0x2) // immeidate addr mode
					{
						read_bus_exec.micro_op_type = DATA_BUS_EXECUTE_ALU;
					}

					micro_op_queue.push_back(read_bus_exec);
				}
				break;
			}
			case 0x2: // op group for bitwise ops and x registry
			{
				if (aaa == 0x4) 
				{
					// STX
					// for bbb = 5 or 7 the mapping is different.
					assert(false);
				}
				else if (aaa == 0x5) 
				{
					// LDX
					// for bbb = 5 or 7 the mapping is different. 
					MicroOp ldx;
					ldx.opcode = current_opcode;
					ldx.micro_op_type = FETCH_IMMEDIATE_EXECUTE_ALU;
					ldx.alu_funct = alu_ldx;
					micro_op_queue.push_back(ldx);
				}
				else
				{
					// other mod ops (asl, rol, lsr, ror, dec, inc)
					addr_mode_function_group_2[bbb]();

					// will read the addr bus address to data bus
					MicroOp read;
					read.opcode = current_opcode;
					read.micro_op_type = READ_ADDR_BUS;
					micro_op_queue.push_back(read);

					// the dummy write. write unmodified back to addr
					MicroOp dummy_write;
					dummy_write.opcode = current_opcode;
					dummy_write.micro_op_type = WRITE_DATA_BUS_TO_ADDR_BUS;
					micro_op_queue.push_back(dummy_write);

					MicroOp mod_write;
					mod_write.opcode = current_opcode;
					mod_write.micro_op_type = MODIFY_WRITE_DATA_BUS_TO_ADDR_BUS;
					mod_write.mod_funct = mod_functions_group_2[aaa];
					micro_op_queue.push_back(mod_write);
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
				current_opcode = readpc_u8();

				decode_opcode(current_opcode);

				// set this as we have started a new opcode
				is_opcode_complete = false;
				break;
			}
			case FETCH_IMMEDIATE:
			{
				assert(op.transfer_dest != nullptr);

				u8 value = readpc_u8();

				if (op.is_high_byte)
				{
					*(op.transfer_dest + 1) = value;
				}
				else
				{
					*op.transfer_dest = value;
				}

				// small hack to set pc to the addr of the addr bus
				if (op.is_transfer_addr_to_pc)
				{
					R.pc = micro_op_addr_bus;
				}

				break;
			}
			case FETCH_IMMEDIATE_EXECUTE_ALU:
			{
				u8 value = readpc_u8();
				op.alu_funct(value);

				break;
			}
			case READ_ADDR_BUS_EXECUTE_ALU:
			{
				micro_op_data_bus = cpu_memory_module::read_memory(micro_op_addr_bus);

				op.alu_funct(micro_op_data_bus);

				break;
			}
			case READ_INDIRECT_ADDR:
			{
				// read indirect using the zeropoint pointer
				if (op.is_high_byte)
				{
					*(op.transfer_dest + 1) = cpu_memory_module::read_memory((micro_op_zeropage + 1) & 0xFF);
				}
				else
				{
					*op.transfer_dest = cpu_memory_module::read_memory(micro_op_zeropage);
				}

				break;
			}
			case READ_ADDR_BUS:
			{
				micro_op_data_bus = cpu_memory_module::read_memory(micro_op_addr_bus);
				break;
			}
			case READ_VALUE_FROM_STACK:
			{
				*op.transfer_dest = cpu_memory_module::read_memory(0x0100 + R.sp);

				if (op.is_update_flags)
				{
					update_flags_nz(*op.transfer_dest);
				}

				R.sp++;

				break;
			}
			case WRITE_VALUE_TO_STACK:
			{
				cpu_memory_module::write_memory(0x0100 + R.sp, *op.transfer_src & 0xFF);

				R.sp--;

				break;
			}
			case WRITE_DATA_BUS_TO_ADDR_BUS:
			{
				cpu_memory_module::write_memory(micro_op_addr_bus, micro_op_data_bus);
				break;
			}
			case MODIFY_WRITE_DATA_BUS_TO_ADDR_BUS:
			{
				// modify value on data bus
				micro_op_data_bus = op.mod_funct(micro_op_data_bus);

				// write it to addr
				cpu_memory_module::write_memory(micro_op_addr_bus, micro_op_data_bus);
				break;
			}
			case EXECUTE_IMPLIED_FUNCTION:
			{
				cpu_memory_module::read_memory(R.pc); // dummy read of the pc without incrementing

				// we can execute the function pointer from the micro op
				op.implied_funct();
				break;
			}
			case DATA_BUS_EXECUTE_ALU:
			{
				op.alu_funct(micro_op_data_bus);
				break;
			}
			case TRANSFER_VALUES:
			{
				// we will move one pointer value to another
				assert(op.transfer_src != nullptr && op.transfer_dest != nullptr);
				
				if (op.is_high_byte)
				{
					*(op.transfer_dest + 1) = *(op.transfer_src + 1);
				}
				else
				{
					*op.transfer_dest = *op.transfer_src;
				}

				if (op.is_update_flags)
				{
					update_flags_nz(*op.transfer_dest);
				}
				break;
			}
			case CONDITIONAL_BRANCH:
			{
				// we will fetch the offset
				s8 offset = (s8)readpc_u8();

				// check if the condition passes
				if (op.conditional_value == op.conditional_flag)
				{
					u16 new_pc = R.pc + offset;

					// push micro op to change low byte of pc
					MicroOp low_byte;
					low_byte.opcode = op.opcode;
					low_byte.micro_op_type = TRANSFER_VALUES;
					low_byte.is_high_byte = false;
					low_byte.is_update_flags = false;
					low_byte.transfer_src = (u8*)&micro_op_addr_bus;
					low_byte.transfer_dest = (u8*)&R.pc;
					micro_op_queue.push_back(low_byte);

					if ((R.pc & 0xFF00) != (new_pc & 0xFF00))
					{
						MicroOp high_byte;
						high_byte.opcode = op.opcode;
						high_byte.micro_op_type = TRANSFER_VALUES;
						high_byte.is_high_byte = true;
						high_byte.is_update_flags = false;
						high_byte.transfer_src = (u8*)&micro_op_addr_bus;
						high_byte.transfer_dest = (u8*)&R.pc;
						micro_op_queue.push_back(high_byte);
					}

					micro_op_addr_bus = new_pc; // store this for the transfer
				}

				break;
			}
			case ADD_INDEX_TO_ADDR_BUS:
			{
				// offset the existing addr on the addr bus based on the transfer_src pointer.
				u16 dummy_addr = (*op.transfer_dest & 0xFF00) | ((*op.transfer_dest + *op.transfer_src) & 0x00FF);
				cpu_memory_module::read_memory(dummy_addr); // dummy address is uncarried. lower byte wraps around. we call this to trigger side effects of the read

				if (op.is_wrap_low)
				{
					*op.transfer_dest = (*op.transfer_dest & 0xFF00) | ((*op.transfer_dest + *op.transfer_src) & 0xFF);
				}
				else
				{
					*op.transfer_dest += *op.transfer_src;
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

			return 1;
		}
	}
}