#pragma once

#include <SFML/Graphics.hpp>
#include <argparse.h>

#include "defines.h"

#include "cpu.h"
#include "input.h"
#include "gpu.h"
#include "rom.h"
#include "boot_rom.h"
#include "debugger.h"
#include "disassembler.h"

//#define USE_BOOT_ROM

namespace gameboy
{
	const u8 pixelSize = 8;

	struct input_binding
	{
		u8 joypad_map;
		bool is_directional;
	};

	struct unit_test
	{
	public:
		std::string filename;
		u16 abort_pc;
		std::string checksum;
	};

	std::map<sf::Keyboard::Key, input_binding> input_map;
	rom loaded_rom;
	sf::Texture framebuffer_texture;

	const u32 cycles_per_frame = cpu::cycles_per_sec / cpu::fps;
	u32 cycle_count = 0;

	debugger gameboy_debugger;
	bool show_debugger = false;
	
	/*std::list<unit_test> unit_test_list;
	
	int run_emulator_rom(std::string filename, bool show_window = true, s32 abort_pc = -1, std::string vram_checksum = "")
	{
		// load and run the rom
		rom rom(filename);

		// load the boot rom file
		bool is_window_enabled = false;
		sf::RenderWindow window;
		sf::Texture framebuffer_texture;
		sf::Sprite framebuffer_sprite = sf::Sprite(framebuffer_texture);
		sf::Font font;
		sf::Text fps_text = sf::Text(font);
		debugger debugger;

		if (show_window)
		{
			// init sfml
			window.create(sf::VideoMode(sf::Vector2u(gpu::width * pixelSize, gpu::height * pixelSize)), "Emulator");
			bool success = framebuffer_texture.resize(sf::Vector2u(gpu::width, gpu::height));
			framebuffer_sprite.setScale(sf::Vector2f(pixelSize, pixelSize));
			framebuffer_sprite.setTexture(framebuffer_texture, true);

			// fps counter and profiler
			success = font.openFromFile("courbd.ttf");

			fps_text.setFont(font);
			fps_text.setFillColor(sf::Color::White);
			fps_text.setPosition(sf::Vector2f(10, 10));
			fps_text.setOutlineColor(sf::Color::Black);
			fps_text.setOutlineThickness(2);
			fps_text.setCharacterSize(18);

			debugger.initialize(window.getSize().x, window.getSize().y);

			is_window_enabled = true;
		}

		bool show_debugger = false;
		u32 fps = 0;

		// init input map
		input_map[sf::Keyboard::Key::Left] = { DIRECTION_LEFT, true };
		input_map[sf::Keyboard::Key::Right] = { DIRECTION_RIGHT, true };
		input_map[sf::Keyboard::Key::Up] = { DIRECTION_UP, true };
		input_map[sf::Keyboard::Key::Down] = { DIRECTION_DOWN, true };
		input_map[sf::Keyboard::Key::A] = { BUTTON_A, false };
		input_map[sf::Keyboard::Key::B] = { BUTTON_B, false };
		input_map[sf::Keyboard::Key::Enter] = { BUTTON_START, false };
		input_map[sf::Keyboard::Key::RShift] = { BUTTON_SELECT, false };
		
		// init cpu and load rom
		warning("fix boot rom loading")

#ifdef USE_BOOT_ROM
		boot_rom boot("gameboy/boot.gb");
		memory_module::initialize(&boot, &rom);
#else
		memory_module::initialize(nullptr, &rom);
#endif

		cpu::initialize();
		gpu::initialize();
		
		auto cur_time = std::chrono::high_resolution_clock::now();
		auto last_time = cur_time;

		const u32 cycles_per_frame = cpu::cycles_per_sec / cpu::fps;
		u32 cycle_count = 0;
		bool running = true;

		while (running)
		{
			// poll for window events
			if (is_window_enabled)
			{
				// poll for window events
				while (std::optional event = window.pollEvent())
				{
					if (event->is<sf::Event::Closed>())
					{
						debugger.destroy();
						window.close();
					}
					else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
					{
						if (keyPressed->code == sf::Keyboard::Key::F1)
						{
							show_debugger = !show_debugger;
						}

						if (show_debugger)
						{
							if (keyPressed->code == sf::Keyboard::Key::Space)
							{
								cpu::reset();
								gpu::reset();
								cycle_count = 0;
							}
							else if (keyPressed->code == sf::Keyboard::Key::F2)
							{
								u8* ptr = memory_module::get_memory(0x9800, true);
								u8* buffer = new u8[0x401];
								memset(buffer, 0x0, 0x401);
								memcpy(buffer, ptr, 0x400);
								//std::string checksum = buffer;

								printf("Checksum: %s", buffer);
							}
							else
							{
								debugger.on_keypressed(keyPressed->code);
							}
						}
						else
						{
							// check for joypad input
							auto itr = input_map.find(keyPressed->code);

							if (itr != input_map.end())
							{
								// handle joypad input
								set_button_pressed(itr->second.joypad_map, itr->second.is_directional);
							}
						}
					}
					else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
					{
						if (show_debugger)
						{

						}
						else
						{
							// check for joypad input
							auto itr = input_map.find(keyReleased->code);

							if (itr != input_map.end())
							{
								// handle joypad input
								set_button_released(itr->second.joypad_map, itr->second.is_directional);
							}
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
				
				// used for unit testing
				if (cpu::R.pc == abort_pc)
				{
					// used to get vram of test passed
					//u8* test = new u8[0xF0];
					//memset(test, 0x0, 0xF0);
					//u8* vram_test = memory_module::get_memory(0x9800, true);
					//memcpy(test, vram_test, 0xEF);

					running = false;

					// need to check the checksum
					u8* vram = memory_module::get_memory(0x9800, true);
					if (memcmp(vram_checksum.c_str(), vram, vram_checksum.length()) == 0)
					{
						return 0;
					}
					else
					{
						return 2;
					}
				}

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

			if (is_window_enabled)
			{
				window.clear();
			}

			// update the framebuffer
			if (gpu::vblank_occurred)
			{
				if (is_window_enabled)
				{
					framebuffer_texture.update(gpu::framebuffer, sf::Vector2u(gpu::width, gpu::height), sf::Vector2u(0, 0));
				}

				gpu::vblank_occurred = false;
			}
			
			if (is_window_enabled)
			{
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
				running = window.isOpen();

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
		}

		if (is_window_enabled)
		{
			// cleanup
			debugger.destroy();
			window.close();
		}

		return 0;
	}

	int run_emulator(int argc, const char* argv[])
	{
		// do some arg parsing
		argparse::ArgumentParser parser("Argument parser for Gameboy");
		parser.add_argument("-d", "--disassemble", "Disassemble the rom", false);
		parser.add_argument("-a", "--assemble", "Assemble the rom", false);
		parser.add_argument("-u", "--unit_test", "Unit test the rom", false);
		parser.add_argument("-p", "--unit_test_abortpc", "Unit test abort pc (required with unit_test)", false);
		parser.add_argument("-c", "--unit_test_check", "Unit test check (required with unit_test)", false);
		parser.add_argument("-r", "--rom_file", "Rom file", true);

		parser.enable_help();
		auto err = parser.parse(argc, argv);
		if (err) 
		{
			std::cout << err << std::endl;
			return 1;
		}

		if (parser.exists("help")) 
		{
			parser.print_help();
			return 0;
		}
		else if (parser.exists("d"))
		{
			std::string rom_filename = parser.get<std::string>("r");
			rom rom(rom_filename);
			memory_module::initialize(nullptr, &rom);

			// export disassembler to file and close
			std::string outfilename = rom.filename.substr(0, rom.filename.rfind("."));
			outfilename.append(".gbasm");

			disassembler::disassemble_to_file(outfilename.c_str());

			return 0;
		}
		else if (parser.exists("a"))
		{
			// not supported
			return 1;
		}
		else if (parser.exists("u"))
		{
			if (parser.exists("p") == false || parser.exists("c") == false)
			{
				parser.print_help();
				return 1;
			}

			std::string rom_filename = parser.get<std::string>("r");
			std::string checksum = parser.get<std::string>("c");
			s32 abort_pc = std::stoi(parser.get<std::string>("p"), 0, 16);

			memory_module::disable_warnings();

			int ret = run_emulator_rom(rom_filename, false, abort_pc, checksum);

			return ret;
		}
		else
		{
			std::string rom_filename = parser.get<std::string>("r");

			int ret = run_emulator_rom(rom_filename);

			return ret;
		}

		return 0;
	}
	*/


