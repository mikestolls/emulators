#pragma once

#include "defines.h"
#include "roms/rom_chip8.h"

namespace chip8
{
	std::map<std::string, u16> symbolTable;

	// begin - functions to parse operands and generate final opcode
	static u16 GetOperandValueType(const char* str, int mask)
	{
		u16 value = 0;

		if (strncmp(str, "r", 1) == 0)
		{
			value = atoi(&str[1]) & 0xF;
		}
		else if (strncmp(str, "0x", 2) == 0)
		{
			char* ptr;
			value = strtol(str, &ptr, 16) & 0xFFF;
		}
		else if (strncmp(str, "$", 1) == 0)
		{
			value = atoi(&str[1]) & 0xFFF;
		}
		else // assume others are labels
		{
			value = symbolTable[str];
		}

		value &= mask;

		return value;
	}

	static u16 assemble_none(std::vector<std::string>& inst)
	{
		return 0x0;
	}

	static u16 assemble_NNN(std::vector<std::string>& split)
	{
		if (split.size() != 2)
		{
			printf("Error: assemble_NNN");
			return 0x0;
		}

		u16 value = GetOperandValueType(split[1].c_str(), 0x0FFF);

		return value;
	}

	static u16 assemble_NN_X(std::vector<std::string>& split)
	{
		if (split.size() != 3)
		{
			printf("Error: assemble_NN_X");
			return 0x0;
		}

		u16 value = (GetOperandValueType(split[2].c_str(), 0x000F) << 8);
		value |= GetOperandValueType(split[1].c_str(), 0x00FF);

		return value;
	}

	static u16 assemble_X_NN(std::vector<std::string>& split)
	{
		if (split.size() != 3)
		{
			printf("Error: assemble_X_NN");
			return 0x0;
		}

		u16 value = (GetOperandValueType(split[1].c_str(), 0x000F) << 8);
		value |= GetOperandValueType(split[2].c_str(), 0x00FF);

		return value;
	}

	static u16 assemble_X_Y(std::vector<std::string>& split)
	{
		if (split.size() != 3)
		{
			printf("Error: assemble_X_Y");
			return 0x0;
		}

		u16 value = (GetOperandValueType(split[1].c_str(), 0x000F) << 8);
		value |= (GetOperandValueType(split[2].c_str(), 0x000F) << 4);

		return value;
	}

	static u16 assemble_Y_X(std::vector<std::string>& split)
	{
		if (split.size() != 3)
		{
			printf("Error: assemble_Y_X");
			return 0x0;
		}

		u16 value = (GetOperandValueType(split[2].c_str(), 0x000F) << 8);
		value |= (GetOperandValueType(split[1].c_str(), 0x000F) << 4);

		return value;
	}

	static u16 assemble_X_Y_N(std::vector<std::string>& split)
	{
		if (split.size() != 4)
		{
			printf("Error: assemble_X_Y_N");
			return 0x0;
		}

		u16 value = (GetOperandValueType(split[1].c_str(), 0x000F) << 8);
		value |= (GetOperandValueType(split[2].c_str(), 0x000F) << 4);
		value |= GetOperandValueType(split[3].c_str(), 0x000F);

		return value;
	}

	static u16 assemble_X(std::vector<std::string>& split)
	{
		if (split.size() != 2)
		{
			printf("Error: assemble_X");
			return 0x0;
		}

		u16 value = (GetOperandValueType(split[1].c_str(), 0x000F) << 8);

		return value;
	}
	// end - functions to parse operands and generate final opcode

	// begin - instruction list for chip8
	struct instruction
	{
		std::string mnemonic;
		u16 opcode;
		u16(*function)(std::vector<std::string>&);
	};

