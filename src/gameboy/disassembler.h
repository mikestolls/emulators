#pragma once

#include "defines.h"

#include "rom.h"

namespace gameboy
{
	namespace disassembler
	{
		gameboy::rom rom;
		u16 PC;

		// read 8 and 16 bit at PC. increment PC
		inline u8 readpc_u8()
		{
			u8 val = rom.romdata[PC++];

			return val;
		}

		inline u16 readpc_u16()
		{
			// lsb is first in memory
			u16 val = rom.romdata[PC++];
			val |= (rom.romdata[PC++] << 8);

			return val;
		}

		int disassemble_nonprefixed(u8 opcode)
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
						break;
					case 0x1:
					{
						// LD mem NN with SP
						break;
					}
					case 0x2:
						// STOP
						break;
					case 0x3:
						// JR d
						break;
					case 0x4:
					case 0x5:
					case 0x6:
					case 0x7:
					{
						// JR conditions[y - 4], d - relative jump
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
						break;
					case 0x1:
						// ADD HL with register_pairs[p]
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
							break;
						case 0x1:
							// LD (DE) with A
							break;
						case 0x2:
							// LDI (HL) with A. inc HL
							break;
						case 0x3:
							// LDD (HL) with A. decr HL
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
							break;
						case 0x1:
							// LD A with (DE)
							break;
						case 0x2:
							// LDI A with (HL). inc HL
							break;
						case 0x3:
							// LDD A with (HL). decr HL
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
						break;
					case 0x1:
						// DEC register_pairs[p]
						break;
					}
					break;
				}
				case 0x4: // z = 4
					// INC register_single[y]
					break;
				case 0x5: // z = 5
					// DEC register_single[y]
					break;
				case 0x6: // z = 6
					// LD register_single[y] with n
					break;
				case 0x7: // z = 7
				{
					switch (y)
					{
					case 0x0:
					{
						// RLC A
						break;
					}
					case 0x1:
					{
						// RRC A
						break;
					}
					case 0x2:
					{
						// RL A
						break;
					}
					case 0x3:
					{
						// RR A
						break;
					}
					case 0x4:
					{
						// DAA
						break;
					}
					case 0x5:
						// CPL
						break;
					case 0x6:
						// SCF
						break;
					case 0x7:
						// CCF
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
				}
				else
				{
					// LD register_single[y] with register_single[z]
				}
				break;
			} // end x = 1
			case 0x2: // x = 2
			{
				// alu[y] with register_single[z]
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
						break;
					case 0x4:
						// LD mem(FF00 + n) with A
						break;
					case 0x5:
					{
						// ADD SP with (signed)n
						break;
					}
					case 0x6:
						// LD A with mem(FF00 + n)
						break;
					case 0x7:
					{
						// ADD (signed)n to SP then LD HL with SP
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
					}
					else
					{
						switch (p)
						{
						case 0x0:
							// RET
							break;
						case 0x1:
							// RETI
							break;
						case 0x2:
							// JP (HL)
							break;
						case 0x3:
							// LD SP with HL
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
						break;
					}
					case 0x4:
						// LD mem(FF00 + C) with A
						break;
					case 0x5:
						// LD mem(nn) with A
						break;
					case 0x6:
						// LD A with mem(FF00 + C)
						break;
					case 0x7:
						// LD A with mem(nn)
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
						break;
					case 0x6:
						// DI - disable interupts
						break;
					case 0x7:
						// EI - enable interupts
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
						// CALL nn if condition_funct[y]
					}
					case 0x4:
					case 0x5:
					case 0x6:
					case 0x7:
						// unsupported by gameboy
						break;
					}
					break;
				}
				case 0x5: // z = 5
				{
					if (q == 0)
					{
						// PUSH register_pairs2[p]
					}
					else
					{
						if (p == 0)
						{
							// CALL nn
						}
						else
						{
							// unsupported by gameboy
						}
					}
					break;
				}
				case 0x6: // z = 6
				{
					// alu[y] with n
					break;
				}
				case 0x7: // z = 7
				{
					// RST at pc 7 * 8. basically a CALL
					break;
				}
				}
				break;
			}
			}

			return cycles;
		}

		int disassemble_prefixed_cb(u8 opcode)
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
				break;
			case 0x1:
				// test bit y from register_single[z]
				break;
			case 0x2:
				// reset bit y from register_single[z]
				break;
			case 0x3:
				// set bit y from register_single[z]
				break;
			}

			return cycles;
		}

		int disassemble(const char* filename)
		{
			rom = gameboy::rom(filename);

			// create a texst file with asa extension
			std::string outfilename = rom.filename.substr(0, rom.filename.rfind("."));
			outfilename.append(".gbasm");

			FILE* file = fopen(outfilename.c_str(), "w");

			// disasseble the rom data
			PC = 0x0;
			while (PC < rom.romsize)
			{
				u8 opcode = readpc_u8();

				if (opcode == 0xCB)
				{
					opcode = readpc_u8();
					disassemble_prefixed_cb(opcode);
				}
				else
				{
					disassemble_nonprefixed(opcode);
				}
			}

			fclose(file);

			return 0;
		}
	}
}