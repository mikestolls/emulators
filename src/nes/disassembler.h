#pragma once

#include "defines.h"

namespace nes
{
	namespace disassembler
	{
		struct symbol
		{
			u16 addr = 0x0;
			std::string mnemonic = "";
			std::string operands = "";
			u8 opcode = 0x0;
			u8 cb_opcode = 0x0;
			std::string comment = "";
		};

		u16 pc;

		inline u8 readpc_u8()
		{
			u8 val = cpu_memory_module::read_memory(pc++, true);

			return val;
		}

		u16 disassemble_instr(u16 addr, symbol& sym)
		{
			pc = addr;

			sym.addr = pc;
			sym.opcode = readpc_u8();

			return pc;
		}

		u16 disassemble_instr(u16 addr)
		{
			symbol sym;
			return disassemble_instr(addr, sym);
		}

		int write_instruction(FILE* file, u16 addr, u8 opcode, u8 cb_opcode, const char* mnemonic, const char* operands, const char* comment)
		{
			return 0;
		}

		int disassemble_to_file(const std::string& filename)
		{
			printf("Error: nes disassembler not implemeted");
			return -1;
		}
	}
}