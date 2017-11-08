#pragma once

#include "defines.h"

#include "rom.h"

namespace gameboy
{
	namespace disassembler
	{
		#define WRITE_HEX_16(x) "0x" << std::setfill('0') << std::setw(4) << std::uppercase << std::hex << x
		#define WRITE_HEX_8(x) "0x" << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << x

		struct symbol
		{
			symbol()
			{
				addr = 0x0;
				mnemonic = "";
				operands = "";
				opcode = 0x0;
				cb_opcode = 0x0;
				comment = "";
			}

			u16 addr;
			std::string mnemonic;
			std::string operands;
			u8 opcode;
			u8 cb_opcode;
			std::string comment;
		};

		std::string condition_function_str[] = { "NZ", "Z", "NC", "C", "INVALID", "INVALID", "INVALID", "INVALID" };
		std::string register_pairs_str[] = { "BC", "DE", "HL", "SP" };
		std::string register_pairs2_str[] = { "BC", "DE", "HL", "AF" };
		std::string register_single_str[] = { "B", "C", "D", "E", "H", "L", "(HL)", "A" };
		std::string alu_function_str[] = { "ADD", "ADC", "SUB", "SBC", "AND", "XOR", "OR", "CP" };
		std::string rot_function_str[] = { "RLC", "RRC", "RL", "RR", "SLA", "SRA", "SWAP", "SRL" };

		u16 PC;

		// read 8 and 16 bit at PC. increment PC
		inline u8 readpc_u8()
		{
			u8 val = memory_module::read_memory(PC++);

			return val;
		}

		inline u16 readpc_u16()
		{
			// lsb is first in memory
			u16 val = memory_module::read_memory(PC++);
			val |= (memory_module::read_memory(PC++) << 8);

			return val;
		}
		
