#pragma once

#include <SFML/Graphics.hpp>
#include <argparse.h>


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

	int run_emulator(int argc, const char* argv[])
	{

		// do some arg parsing
		argparse::ArgumentParser parser("Argument parser for Chip8");
		parser.add_argument("-d", "--disassemble", "Disassemble the rom", false);
		parser.add_argument("-a", "--assemble", "Assemble the rom", false);
		parser.add_argument("-r", "--rom_file", "Rom file", true);

		parser.enable_help();
		auto err = parser.parse(argc, argv);
		if (err)
		{
			std::cout << err << std::endl;
			return -1;
		}

		if (parser.exists("help"))
		{
			parser.print_help();
			return 0;
		}
		else if (parser.exists("d"))
		{
			std::string rom_filename = parser.get<std::string>("r");

			return chip8::disassemble(rom_filename.c_str());
		}
		else if (parser.exists("a"))
		{
			std::string rom_filename = parser.get<std::string>("r");

			return chip8::assemble(rom_filename.c_str());
		}

		std::string rom_filename = parser.get<std::string>("r");

		// load and run the rom
		chip8::rom rom(rom_filename.c_str());

		// init sfml
		u8 pixelSize = 16;
		sf::RenderWindow window(sf::VideoMode(cpu::width * pixelSize, cpu::height * pixelSize), "Emulator");
		sf::RectangleShape whiteRect(sf::Vector2f(pixelSize, pixelSize));
		whiteRect.setFillColor(sf::Color::White);
	
		// fps counter and profiler
		sf::Font font;
		font.loadFromFile("courbd.ttf");

		sf::Text fps_text;
		fps_text.setFont(font);
		fps_text.setFillColor(sf::Color::White);
		fps_text.setPosition(10, 10);
		fps_text.setOutlineColor(sf::Color::Black);
		fps_text.setOutlineThickness(2);
		fps_text.setCharacterSize(18);

		// init scpu and load rom
		cpu::initialize();
		cpu::load_rom(rom.romdata, rom.romsize & 0xFFFF);

		auto cur_time = std::chrono::high_resolution_clock::now();
		auto last_time = cur_time;
		u32 fps = 0;

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

				// show profliler stats
				std::stringstream stream;
				stream << "FPS: " << fps << "\n";

				fps_text.setString(stream.str());
				window.draw(fps_text);

				// display on windows
				window.display();
			}

			// limit fps
			cur_time = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> delta = cur_time - last_time;
			std::chrono::duration<double, std::milli> min_frame_time(1000.0 / 360.0f);

			if (delta < min_frame_time)
			{
				std::this_thread::sleep_for(min_frame_time - delta);
			}

			// recalculate fps
			cur_time = std::chrono::high_resolution_clock::now();
			delta = cur_time - last_time;

			if (delta.count() != 0)
			{
				fps = (u32)(1000 / delta.count());
			}

			last_time = cur_time;
		}

		return 0;
	}
}
