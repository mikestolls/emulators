#pragma once

#include <SFML/Graphics.hpp>

#include "defines.h"

#include "cpu.h"
#include "gpu.h"
#include "rom.h"
#include "cpu_memory_module.h"

namespace nes
{
	bool is_debugger_visible;
}

#include "debugger/debugger.h"

namespace nes
{
	sf::Texture framebuffer_texture;

	int init_emulator(const std::string& rom_filename)
	{
		// load and run the rom
		rom::load(rom_filename);

		// load the boot rom file
		bool success = framebuffer_texture.resize(sf::Vector2u(gpu::width, gpu::height));

		cpu_memory_module::initialize();

		cpu::initialize();

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

	int process_event(const sf::Event* event)
	{
		return 0;
	}

	int update(const sf::Time& deltaTime)
	{
		s32 cycles = cpu::cpu_cycles_per_frame;
		
		while (cycles >= 0)
		{
			cycles -= cpu::update();
		}

		// update 
		debugger::update_debugger(deltaTime);

		return 0;
	}
	
	const sf::Texture* get_emulator_texture()
	{
		return 0;
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