	int init_emulator(const std::string& rom_filename)
	{
		// load and run the rom
		rom rom(rom_filename);

		// load the boot rom file
		bool success = framebuffer_texture.resize(sf::Vector2u(gpu::width, gpu::height));

		// init input map
		input_map[sf::Keyboard::Key::Left] = { DIRECTION_LEFT, true };
		input_map[sf::Keyboard::Key::Right] = { DIRECTION_RIGHT, true };
		input_map[sf::Keyboard::Key::Up] = { DIRECTION_UP, true };
		input_map[sf::Keyboard::Key::Down] = { DIRECTION_DOWN, true };
		input_map[sf::Keyboard::Key::A] = { BUTTON_A, false };
		input_map[sf::Keyboard::Key::B] = { BUTTON_B, false };
		input_map[sf::Keyboard::Key::Enter] = { BUTTON_START, false };
		input_map[sf::Keyboard::Key::RShift] = { BUTTON_SELECT, false };

		// init cpu and load rom
		warning("fix boot rom loading")

#ifdef USE_BOOT_ROM
		boot_rom boot("gameboy/boot.gb");
		memory_module::initialize(&boot, &rom);
#else
		memory_module::initialize(nullptr, &rom);
#endif

		cpu::initialize();
		gpu::initialize();

		gameboy_debugger.initialize(gpu::width * pixelSize, gpu::height * pixelSize);

		return 0;
	}