	static const instruction instructions[] = {
		{ "CLR", 0x00E0, assemble_none },
		{ "RTS", 0x00EE, assemble_none },
		{ "JUMP", 0x1000, assemble_NNN },
		{ "CALL", 0x2000, assemble_NNN },
		{ "SKE", 0x3000, assemble_X_NN },
		{ "SKNE", 0x4000, assemble_X_NN },
		{ "SKRE", 0x5000, assemble_X_Y },
		{ "LOAD", 0x6000, assemble_NN_X },
		{ "ADD", 0x7000, assemble_NN_X },
		{ "MOVE", 0x8000, assemble_Y_X },
		{ "OR", 0x8001, assemble_Y_X },
		{ "AND", 0x8002, assemble_Y_X },
		{ "XOR", 0x8003, assemble_Y_X },
		{ "ADDR", 0x8004, assemble_Y_X },
		{ "SUB", 0x8005, assemble_Y_X },
		{ "SHR", 0x8006, assemble_X },
		{ "SHL", 0x800E, assemble_X },
		{ "SKRNE", 0x9000, assemble_X_Y },
		{ "LOADI", 0xA000, assemble_NNN },
		{ "JUMPR", 0xB000, assemble_NNN },
		{ "RAND", 0xC000, assemble_NN_X },
		{ "DRAW", 0xD000, assemble_X_Y_N },
		{ "SKEY", 0xE09E, assemble_X },
		{ "SNKEY", 0xE0A1, assemble_X },
		{ "MOVED", 0xF007, assemble_X },
		{ "WKEY", 0xF00A, assemble_X },
		{ "LOADD", 0xF015, assemble_X },
		{ "LOADS", 0xF018, assemble_X },
		{ "ADDI", 0xF01E, assemble_X },
		{ "LDSPR", 0xF029, assemble_X },
		{ "BCD", 0xF033, assemble_X },
		{ "STORE", 0xF055, assemble_X },
		{ "READ", 0xF065, assemble_X },
	};

	static const u16 instructionCount = sizeof(instructions) / sizeof(instructions[0]);
	//end - instruction list for chip8

	int assemble(const char* filename)
	{
		std::string outfilename = filename;

		// create a texst file with asa extension
		outfilename = outfilename.substr(0, outfilename.rfind("."));
		outfilename.append(".c8");

		std::ifstream infile(filename);

		// open out binary file
		FILE* file = fopen(outfilename.c_str(), "wb");

		// assemble the rom data
		std::string line;
		std::vector<std::vector<std::string>> instructionLines;
		std::vector<std::string> splitVec;

		u16 PC = 0x200;

		// first pass to determine addr of labels
		while (std::getline(infile, line))
		{
			// remove the comments
			line = line.substr(0, line.rfind(';'));

			if (line.length() == 0)
			{
				continue;
			}

			line = string_helpers::replaceAll(line, std::string("\t"), std::string(" "));
			line = string_helpers::replaceAll(line, std::string(","), std::string(""));
			line = string_helpers::trim(line);

			// check if there is a label on the line
			size_t pos = line.rfind(':');
			if (pos != std::string::npos)
			{
				// store the symbol for the label
				std::string symbol = line.substr(0, pos);
				symbolTable[symbol] = PC;

				line = line.substr(pos + 1);
			}

			if (line.rfind(':') == std::string::npos)
			{
				string_helpers::split(splitVec, line, ' ');

				// bring mnemonic name to upper
				std::transform(splitVec[0].begin(), splitVec[0].end(), splitVec[0].begin(), [](unsigned char c) -> unsigned char { return std::toupper(c); });

				// assemble the opcode
				u16 opcode = 0x0;
				for (u16 i = 0; i < instructionCount; i++)
				{
					if (instructions[i].mnemonic.compare(splitVec[0]) == 0)
					{
						instructionLines.push_back(splitVec); // store the instruction line

						PC += 2; // 2 bytes for instruction
						break;
					}
				}
			}
		}

		// close input file. we have everyhting we need
		infile.close();
		
		// for each instruction line we process the opcode and write to file
		for (auto instrVec : instructionLines)
		{
			// assemble the opcode
			u16 opcode = 0x0;
			for (u16 i = 0; i < instructionCount; i++)
			{
				if (instructions[i].mnemonic.compare(instrVec[0]) == 0)
				{
					opcode = instructions[i].opcode;
					opcode |= instructions[i].function(instrVec);

					// write big endian to rom file
					fputc(opcode >> 8, file);
					fputc(opcode & 0xFF, file);

					break;
				}
			}
		}

		// close out file
		fclose(file);

		return 0;
	}
}