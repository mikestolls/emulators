#pragma once

#include <SFML/Graphics.hpp>

#include "cpu.h"
#include "gpu.h"
#include "rom.h"

namespace gameboy
{
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
				return -1;
			}
			else if (strcmp("-a", argv[1]) == 0)
			{
				return -1;
			}
		}

		// load and run the rom
		gameboy::rom rom(filename.c_str());
		gameboy::memory_module memory(&rom);

		// init sfml
		u8 pixelSize = 4;
		u8 width = 160;
		u8 height = 144;
		sf::RenderWindow window(sf::VideoMode(width * pixelSize, height * pixelSize), "Emulator");
		sf::RectangleShape whiteRect(sf::Vector2f(pixelSize, pixelSize));
		whiteRect.setFillColor(sf::Color::White);

		// init cpu and load rom
		cpu::initialize(&memory);
		gpu::initialize(&memory);

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

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			{
				//cpu.reset();
			}

			// update the cpu emulation
			cpu::execute_opcode();

			//if (cpu.drawFlag)
			{
				// clear window
				window.clear();

				// display on windows
				window.display();
			}

			curTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
			std::chrono::milliseconds delta = curTime - lastTime;

			std::chrono::duration<double, std::milli> minFrameTime(1000.0 / 60.0);
			if (delta < minFrameTime)
			{
				std::this_thread::sleep_for(minFrameTime - delta);
			}

			lastTime = curTime;
		}

		return 0;
	}
}