	int process_event(const sf::Event* event)
	{
		if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
		{
			if (keyPressed->code == sf::Keyboard::Key::F1)
			{
				show_debugger = !show_debugger;
			}

			if (show_debugger)
			{
				if (keyPressed->code == sf::Keyboard::Key::Space)
				{
					cpu::reset();
					gpu::reset();
					cycle_count = 0;
				}
				else if (keyPressed->code == sf::Keyboard::Key::F2)
				{
					u8* ptr = memory_module::get_memory(0x9800, true);
					u8* buffer = new u8[0x401];
					memset(buffer, 0x0, 0x401);
					memcpy(buffer, ptr, 0x400);
					//std::string checksum = buffer;

					printf("Checksum: %s", buffer);
				}
				else
				{
					gameboy_debugger.on_keypressed(keyPressed->code);
				}
			}
			else
			{
				// check for joypad input
				auto itr = input_map.find(keyPressed->code);

				if (itr != input_map.end())
				{
					// handle joypad input
					set_button_pressed(itr->second.joypad_map, itr->second.is_directional);
				}
			}
		}
		else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
		{
			if (show_debugger)
			{

			}
			else
			{
				// check for joypad input
				auto itr = input_map.find(keyReleased->code);

				if (itr != input_map.end())
				{
					// handle joypad input
					set_button_released(itr->second.joypad_map, itr->second.is_directional);
				}
			}
		}

		return 0;
	}

	int update()
	{
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

		// update the framebuffer
		if (gpu::vblank_occurred)
		{
			framebuffer_texture.update(gpu::framebuffer, sf::Vector2u(gpu::width, gpu::height), sf::Vector2u(0, 0));

			gpu::vblank_occurred = false;
		}

		return 0;
	}
	
	const sf::Texture* get_emulator_texture()
	{
		return &framebuffer_texture;
	}

	int debugger_update()
	{
		return gameboy_debugger.update();
	}

	int debugger_process_event(const sf::Event* event)
	{
		/*if (event->is<sf::Event::Closed>())
		{
			//gameboy_debugger.destroy();
		}*/

		if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
		{
			if (keyPressed->code == sf::Keyboard::Key::Space)
			{
				cpu::reset();
				gpu::reset();
				cycle_count = 0;
			}
			else if (keyPressed->code == sf::Keyboard::Key::F2)
			{
				u8* ptr = memory_module::get_memory(0x9800, true);
				u8* buffer = new u8[0x401];
				memset(buffer, 0x0, 0x401);
				memcpy(buffer, ptr, 0x400);
				//std::string checksum = buffer;

				printf("Checksum: %s", buffer);
			}
			else
			{
				gameboy_debugger.on_keypressed(keyPressed->code);
			}
		}

		return 0;
	}

	const sf::Texture* get_debugger_texture()
	{
		return &gameboy_debugger.window_texture.getTexture();
	}
}