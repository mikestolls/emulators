#pragma once

#include <SFML/Graphics.hpp>

#include "cpu.h"
#include "gpu.h"
#include "rom.h"
#include "debugger.h"
#include "disassembler.h"

namespace gameboy
{
	const u8 pixelSize = 8;
	
	int run_emulator(int argc, char** argv)
	{
		std::string filename = "";
		bool disassemble = false;

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
				rom rom(filename.c_str());
				memory_module::initialize(&rom);

				// export disassembler to file and close
				std::string outfilename = rom.filename.substr(0, rom.filename.rfind("."));
				outfilename.append(".gbasm");

				disassembler::disassemble_to_file(outfilename.c_str());

				return 0;
			}
			else if (strcmp("-a", argv[1]) == 0)
			{
				return -1;
			}
		}

		// load and run the rom
		rom rom(filename.c_str());

		// init sfml
		sf::RenderWindow window(sf::VideoMode(gpu::width * pixelSize, gpu::height * pixelSize), "Emulator");
		sf::Texture framebuffer_texture;
		sf::Sprite framebuffer_sprite;
		framebuffer_texture.create(gpu::width, gpu::height);
		framebuffer_sprite.setTexture(framebuffer_texture);
		framebuffer_sprite.setScale(pixelSize, pixelSize);

		debugger debugger;
		debugger.initialize(window.getSize().x, window.getSize().y);
		bool show_debugger = false;

		// init cpu and load rom
		memory_module::initialize(&rom);
		cpu::initialize();
		gpu::initialize();

		std::chrono::milliseconds cur_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		std::chrono::milliseconds last_time = cur_time;

		const u32 cycles_per_frame = cpu::cycles_per_sec / cpu::fps;
		u32 cycle_count = 0;

		while (window.isOpen())
		{
			// poll for window events
			sf::Event event;
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
				{
					debugger.destroy();
					window.close();
				}
				else if (event.type == sf::Event::KeyPressed)
				{
					if (event.key.code == sf::Keyboard::Space)
					{
						cpu::reset();
						gpu::reset();
						cycle_count = 0;
					}
					else if (event.key.code == sf::Keyboard::F1)
					{
						show_debugger = !show_debugger;
					}
					else
					{
						if (show_debugger)
						{
							debugger.on_keypressed(event.key.code);
						}
					}
				}
			}

			while (cycle_count < cycles_per_frame)
			{
				// update the cpu emulation
				u8 cpu_cycles = cpu::execute_opcode();
				cpu::update_timer(cpu_cycles);
				cpu_cycles += cpu::check_interrupts();
				cycle_count += cpu_cycles;
				gpu::update(cpu_cycles);
				
				if (cpu::paused)
				{
					break;
				}
			}

			if (!cpu::paused)
			{
				// once we have passed cycles per frame reset cycle count
				cycle_count -= cycles_per_frame;
			}

			window.clear();

			// update the framebuffer
			framebuffer_texture.update(gpu::framebuffer, gpu::width, gpu::height, 0, 0);
			
			// draw framebuffer
			window.draw(framebuffer_sprite);

			// draw debugger if shown
			if (show_debugger)
			{
				debugger.update();

				window.draw(debugger.window_sprite);
			}

			// display on windows
			window.display();

			// limit fps
			cur_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
			std::chrono::milliseconds delta = cur_time - last_time;

			std::chrono::duration<double, std::milli> min_frame_time(1000.0 / (float)cpu::fps);
			if (delta < min_frame_time)
			{
				std::this_thread::sleep_for(min_frame_time - delta);
			}

			last_time = cur_time;
		}

		// cleanup
		debugger.destroy();
		window.close();

		return 0;
	}
}