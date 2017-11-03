#pragma once

#include <SFML/Graphics.hpp>

#include "cpu.h"
#include "rom.h"
#include "disassembler.h"
#include "assembler.h"

namespace chip8
{
	static const u8 keyboard[] = { sf::Keyboard::Num1, 0x0,
									sf::Keyboard::Num2, 0x1,
									sf::Keyboard::Num3, 0x2,
									sf::Keyboard::Num4, 0x3,
									sf::Keyboard::Q, 0x4,
									sf::Keyboard::W, 0x5,
									sf::Keyboard::E, 0x6,
									sf::Keyboard::R, 0x7,
									sf::Keyboard::A, 0x8,
									sf::Keyboard::S, 0x9,
									sf::Keyboard::D, 0xA,
									sf::Keyboard::F, 0xB,
									sf::Keyboard::Z, 0xC,
									sf::Keyboard::X, 0xD,
									sf::Keyboard::C, 0xE,
									sf::Keyboard::V, 0xF,
	};

	int run_emulator(int argc, char** argv)
	{
		std::string filename = "";

		if (argc < 2)
		{
			printf("Error - arguments: [options] rom_filename\noptions:\n\t-d\tdisassemble rom\n\t-a\tassemle rom\n");
			return -1;
		}
		else if (argc < 3)
		{
			filename = argv[1];
		}
		else if (argc < 4)
		{
			filename = argv[2];
			if (strcmp("-d", argv[1]) == 0)
			{
				return chip8::disassemble(filename.c_str());
			}
			else if (strcmp("-a", argv[1]) == 0)
			{
				return chip8::assemble(filename.c_str());
			}
		}
		
		// load and run the rom
		chip8::rom rom(filename.c_str());

		// init sfml
		u8 pixelSize = 16;
		sf::RenderWindow window(sf::VideoMode(cpu::width * pixelSize, cpu::height * pixelSize), "Emulator");
		sf::RectangleShape whiteRect(sf::Vector2f(pixelSize, pixelSize));
		whiteRect.setFillColor(sf::Color::White);
	
		// init scpu and load rom
		cpu::initialize();
		cpu::load_rom(rom.romdata, rom.romsize & 0xFFFF);

		std::chrono::milliseconds curTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		std::chrono::milliseconds lastTime = curTime;

		while (window.isOpen())
		{
			// poll for window events
			sf::Event event;
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();
			}

			for (u8 i = 0; i < sizeof(chip8::keyboard); i+=2)
			{
				if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(chip8::keyboard[i])))
				{
					cpu::set_keys(chip8::keyboard[i + 1], true);
				}
				else
				{
					cpu::set_keys(chip8::keyboard[i + 1], false);
				}
			}
		
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			{
				cpu::reset();
			}

			// update the cpu emulation
			cpu::update_cycle();

			if (cpu::drawFlag)
			{
				// clear window
				window.clear();

				// draw screen
				u16 pixel = 0;
				for (u8 y = 0; y < cpu::height; y++)
				{
					for (u8 x = 0; x < cpu::width; x++)
					{
						if (cpu::gfx[pixel++] != 0)
						{
							whiteRect.setPosition((float)(x * pixelSize), (float)(y * pixelSize));
							window.draw(whiteRect);
						}
					}
				}

				// display on windows
				window.display();
			}

			curTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
			std::chrono::milliseconds delta = curTime - lastTime;

			std::chrono::duration<double, std::milli> minFrameTime(1000.0 / 360.0);
			if (delta < minFrameTime)
			{
				std::this_thread::sleep_for(minFrameTime - delta);
			}

			lastTime = curTime;
		}

		return 0;
	}
}