		int disassemble_nonprefixed(u8 opcode, symbol& sym)
		{
			u8 x = (opcode >> 6);
			u8 y = (opcode >> 3) & 0x7;
			u8 z = (opcode & 0x7);
			u8 p = (opcode >> 4) & 0x3;
			u8 q = (opcode >> 3) & 0x1;
			
			std::stringstream mnemonic_stream;
			std::stringstream operand_stream;
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
						sym.mnemonic = "NOP";
						break;
					case 0x1:
					{
						// LD mem NN with SP
						u16 addr = readpc_u16();

						sym.mnemonic = "LD";
						operand_stream << "(" << WRITE_HEX_16(addr) << "), SP";
						sym.operands = operand_stream.str();
						break;
					}
					case 0x2:
						// STOP
						sym.mnemonic = "STOP";
						break;
					case 0x3:
						// JR d
						sym.mnemonic = "JR";
						operand_stream << (s32)((s8)readpc_u8());
						sym.operands = operand_stream.str();
						break;
					case 0x4:
					case 0x5:
					case 0x6:
					case 0x7:
						// JR conditions[y - 4], d - relative jump
						sym.mnemonic = "JR";
						operand_stream << condition_function_str[y - 4] << ", " << (s32)((s8)readpc_u8());
						sym.operands = operand_stream.str();
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
						sym.mnemonic = "LD";
						operand_stream << register_pairs_str[p] << ", " << readpc_u16();
						sym.operands = operand_stream.str();
						break;
					case 0x1:
						// ADD HL with register_pairs[p]
						sym.mnemonic = "ADD";
						operand_stream << "HL, " << register_pairs_str[p];
						sym.operands = operand_stream.str();
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
							sym.mnemonic = "LD";
							sym.operands = "(BC), A";
							break;
						case 0x1:
							// LD (DE) with A
							sym.mnemonic = "LD";
							sym.operands = "(DE), A";
							break;
						case 0x2:
							// LDI (HL) with A. inc HL
							sym.mnemonic = "LDI";
							sym.operands = "(HL), A";
							break;
						case 0x3:
							// LDD (HL) with A. decr HL
							sym.mnemonic = "LDD";
							sym.operands = "(HL), A";
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
							sym.mnemonic = "LD";
							sym.operands = "A, (BC)";
							break;
						case 0x1:
							// LD A with (DE)
							sym.mnemonic = "LD";
							sym.operands = "A, (DE)";
							break;
						case 0x2:
							// LDI A with (HL). inc HL
							sym.mnemonic = "LDI";
							sym.operands = "A, (HL)";
							break;
						case 0x3:
							// LDD A with (HL). decr HL
							sym.mnemonic = "LDD";
							sym.operands = "A, (HL)";
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
						sym.mnemonic = "INC";
						operand_stream << register_pairs_str[p];
						sym.operands = operand_stream.str();
						break;
					case 0x1:
						// DEC register_pairs[p]
						sym.mnemonic = "DEC";
						operand_stream << register_pairs_str[p];
						sym.operands = operand_stream.str();
						break;
					}
					break;
				}
				case 0x4: // z = 4
					// INC register_single[y]
					sym.mnemonic = "INC";
					operand_stream << register_single_str[y];
					sym.operands = operand_stream.str();
					break;
				case 0x5: // z = 5
					// DEC register_single[y]
					sym.mnemonic = "DEC";
					operand_stream << register_single_str[y];
					sym.operands = operand_stream.str();
					break;
				case 0x6: // z = 6
					// LD register_single[y] with n
					sym.mnemonic = "LD";
					operand_stream << register_single_str[y] << ", " << (u32)readpc_u8();
					sym.operands = operand_stream.str();
					break;
				case 0x7: // z = 7
				{
					switch (y)
					{
					case 0x0:
					{
						// RLC A
						sym.mnemonic = "RLC";
						sym.operands = "A";
						break;
					}
					case 0x1:
					{
						// RRC A
						sym.mnemonic = "RRC";
						sym.operands = "A";
						break;
					}
					case 0x2:
					{
						// RL A
						sym.mnemonic = "RL";
						sym.operands = "A";
						break;
					}
					case 0x3:
					{
						// RR A
						sym.mnemonic = "RR";
						sym.operands = "A";
						break;
					}
					case 0x4:
					{
						// DAA
						sym.mnemonic = "DAA";
						break;
					}
					case 0x5:
						// CPL
						sym.mnemonic = "CPL";
						break;
					case 0x6:
						// SCF
						sym.mnemonic = "SCF";
						break;
					case 0x7:
						// CCF
						sym.mnemonic = "CCF";
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
					sym.mnemonic = "HALT";
				}
				else
				{
					// LD register_single[y] with register_single[z]
					sym.mnemonic = "LD";
					operand_stream << register_single_str[y] << ", " << register_single_str[z];
					sym.operands = operand_stream.str();
				}
				break;
			} // end x = 1
			case 0x2: // x = 2
			{
				// alu[y] with register_single[z]
				mnemonic_stream << alu_function_str[y];
				sym.mnemonic = mnemonic_stream.str();
				operand_stream << "A, " << register_single_str[z];
				sym.operands = operand_stream.str();
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
						sym.mnemonic = "RET";
						operand_stream << condition_function_str[y];
						sym.operands = operand_stream.str();
						break;
					case 0x4:
						// LD mem(FF00 + n) with A
						sym.mnemonic = "LDH";
						operand_stream << "(" << WRITE_HEX_16(0xFF00 + (u32)readpc_u8()) << "), A";
						sym.operands = operand_stream.str();
						break;
					case 0x5:
					{
						// ADD SP with (signed)n
						sym.mnemonic = "ADD";
						operand_stream << "SP, " << (s32)((s8)readpc_u8());
						sym.operands = operand_stream.str();
						break;
					}
					case 0x6:
						// LD A with mem(FF00 + n)
						sym.mnemonic = "LDH";
						operand_stream << "A, " << WRITE_HEX_16(0xFF00 + (u32)readpc_u8());
						sym.operands = operand_stream.str();
						break;
					case 0x7:
					{
						// ADD (signed)n to SP then LD HL with SP
						sym.mnemonic = "LDHL";
						operand_stream << "SP, " << (s32)((s8)readpc_u8());
						sym.operands = operand_stream.str();
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
						sym.mnemonic = "POP";
						operand_stream << register_pairs2_str[p];
						sym.operands = operand_stream.str();
					}
					else
					{
						switch (p)
						{
						case 0x0:
							// RET
							sym.mnemonic = "RET";
							break;
						case 0x1:
							// RETI
							sym.mnemonic = "RETI";
							break;
						case 0x2:
							// JP (HL)
							sym.mnemonic = "JP";
							sym.operands = "(HL)";
							break;
						case 0x3:
							// LD SP with HL
							sym.mnemonic = "LD";
							sym.operands = "SP, HL";
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
						// JP to nn if condition_funct[y]
						sym.mnemonic = "JP";
						operand_stream << condition_function_str[y] << ", " << readpc_u16();
						sym.operands = operand_stream.str();
						break;
					}
					case 0x4:
						// LD mem(FF00 + C) with A
						sym.mnemonic = "LDH";
						sym.operands = "(C), A";
						break;
					case 0x5:
						// LD mem(nn) with A
						sym.mnemonic = "LD";
						operand_stream << WRITE_HEX_16(readpc_u16()) << "), A";
						sym.operands = operand_stream.str();
						break;
					case 0x6:
						// LD A with mem(FF00 + C)
						sym.mnemonic = "LDH";
						sym.operands = "A, (C)";
						break;
					case 0x7:
						// LD A with mem(nn)
						sym.mnemonic = "LD";
						operand_stream << "A, (" << WRITE_HEX_16(readpc_u16()) << ")";
						sym.operands = operand_stream.str();
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
						sym.mnemonic = "JP";
						operand_stream << WRITE_HEX_16(readpc_u16());
						sym.operands = operand_stream.str();
						break;
					case 0x1:
						// CB prefix
						assert(0);
						break;
					case 0x2:
					case 0x3:
					case 0x4:
					case 0x5:
						// unsupported by gameboy
						sym.mnemonic = "NOP";
						sym.comment = "Unsupported opcode";
						break;
					case 0x6:
						// DI - disable interupts
						sym.mnemonic = "DI";
						break;
					case 0x7:
						// EI - enable interupts
						sym.mnemonic = "EI";
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
						// CALL nn if condition_funct[y]
						sym.mnemonic = "CALL";
						operand_stream << condition_function_str[y] << ", " << WRITE_HEX_16(readpc_u16());
						sym.operands = operand_stream.str();
						break;
					}
					case 0x4:
					case 0x5:
					case 0x6:
					case 0x7:
						// unsupported by gameboy
						sym.mnemonic = "NOP";
						sym.comment = "Unsupported opcode";
						break;
					}
					break;
				}
				case 0x5: // z = 5
				{
					if (q == 0)
					{
						// PUSH register_pairs2[p]
						sym.mnemonic = "PUSH";
						operand_stream << register_pairs2_str[p];
						sym.operands = operand_stream.str();
					}
					else
					{
						if (p == 0)
						{
							// CALL nn
							sym.mnemonic = "CALL";
							operand_stream << WRITE_HEX_16(readpc_u16());
							sym.operands = operand_stream.str();
						}
						else
						{
							// unsupported by gameboy
							sym.mnemonic = "NOP";
							sym.comment = "Unsupported opcode";
						}
					}
					break;
				}
				case 0x6: // z = 6
				{
					// alu[y] with n
					mnemonic_stream << alu_function_str[y];
					sym.mnemonic = mnemonic_stream.str();
					operand_stream << "A, " << (u32)readpc_u8();
					sym.operands = operand_stream.str();
					break;
				}
				case 0x7: // z = 7
				{
					// RST at pc 7 * 8. basically a CALL
					sym.mnemonic = "RST";
					operand_stream << (y * 8);
					sym.operands = operand_stream.str();
					break;
				}
				}
				break;
			}
			}

			return 0;
		}

		int disassemble_prefixed_cb(u8 opcode, symbol& sym)
		{
			u8 x = (opcode >> 6);
			u8 y = (opcode >> 3) & 0x7;
			u8 z = (opcode & 0x7);
			u8 p = (opcode >> 4) & 0x3;
			u8 q = (opcode >> 3) & 0x1;
			
			std::stringstream mnemonic_stream;
			std::stringstream operand_stream;

			switch (x)
			{
			case 0x0:
				// rot_function[y] with register_single[z]
				mnemonic_stream << rot_function_str[y];
				sym.mnemonic = mnemonic_stream.str();
				operand_stream << register_single_str[z];
				sym.operands = operand_stream.str();
				break;
			case 0x1:
				// test bit y from register_single[z]
				sym.mnemonic = "BIT";
				operand_stream << (u32)y << ", " << register_single_str[z];
				sym.operands = operand_stream.str();
				break;
			case 0x2:
				// reset bit y from register_single[z]
				sym.mnemonic = "RES";
				operand_stream << (u32)y << ", " << register_single_str[z];
				sym.operands = operand_stream.str();
				break;
			case 0x3:
				// set bit y from register_single[z]
				sym.mnemonic = "SET";
				operand_stream << (u32)y << ", " << register_single_str[z];
				sym.operands = operand_stream.str();
				break;
			}

			return 0;
		}

		u16 disassemble_instr(u16 addr, symbol& sym)
		{
			PC = addr;

			sym.addr = PC;
			sym.opcode = readpc_u8();

			if (sym.opcode == 0xCB)
			{
				sym.cb_opcode = readpc_u8();
				disassemble_prefixed_cb(sym.opcode, sym);
			}
			else
			{
				disassemble_nonprefixed(sym.opcode, sym);
			}

			return PC;
		}

		int write_instruction(FILE * file, u16 addr, u8 opcode, u8 cb_opcode, const char* mnemonic, const char* operands, const char* comment)
		{
			static char writebuffer[256];
			std::string line = "";

			sprintf_s(writebuffer, 256, "0x%.4X", addr); // write addr
			line.append(writebuffer);
			line.append(std::string((8 - strlen(writebuffer)), ' '));
			
			if (opcode == 0xCB)
			{
				sprintf_s(writebuffer, 256, "0x%.2X 0x%.2X", opcode, cb_opcode); // write opcode + cb_opcode
				line.append(writebuffer);
				line.append(std::string((16 - strlen(writebuffer)), ' '));
			}
			else
			{
				sprintf_s(writebuffer, 256, "0x%.2X", opcode); // write opcode
				line.append(writebuffer);
				line.append(std::string((16 - strlen(writebuffer)), ' '));
			}

			sprintf_s(writebuffer, 256, "%s", mnemonic); // write mnemonic
			line.append(writebuffer);
			line.append(std::string((8 - strlen(writebuffer)), ' '));

			sprintf_s(writebuffer, 256, "%s", operands); // write operands
			line.append(writebuffer);
			line.append(std::string((16 - strlen(writebuffer)), ' '));

			sprintf_s(writebuffer, 256, "%s", comment); // write comment
			line.append(writebuffer);

			fprintf(file, line.c_str());
			fprintf(file, "\n");

			return 0;
		}

		int disassemble_to_file(const char* filename)
		{
			// export to file
			FILE* file = fopen(filename, "w");

			// disasseble the rom data
			PC = 0x0;
			while (PC <= memory_module::memory_map[memory_module::MEMORY_CATRIDGE_SWITCHABLE_ROM].addr_max)
			{
				symbol sym;
				PC = disassemble_instr(PC, sym);
				write_instruction(file, sym.addr, sym.opcode, sym.cb_opcode, sym.mnemonic.c_str(), sym.operands.c_str(), sym.comment.c_str());
			}

			fclose(file);

			return 0;
		}
	}
}