#pragma once

#include "roms/rom_chip8.h"

namespace chip8
{
	int writeInstruction(FILE * file, u16 addr, u16 opcode, const char* mnemonic, const char* operands, const char* comment)
	{
		static char writebuffer[256];
		std::string line = "";

		sprintf_s(writebuffer, 256, "0x%.3X", 0x200 + addr); // write addr
		line.append(writebuffer);
		line.append(std::string((8 - strlen(writebuffer)), ' '));

		sprintf_s(writebuffer, 256, "0x%.4X", opcode); // write opcode
		line.append(writebuffer);
		line.append(std::string((8 - strlen(writebuffer)), ' '));

		sprintf_s(writebuffer, 256, "%s", mnemonic); // write mnemonic
		line.append(writebuffer);
		line.append(std::string((8 - strlen(writebuffer)), ' '));

		sprintf_s(writebuffer, 256, "%s", operands); // write operands
		line.append(writebuffer);
		line.append(std::string((16 - strlen(writebuffer)), ' '));

		sprintf_s(writebuffer, 256, "%s", comment); // write comment
		line.append(writebuffer);

		fprintf(file, line.c_str());

		return 0;
	}

	int disassemble(const char* filename)
	{
		chip8::rom rom(filename);

		// create a texst file with asa extension
		std::string outfilename = rom.filename.substr(0, rom.filename.rfind("."));
		outfilename.append(".c8a");

		FILE* file = fopen(outfilename.c_str(), "w");

		// disasseble the rom data
		u16 PC = 0;
		while (PC < rom.romsize)
		{
			u16 addr = PC;

			// fetch opcode
			u16 opcode = (rom.romdata[PC++] << 8);
			opcode |= rom.romdata[PC++];

			u16 NNN = opcode & 0x0FFF;
			u8 NN = opcode & 0x00FF;
			u8 N = opcode & 0x000F;
			u8 X = (opcode & 0x0F00) >> 8;
			u8 Y = (opcode & 0x00F0) >> 4;

			bool error = false;
			char operandbuffer[256];
			// decode opcode
			switch (opcode & 0xF000)
			{
			case 0x0000:
			{
				switch (opcode & 0x000F)
				{
				case 0x0000: // 00E0: clears the screen
					writeInstruction(file, addr, opcode, "CLR", "", "# 00E0: clears the screen");
					break;
				case 0x000E: // 00EE: returns from subroutine
					writeInstruction(file, addr, opcode, "RTS", "", "# 00EE: returns from subroutine");
					break;
				default:
					error = true;
				}
				break;
			}
			case 0x1000: // 1NNN: set program counter to NNN
				sprintf_s(operandbuffer, 256, "0x%.3X", NNN);
				writeInstruction(file, addr, opcode, "JUMP", operandbuffer, "# 1NNN: set program counter to NNN");
				break;
			case 0x2000: // 2NNN: call subroutine at addr NNN
				sprintf_s(operandbuffer, 256, "0x%.3X", NNN);
				writeInstruction(file, addr, opcode, "CALL", operandbuffer, "# 2NNN: call subroutine at addr NNN");
				break;
			case 0x3000: // 3XNN: skip next instruction if X register == NN
				sprintf_s(operandbuffer, 256, "r%d,$%d", X, NN);
				writeInstruction(file, addr, opcode, "SKE", operandbuffer, "# 3XNN: skip next instruction if X register == NN");
				break;
			case 0x4000: // 4XNN: skip next instruction if X register != NN
				sprintf_s(operandbuffer, 256, "r%d,$%d", X, NN);
				writeInstruction(file, addr, opcode, "SKNE", operandbuffer, "# 4XNN: skip next instruction if X register != NN");
				break;
			case 0x5000: // 5XY0: skip next instruction if X register = Y register
				sprintf_s(operandbuffer, 256, "r%d,r%d", X, Y);
				writeInstruction(file, addr, opcode, "SKRE", operandbuffer, "# 5XY0: skip next instruction if X register = Y register");
				break;
			case 0x6000: // 6XNN: put NN into X register
				sprintf_s(operandbuffer, 256, "$%d,r%d", NN, X);
				writeInstruction(file, addr, opcode, "LOAD", operandbuffer, "# 6XNN: put NN into X register");
				break;
			case 0x7000: // 7XNN: adds NN to the value od register X
				sprintf_s(operandbuffer, 256, "$%d,r%d", NN, X);
				writeInstruction(file, addr, opcode, "ADD", operandbuffer, "# 7XNN: adds NN to the value of register X");
				break;
			case 0x8000:
			{
				switch (opcode & 0x000F)
				{
				case 0x0000: // 8XY0: stores value of register Y into register X
					sprintf_s(operandbuffer, 256, "r%d,r%d", Y, X);
					writeInstruction(file, addr, opcode, "MOVE", operandbuffer, "# 8XY0: stores value of register Y into register X");
					break;
				case 0x0001: // 8XY1: OR X and Y and store in X
					sprintf_s(operandbuffer, 256, "r%d,r%d", Y, X);
					writeInstruction(file, addr, opcode, "OR", operandbuffer, "# 8XY1: OR X and Y and store in X");
					break;
				case 0x0002: // 8XY2: AND X and Y and store in X
					sprintf_s(operandbuffer, 256, "r%d,r%d", Y, X);
					writeInstruction(file, addr, opcode, "AND", operandbuffer, "# 8XY2: AND X and Y and store in X");
					break;
				case 0x0003: // 8XY3: XOR X and Y and store in X
					sprintf_s(operandbuffer, 256, "r%d,r%d", Y, X);
					writeInstruction(file, addr, opcode, "XOR", operandbuffer, "# 8XY3: XOR X and Y and store in X");
					break;
				case 0x0004: // 8XY4: ADD X and Y and store in X. set carry flag if > 255
					sprintf_s(operandbuffer, 256, "r%d,r%d", Y, X);
					writeInstruction(file, addr, opcode, "ADD", operandbuffer, "# 8XY4: ADD X and Y and store in X. set carry flag if > 255");
					break;
				case 0x0005: // 8XY5: SUB Y from X and store in X. set carry flag to 0 if borrow, 1 if not
					sprintf_s(operandbuffer, 256, "r%d,r%d", Y, X);
					writeInstruction(file, addr, opcode, "SUB", operandbuffer, "# 8XY5: SUB Y from X and store in X. set carry flag to 0 if borrow, 1 if not");
					break;
				case 0x0006: // 8XY6: if lowest bit of X is 1 set carry flag to 1, then X is divided by 2
					sprintf_s(operandbuffer, 256, "r%d", X);
					writeInstruction(file, addr, opcode, "SHR", operandbuffer, "# 8XY6: if lowest bit of X is 1 set carry flag to 1, then X is divided by 2");
					break;
				case 0x0007: // 8XY7: if X > Y there is borrow, set carry flag to 0, 1 if not. then X = Y - X
					sprintf_s(operandbuffer, 256, "r%d,r%d", Y, X);
					writeInstruction(file, addr, opcode, "SUBN", operandbuffer, "# 8XY7: if X > Y there is borrow, set carry flag to 0, 1 if not. then X = Y - X");
					break;
				case 0x000E: // 8XYE: if highest bit of Y is 1 set carry to 1, then multiply X by 2
					sprintf_s(operandbuffer, 256, "r%d", X);
					writeInstruction(file, addr, opcode, "SHL", operandbuffer, "# 8XYE: if highest bit of Y is 1 set carry to 1, then multiply X by 2");
					break;
				default:
					error = true;
				}
				break;
			}
			case 0x9000: // 9XY0: skip next instruction if X != Y
				sprintf_s(operandbuffer, 256, "r%d,r%d", X, Y);
				writeInstruction(file, addr, opcode, "SKRNE", operandbuffer, "# 9XY0: skip next instruction if X != Y");
				break;
			case 0xA000: // ANNN: put NNN to register I
				sprintf_s(operandbuffer, 256, "$%d", NNN);
				writeInstruction(file, addr, opcode, "LOADI", operandbuffer, "# ANNN: load NNN to register I");
				break;
			case 0xB000: // BNNN: set program counter to NNN + V[0]
				sprintf_s(operandbuffer, 256, "$%d", NNN);
				writeInstruction(file, addr, opcode, "JUMPR", operandbuffer, "# BNNN: set program counter to NNN + r1");
				break;
			case 0xC000: // CXNN: set X to rand(0 - 255) & NN
				sprintf_s(operandbuffer, 256, "$%d,r%d", NN, X);
				writeInstruction(file, addr, opcode, "RAND", operandbuffer, "# CXNN: set X to rand(0 - 255) & NN");
				break;
			case 0xD000: // DXYN: Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
				sprintf_s(operandbuffer, 256, "r%d,r%d,$%d", X, Y, N);
				writeInstruction(file, addr, opcode, "DRAW", operandbuffer, "# DXYN: Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision");
				break;
			case 0xE000:
			{
				switch (opcode & 0x00FF)
				{
				case 0x009E: // EX9E: skip next instruction if key[X] is pressed
					sprintf_s(operandbuffer, 256, "r%d", X);
					writeInstruction(file, addr, opcode, "SKEY", operandbuffer, "# EX9E: skip next instruction if key[X] is pressed");
					break;
				case 0x00A1: // EXA1: skip next instruction if key[X] is not pressed
					sprintf_s(operandbuffer, 256, "r%d", X);
					writeInstruction(file, addr, opcode, "SNKEY", operandbuffer, "# EXA1: skip next instruction if key[X] is not pressed");
					break;
				default:
					error = true;
				}
				break;
			}
			case 0xF000:
			{
				switch (opcode & 0x00FF)
				{
				case 0x0007: // FX07: set X to delay timer
					sprintf_s(operandbuffer, 256, "r%d", X);
					writeInstruction(file, addr, opcode, "MOVED", operandbuffer, "# FX07: set X to delay timer");
					break;
				case 0x000A: // FX0A: block instructions until keypress. add key value to X
					sprintf_s(operandbuffer, 256, "r%d", X);
					writeInstruction(file, addr, opcode, "WKEY", operandbuffer, "# FX0A: block instructions until keypress. add key value to X");
					break;
				case 0x0015: // FX15: set delay timer to X
					sprintf_s(operandbuffer, 256, "r%d", X);
					writeInstruction(file, addr, opcode, "MOVED", operandbuffer, "# FX15: set delay timer to X");
					break;
				case 0x0018: // FX18: set sound timer to X
					sprintf_s(operandbuffer, 256, "r%d", X);
					writeInstruction(file, addr, opcode, "MOVES", operandbuffer, "# FX18: set sound timer to X");
					break;
				case 0x001E: // FX1E: add X to I
					sprintf_s(operandbuffer, 256, "r%d", X);
					writeInstruction(file, addr, opcode, "ADDI", operandbuffer, "# FX1E: add X to I");
					break;
				case 0x0029: // FX29: set I to the memory addr of font character in X
					sprintf_s(operandbuffer, 256, "r%d", X);
					writeInstruction(file, addr, opcode, "LDSPR", operandbuffer, "# FX29: set I to the memory addr of font character in X");
					break;
				case 0x0033: // FX33: store binary decimals into I, I+1, I+2
					sprintf_s(operandbuffer, 256, "r%d", X);
					writeInstruction(file, addr, opcode, "BCD", operandbuffer, "# FX33: store binary decimals of X into I, I+1, I+2");
					break;
				case 0x0055: // FX55: store values of 0 to X in memory starting at I
					sprintf_s(operandbuffer, 256, "r%d", X);
					writeInstruction(file, addr, opcode, "STORE", operandbuffer, "# FX55: store values of 0 to X in memory starting at I");
					break;
				case 0x0065: // FX65: store memory starting at I into 0 to X
					sprintf_s(operandbuffer, 256, "r%d", X);
					writeInstruction(file, addr, opcode, "READ", operandbuffer, "# FX65: store memory starting at I into 0 to X");
					break;
				default:
					error = true;
				}
				break;
			}
			default:
				error = true;
				break;
			}

			if (error)
			{
				sprintf_s(operandbuffer, 256, "r%d", X);
				writeInstruction(file, addr, opcode, "DEFW", "", "# Unknown opcode");
			}

			// start a newline
			fprintf(file, "\n");
		}

		fclose(file);

		return 0;
	}
}