#pragma once

#include <SFML/Graphics.hpp>

#include "cpu.h"
#include "input.h"
#include "gpu.h"
#include "apu.h"
#include "rom.h"
#include "boot_rom.h"
#include "debugger.h"
#include "disassembler.h"

#include <map>

#define USE_BOOT_ROM

namespace gameboy
{
	const u8 pixelSize = 8;

	struct input_binding
	{
		u8 joypad_map;
		bool is_directional;
	};

	std::map<sf::Keyboard::Key, input_binding> input_map;
	
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
				memory_module::initialize(nullptr, &rom);

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

		// load the boot rom file
		boot_rom boot("gameboy\\boot.gb");

		// init sfml
		sf::RenderWindow window(sf::VideoMode(gpu::width * pixelSize, gpu::height * pixelSize), "Emulator");
		sf::Texture framebuffer_texture;
		sf::Sprite framebuffer_sprite;
		framebuffer_texture.create(gpu::width, gpu::height);
		framebuffer_sprite.setTexture(framebuffer_texture);
		framebuffer_sprite.setScale(pixelSize, pixelSize);

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

		debugger debugger;
		debugger.initialize(window.getSize().x, window.getSize().y);
		bool show_debugger = false;
		u32 fps = 0;

		// init input map
		input_map[sf::Keyboard::Left] = { DIRECTION_LEFT, true };
		input_map[sf::Keyboard::Right] = { DIRECTION_RIGHT, true };
		input_map[sf::Keyboard::Up] = { DIRECTION_UP, true };
		input_map[sf::Keyboard::Down] = { DIRECTION_DOWN, true };
		input_map[sf::Keyboard::A] = { BUTTON_A, false };
		input_map[sf::Keyboard::S] = { BUTTON_B, false };
		input_map[sf::Keyboard::Return] = { BUTTON_START, false };
		input_map[sf::Keyboard::RShift] = { BUTTON_SELECT, false };
		
		// init cpu and load rom
#ifdef USE_BOOT_ROM
		memory_module::initialize(&boot, &rom);
#else
		memory_module::initialize(nullptr, &rom);
#endif

		cpu::initialize();
		gpu::initialize();
		apu::initialize();
		
		auto cur_time = std::chrono::high_resolution_clock::now();
		auto last_time = cur_time;

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
					if (event.key.code == sf::Keyboard::F1)
					{
						show_debugger = !show_debugger;
					}

					if (show_debugger)
					{
						if (event.key.code == sf::Keyboard::Space)
						{
							cpu::reset();
							gpu::reset();
							apu::reset();
							cycle_count = 0;
						}
						else
						{
							debugger.on_keypressed(event.key.code);
						}
					}
					else
					{
						// check for joypad input
						auto itr = input_map.find(event.key.code);

						if (itr != input_map.end())
						{
							// handle joypad input
							set_button_pressed(itr->second.joypad_map, itr->second.is_directional);
						}
					}
				}
				else if (event.type == sf::Event::KeyReleased)
				{
					if (show_debugger)
					{

					}
					else
					{
						// check for joypad input
						auto itr = input_map.find(event.key.code);

						if (itr != input_map.end())
						{
							// handle joypad input
							set_button_released(itr->second.joypad_map, itr->second.is_directional);
						}
					}
				}
			}
			
			while (cycle_count < cycles_per_frame)
			{
				// update the cpu emulation
				u8 cpu_cycles = cpu::check_interrupts();
				cpu_cycles += cpu::execute_opcode();
				cycle_count += cpu_cycles;
				
				if (cpu::paused || !cpu::running)
				{
					break;
				}
			}

			if (!cpu::paused && cpu::running)
			{
				// once we have passed cycles per frame reset cycle count
				cycle_count -= cycles_per_frame;
			}

			window.clear();

			// update the framebuffer
			if (gpu::vblank_occurred)
			{
				framebuffer_texture.update(gpu::framebuffer, gpu::width, gpu::height, 0, 0);
				gpu::vblank_occurred = false;
			}
			
			// draw framebuffer
			window.draw(framebuffer_sprite);

			// draw debugger if shown
			if (show_debugger)
			{
				debugger.update();

				window.draw(debugger.window_sprite);
			}

			// show profliler stats
			std::stringstream stream;
			stream << "FPS: " << fps << "\n";

			fps_text.setString(stream.str());
			window.draw(fps_text);

			// display on windows
			window.display();
			
			// limit fps
			cur_time = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> delta = cur_time - last_time;
			std::chrono::duration<double, std::milli> min_frame_time(1000.0 / (float)cpu::fps);

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

		// cleanup
		debugger.destroy();
		window.close();

		return 0;
	}
}