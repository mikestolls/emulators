#pragma once

#include "defines.h"

namespace chip8
{
	unsigned char chip8_fontset[80] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, //0
		0x20, 0x60, 0x20, 0x20, 0x70, //1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
		0x90, 0x90, 0xF0, 0x10, 0x10, //4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
		0xF0, 0x10, 0x20, 0x40, 0x40, //7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
		0xF0, 0x90, 0xF0, 0x90, 0x90, //A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
		0xF0, 0x80, 0x80, 0x80, 0xF0, //C
		0xE0, 0x90, 0x90, 0x90, 0xE0, //D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
		0xF0, 0x80, 0xF0, 0x80, 0x80  //F
	};

	class cpu
	{
	public:
		u16 opcode; // current opcode
		u8 memory[4096]; // 4k of memory: 0x000 - 0x1FF - chip8 interpreter, 0x050 - 0x0A - used for font set, 0x200 - 0xFFF - program rom and working RAM

		u8 V[16]; // 15 8-bit registers + 1 carry flag
		u16 I; // index register
		u16 PC; // program counter;

		u8 width = 64;
		u8 height = 32;

		u8 gfx[64 * 32]; // graphics for screen display

		u8 delaytimer;
		u8 soundtimer;

		u16 stack[16]; // stack used for sub routine implementation
		u16 sp;

		u8 keystate[16]; // used for keypad

		bool drawFlag;

		int initialize()
		{
			// initialize the registers and memory
			opcode = 0;
			memset(memory, 0x0, sizeof(memory));

			// add in the font set at begining of memory
			memcpy(memory, chip8_fontset, sizeof(chip8_fontset));

			reset();

			return 0;
		}

		int reset()
		{
			memset(V, 0x0, sizeof(V));
			I = 0;
			PC = 0;

			memset(gfx, 0x0, sizeof(gfx));

			delaytimer = 0;
			soundtimer = 0;

			memset(stack, 0x0, sizeof(stack));
			sp = 0;

			memset(keystate, 0x0, sizeof(keystate));

			srand((unsigned int)time(0));

			drawFlag = false;

			return 0;
		}

		int load_rom(u8* romdata, u16 romsize)
		{
			// copy rom memory into chip memory. program starts at location 0x200
			memcpy(&memory[0x200], romdata, romsize);
			PC = 0x200;

			return 0;
		}

		int update_cycle()
		{
			// fetch opcode
			u16 opcode = (memory[PC++] << 8);
			opcode |= memory[PC++];

			drawFlag = false;
			bool error = false;

			// decode opcode
			switch (opcode & 0xF000)
			{
			case 0x0000:
			{
				switch (opcode & 0x000F)
				{
				case 0x0000: // 0x00E0: clears the screen
					memset(gfx, 0x0, sizeof(gfx));
					drawFlag = true;
					break;
				case 0x000E: // 0x00EE: returns from subroutine
					PC = stack[--sp];
					PC += 2;
					break;
				default:
					error = true;
				}
				break;
			}
			case 0x1000: // 1NNN: set program counter to NNN
				PC = opcode & 0x0FFF;
				break;
			case 0x2000: // 2NNN: call subroutine at addr NNN
				stack[sp++] = PC - 2;
				PC = (opcode & 0x0FFF);
				break;
			case 0x3000: // 3XKK: skip next instruction if X register == KK
				if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
				{
					PC += 2;
				}
				break;
			case 0x4000: // 4XKK: skip next instruction if X register != KK
				if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
				{
					PC += 2;
				}
				break;
			case 0x5000: // 5XY0: skip next instruction if X register = Y register
				if (V[(opcode & 0x0F00) >> 8] == (V[(opcode & 0x00F0) >> 4]))
				{
					PC += 2;
				}
				break;
			case 0x6000: // 6XKK: put KK into X register
				V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
				break;
			case 0x7000: // 7XKK: adds KK to the value od register X
				V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
				break;
			case 0x8000:
			{
				switch (opcode & 0x000F)
				{
				case 0x0000: // 8XY0: stores value of register Y into register X
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
					break;
				case 0x0001: // 8XY1: OR X and Y and store in X
					V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
					break;
				case 0x0002: // 8XY2: AND X and Y and store in X
					V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
					break;
				case 0x0003: // 8XY3: XOR X and Y and store in X
					V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
					break;
				case 0x0004: // 8XY4: ADD X and Y and store in X. set carry flag if > 255
				{
					u16 value = V[(opcode & 0x0F00) >> 8] + V[(opcode & 0x00F0) >> 4];
					V[0xF] = (value > 255 ? 1 : 0);
					V[(opcode & 0x0F00) >> 8] = value & 0xFF; // store lowest 8 bits
					break;
				}
				case 0x0005: // 8XY5: SUB Y from X and store in X. set carry flag to 0 if borrow, 1 if not
					V[0xF] = (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8] ? 0 : 1);
					V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
					break;
				case 0x0006: // 8XY6: if lowest bit of X is 1 set carry flag to 1, then X is divided by 2
					V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x01;
					V[(opcode & 0x0F00) >> 8] >>= 1; // divide by 2
					break;
				case 0x0007: // 8XY7: if X > Y there is borrow, set carry flag to 0, 1 if not. then X = Y - X
					V[0xF] = (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4] ? 0 : 1);
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
					break;
				case 0x000E: // 8XYE: if highest bit of Y is 1 set carry to 1, then multiply X by 2
					V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
					V[(opcode & 0x0F00) >> 8] <<= 1;
					break;
				default:
					error = true;
				}
				break;
			}
			case 0x9000: // 9XY0: skip next instruction if X != Y:
				if (V[(opcode & 0x0F00) >> 8] != (V[(opcode & 0x00F0) >> 4]))
				{
					PC += 2;
				}
				break;
			case 0xA000: // ANNN: put NNN to register I
				I = opcode & 0x0FFF;
				break;
			case 0xB000: // BNNN: set program counter to NNN + V[0]
				PC = V[0] + (opcode & 0x0FFF);
				break;
			case 0xC000: // CXKK: set X to rand(0 - 255) & NN
				V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF); // rand byte AND KK
				break;
			case 0xD000: // DXYN: Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
			{
				u8 xStart = V[(opcode & 0x0F00) >> 8];
				u8 yStart = V[(opcode & 0x00F0) >> 4];
				u8 rows = (opcode & 0x000F);
				u8* memPtr = &memory[I];

				V[0xF] = 0;
				for (u8 y = 0; y < rows; y++)
				{
					for (u8 x = 0; x < 8; x++)
					{
						u8 newValue = *memPtr & (0x80 >> x);

						if (newValue == 0x0) // continue
						{
							continue;
						}

						u8 xPos = (xStart + x) & 0x3F;
						u8 yPos = (yStart + y) & 0x1F;
						
						u16 pixel = yPos * 64 + xPos;

						if (pixel >= (width * height))
						{
							printf("Error: pixel index out of range\n");
						}
						if (gfx[pixel] != 0) // set flag because pixel being cleared
						{
							V[0xF] = 1;
						}

						// xor the pixel
						gfx[pixel] ^= 1;
					}

					// step sprite memory
					memPtr++;
				}

				drawFlag = true;

				break;
			}
			case 0xE000:
			{
				switch (opcode & 0x00FF)
				{
				case 0x009E: // EX9E: skip next instruction if key[X] is pressed
					if (keystate[V[(opcode & 0x0F00) >> 8]] != 0)
					{
						PC += 2;
					}
					break;
				case 0x00A1: // EXA1: skip next instruction if key[X] is not pressed
					if (keystate[V[(opcode & 0x0F00) >> 8]] == 0)
					{
						PC += 2;
					}
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
					V[(opcode & 0x0F00) >> 8] = delaytimer;
					break;
				case 0x000A: // FX0A: block instructions until keypress
				{
					bool pressed = false;

					for (u8 i = 0; i < 0xF; ++i)
					{
						if (keystate[i] != 0)
						{
							V[(opcode & 0x0F00) >> 8] = i;
							pressed = true;
							break;
						}
					}

					if (!pressed)
					{
						PC -= 2; // return PC to block operations
						return 0;
					}

					break;
				}
				case 0x0015: // FX15: set delay timer to X
					delaytimer = V[(opcode & 0x0F00) >> 8];
					break;
				case 0x0018: // FX18: set sound timer to X
					soundtimer = V[(opcode & 0x0F00) >> 8];
					break;
				case 0x001E: // FX1E: add X to I
					I += V[(opcode & 0x0F00) >> 8];
					V[0xF] = (I > 0xFFF ? 1 : 0); // undocumented to store the carry flag
					break;
				case 0x0029: // FX29: set I to the memory addr of font character in X
					I = V[(opcode & 0x0F00) >> 8] * 0x5;
					break;
				case 0x0033: // FX33: store binary decimals into I, I+1, I+2
				{
					u8 value = V[(opcode & 0x0F00) >> 8];
					memory[I] = value / 100;
					memory[I + 1] = (value - (memory[I] * 100)) / 10;
					memory[I + 2] = value % 10;
					break;
				}
				case 0x0055: // FX55: store values of 0 to X in memory starting at I
				{
					u8 count = ((opcode & 0xF00) >> 8);
					u8* memPtr = &memory[I];
					for (u8 i = 0; i <= count; ++i)
					{
						*memPtr++ = V[i];
					}

					//I += count + 1; // I is set to I + X + 1 after
					break;
				}
				case 0x0065: // FX65: store memory starting at I into 0 to X
				{
					u8 count = ((opcode & 0xF00) >> 8);
					u8* memPtr = &memory[I];
					for (u8 i = 0; i <= count; ++i)
					{
						V[i] = *memPtr++;
					}

					//I += count + 1; // I is set to I + X + 1 after
					break;
				}
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
				printf("Error: Unkown opcode: 0x%X\n", opcode);
			}

			// update timers
			if (delaytimer > 0)
			{
				--delaytimer;
			}

			if (soundtimer > 0)
			{
				if (soundtimer == 1)
				{
					printf("BEEP!\n\a");
				}
				--soundtimer;
			}

			return 0;
		}

		int set_keys(u8 key, bool pressed)
		{
			keystate[key] = (pressed ? 1 : 0);

			return 0;
		}
	};
}