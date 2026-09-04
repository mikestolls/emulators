#pragma once

#include <SFML/Graphics.hpp>
#include <argparse.h>

#include "defines.h"

#include "cpu.h"
#include "input.h"
#include "gpu.h"
#include "apu.h"
#include "rom.h"
#include "boot_rom.h"
#include "disassembler.h"

//#define USE_BOOT_ROM

namespace gameboy
{
	rom loaded_rom;
	u32 cycle_count = 0;

	bool is_debugger_visible;
}

#include "debugger/debugger.h"

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
	sf::Texture framebuffer_texture;

	int init_emulator(const std::string& rom_filename)
	{
		// load and run the rom
		loaded_rom.load(rom_filename);

		// load the boot rom file
		bool success = framebuffer_texture.resize(sf::Vector2u(gpu::width, gpu::height));

		// init input map
		input_map[sf::Keyboard::Key::Left] = { input::DIRECTION_LEFT, true };
		input_map[sf::Keyboard::Key::Right] = { input::DIRECTION_RIGHT, true };
		input_map[sf::Keyboard::Key::Up] = { input::DIRECTION_UP, true };
		input_map[sf::Keyboard::Key::Down] = { input::DIRECTION_DOWN, true };
		input_map[sf::Keyboard::Key::A] = { input::BUTTON_A, false };
		input_map[sf::Keyboard::Key::B] = { input::BUTTON_B, false };
		input_map[sf::Keyboard::Key::Enter] = { input::BUTTON_START, false };
		input_map[sf::Keyboard::Key::RShift] = { input::BUTTON_SELECT, false };

		// init cpu and load rom
#ifdef USE_BOOT_ROM
		boot_rom boot("gameboy/boot.gb");
		memory_module::initialize(&boot, &loaded_rom);
#else
		memory_module::initialize(nullptr, &loaded_rom);
#endif

		cpu::initialize();
		gpu::initialize();
		apu::initialize();

		debugger::init_debugger();
		debugger::window.setVisible(false);
		is_debugger_visible = false;

		return 0;
	}

	int destroy_emulator()
	{
		debugger::destroy_debugger();

		return 0;
	}

	void check_test_status()
	{
		static bool is_ended = false;
		static u16 last_pc = 0;

		// Check if PC is stuck (test might be done)
		if (!is_ended)
		{
			if (cpu::R.pc == last_pc && cpu::R.pc != 0)
			{
				// Test might be finished, check result register
				u8 result = gameboy::memory_module::read_memory(0xA000, true);  // Common result address

				if (result != 0)
				{
					printf("Test result at 0xA000: 0x%02X\n", result);

					// Read result string if available
					for (int i = 0; i < 256; i++)
					{
						u8 c = gameboy::memory_module::read_memory(0xA004 + i, true);
						if (c == 0) break;
						printf("%c", c);
					}
					printf("\n");

					is_ended = true;
				}
			}
		}

		last_pc = cpu::R.pc;
	}

	int process_event(const sf::Event* event)
	{
		if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
		{
			// check for joypad input
			auto itr = input_map.find(keyPressed->code);

			if (itr != input_map.end())
			{
				// handle joypad input
				input::set_button_pressed(itr->second.joypad_map, itr->second.is_directional);
			}
		}
		else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
		{
			// check for joypad input
			auto itr = input_map.find(keyReleased->code);

			if (itr != input_map.end())
			{
				// handle joypad input
				input::set_button_released(itr->second.joypad_map, itr->second.is_directional);
			}
		}

		return 0;
	}

	int update_peripherals(u8 cycles)
	{
		u8 peripheral_cycle = cycles;
		if (cpu::is_double_speed)
		{
			peripheral_cycle >>= 1;
		}


		gpu::update(peripheral_cycle);
		apu::update(peripheral_cycle);

		return 0;
	}

	int update(const sf::Time& deltaTime)
	{
		u32 cycles_needed = cpu::cycles_per_frame;
		if (cpu::is_double_speed)
		{
			cycles_needed *= 2;  // Need 2x CPU cycles in double-speed for same real-time duration
		}

		// check if vblank happened with proper amount of cycles

		while (!gpu::vblank_occurred)
		{
			// update cpu to fetch new opcode or execute mico ops. 1 micro op at a time. 
			u8 cycles = cpu::update();

			cpu::update_timer(cycles);

			if (cpu::paused)
			{
				// cpu is paused
				break;
			}

			update_peripherals(cycles);

			//check_test_status();

			if (cpu::is_opcode_complete)
			{
				// Check for interrupts after instruction
				cycles = cpu::check_interrupts();
				cpu::update_timer(cycles);

				update_peripherals(cycles);
			}
		}

		if (!cpu::paused && cpu::running)
		{
			// once we have passed cycles per frame reset cycle count
			cycle_count -= cycles_needed;
		}

		// update the framebuffer
		framebuffer_texture.update(gpu::framebuffer, sf::Vector2u(gpu::width, gpu::height), sf::Vector2u(0, 0));
		gpu::vblank_occurred = false;

		// update 
		debugger::update_debugger(deltaTime);

		return 0;
	}
	
	const sf::Texture* get_emulator_texture()
	{
		return &framebuffer_texture;
	}

	int set_debugger_visible(bool visible)
	{
		is_debugger_visible = visible;
		debugger::window.setVisible(is_debugger_visible);

		return 0;
	}

	bool get_debugger_visible()
	{
		return is_debugger_visible;
	}
}