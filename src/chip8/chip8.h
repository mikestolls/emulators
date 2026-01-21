#pragma once

#include <SFML/Graphics.hpp>
#include <argparse.h>


#include "cpu.h"
#include "rom.h"
#include "disassembler.h"
#include "assembler.h"

namespace chip8
{
	static const std::pair<sf::Keyboard::Key, u8> keyboard[] = {
		{sf::Keyboard::Key::Num0, 0x0},
		{sf::Keyboard::Key::Num2, 0x1},
		{sf::Keyboard::Key::Num3, 0x2},
		{sf::Keyboard::Key::Num4, 0x3},
		{sf::Keyboard::Key::Q, 0x4},
		{sf::Keyboard::Key::W, 0x5},
		{sf::Keyboard::Key::E, 0x6},
		{sf::Keyboard::Key::R, 0x7},
		{sf::Keyboard::Key::A, 0x8},
		{sf::Keyboard::Key::S, 0x9},
		{sf::Keyboard::Key::D, 0xA},
		{sf::Keyboard::Key::F, 0xB},
		{sf::Keyboard::Key::Z, 0xC},
		{sf::Keyboard::Key::X, 0xD},
		{sf::Keyboard::Key::C, 0xE},
		{sf::Keyboard::Key::V, 0xF},
	};

	rom loaded_rom;

	// init sfml
	u8 pixelSize = 16;
	sf::RectangleShape whiteRect(sf::Vector2f(pixelSize, pixelSize));
	/*
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
		sf::RenderWindow window(sf::VideoMode(sf::Vector2u(cpu::width * pixelSize, cpu::height * pixelSize)), "Emulator");
		sf::RectangleShape whiteRect(sf::Vector2f(pixelSize, pixelSize));
		whiteRect.setFillColor(sf::Color::White);
	
		// fps counter and profiler
		sf::Font font;
		bool success = font.openFromFile("courbd.ttf");

		sf::Text fps_text(font);
		fps_text.setFillColor(sf::Color::White);
		fps_text.setPosition(sf::Vector2f(10, 10));
		fps_text.setOutlineColor(sf::Color::Black);
		fps_text.setOutlineThickness(2);
		fps_text.setCharacterSize(18);

		// init cpu and load rom
		cpu::initialize();
		cpu::load_rom(rom.romdata, rom.romsize & 0xFFFF);

		auto cur_time = std::chrono::high_resolution_clock::now();
		auto last_time = cur_time;
		u32 fps = 0;

		while (window.isOpen())
		{
			// poll for window events
			while (std::optional event = window.pollEvent())
			{
				if (event->is<sf::Event::Closed>())
				{
					window.close();
				}
				else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
				{
					// check if key is pressed and set the corresponding key in the cpu
					for (const auto& key : keyboard)
					{
						if (keyPressed->code == key.first)
						{
							cpu::set_keys(key.second, true);
							break;
						}
					}

					if (keyPressed->code == sf::Keyboard::Key::Space)
					{
						cpu::reset();
					}
				}
				else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
				{
					// check if key is released and unset the corresponding key in the cpu
					for (const auto& key : keyboard)
					{
						if (keyReleased->code == key.first)
						{
							cpu::set_keys(key.second, false);
							break;
						}
					}
				}
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
							whiteRect.setPosition(sf::Vector2f((float)(x * pixelSize), (float)(y * pixelSize)));
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
	*/
	int init_emulator(std::string rom_filename)
	{
		// init cpu and load rom
		cpu::initialize();
		loaded_rom.load(rom_filename);
		cpu::load_rom(loaded_rom.romdata, loaded_rom.romsize & 0xFFFF);

		whiteRect.setFillColor(sf::Color::White);

		return 0;
	}

	int process_event(const sf::Event* event)
	{
		if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
		{
			// check if key is pressed and set the corresponding key in the cpu
			for (const auto& key : keyboard)
			{
				if (keyPressed->code == key.first)
				{
					cpu::set_keys(key.second, true);
					break;
				}
			}

			if (keyPressed->code == sf::Keyboard::Key::Space)
			{
				cpu::reset();
			}
		}
		else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
		{
			// check if key is released and unset the corresponding key in the cpu
			for (const auto& key : keyboard)
			{
				if (keyReleased->code == key.first)
				{
					cpu::set_keys(key.second, false);
					break;
				}
			}
		}

		return 0;
	}

	bool update(sf::RenderWindow& window)
	{
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
						whiteRect.setPosition(sf::Vector2f((float)(x * pixelSize), (float)(y * pixelSize)));
						window.draw(whiteRect);
					}

				}
			}

			return true;
		}

		return false;
	}
}
