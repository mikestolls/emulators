#pragma once

#include <SFML/Graphics.hpp>

#include "cpu.h"
#include "gpu.h"
#include "rom.h"
#include "debugger.h"

namespace gameboy
{
	const u8 pixelSize = 8;
	
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
		sf::RenderWindow window(sf::VideoMode(gpu::width * pixelSize, gpu::height * pixelSize), "Emulator");
		sf::Texture framebuffer_texture;
		sf::Sprite framebuffer_sprite;
		framebuffer_texture.create(gpu::width, gpu::height);
		framebuffer_sprite.setTexture(framebuffer_texture);
		framebuffer_sprite.setScale(pixelSize, pixelSize);

		debugger debug_window;
		debug_window.initialize(&memory);

		// init cpu and load rom
		cpu::initialize(&memory);
		gpu::initialize(&memory);

		std::chrono::milliseconds curTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		std::chrono::milliseconds lastTime = curTime;

		const u32 cycles_per_sec = 4194304;
		const u32 fps = 60;
		const u32 cycles_per_frame = cycles_per_sec / fps;
		u32 cycle_count = 0;

		while (window.isOpen())
		{
			// poll for window events
			sf::Event event;
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
				{
					window.close();
					debug_window.close();
				}
				else if (event.type == sf::Event::KeyPressed)
				{
					if (event.key.code == sf::Keyboard::Space)
					{
						cpu::reset();
						gpu::reset();
						cycle_count = 0;
					}
					else if (event.key.code == sf::Keyboard::F5)
					{
						cpu::debugging = !cpu::debugging;
						cpu::debugging_step = false;
						printf("Debugging: %s\n", (cpu::debugging ? "True" : "False"));
					}
					else if (event.key.code == sf::Keyboard::F10)
					{
						cpu::debugging_step = true;
						printf("Debugging Step\n");
					}
				}
			}

			// this gets a little messy because of debug stepping
			if (cpu::debugging && cpu::debugging_step)
			{
				u8 cpu_cycles = cpu::execute_opcode();
				cycle_count += cpu_cycles;
				cpu::check_interrupts();
				gpu::update(cpu_cycles);

				cpu::debugging_step = false;
			}
			else if (!cpu::debugging)
			{
				while (cycle_count < cycles_per_frame)
				{
					// update the cpu emulation
					u8 cpu_cycles = cpu::execute_opcode();
					cycle_count += cpu_cycles;
					cpu::check_interrupts();
					gpu::update(cpu_cycles);

					if (cpu::debugging)
					{
						break;
					}
				}

				if (!cpu::debugging)
				{
					// once we have passed cycles per frame reset cycle count
					cycle_count -= cycles_per_frame;
				}
			}

			debug_window.update();

			// update the framebuffer
			framebuffer_texture.update(gpu::framebuffer, gpu::width, gpu::height, 0, 0);
			
			// draw framebuffer
			window.draw(framebuffer_sprite);

			// display on windows
			window.display();

			// limit fps
			curTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
			std::chrono::milliseconds delta = curTime - lastTime;

			std::chrono::duration<double, std::milli> minFrameTime(1000.0 / (float)fps);
			if (delta < minFrameTime)
			{
				std::this_thread::sleep_for(minFrameTime - delta);
			}

			lastTime = curTime;
		}

		return 0;
	}
}