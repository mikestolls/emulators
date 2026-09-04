#pragma once

#include <SFML/Graphics.hpp>

#include "defines.h"

#include "cpu.h"
#include "gpu.h"
#include "rom.h"
#include "cpu_memory_module.h"

namespace nes
{
	rom loaded_rom;

	sf::Texture framebuffer_texture;

	int init_emulator(const std::string& rom_filename)
	{
		// load and run the rom
		loaded_rom.load(rom_filename);

		// load the boot rom file
		bool success = framebuffer_texture.resize(sf::Vector2u(gpu::width, gpu::height));

		cpu_memory_module::initialize(&loaded_rom);

		cpu::initialize();

		return 0;
	}

	int destroy_emulator()
	{
		return 0;
	}

	int process_event(const sf::Event* event)
	{
		return 0;
	}

	int update(const sf::Time& deltaTime)
	{
		u8 cycles = cpu::update();

		return 0;
	}
	
	const sf::Texture* get_emulator_texture()
	{
		return 0;
	}

	int set_debugger_visible(bool visible)
	{
		return 0;
	}

	bool get_debugger_visible()
	{
		return false;
	}
}