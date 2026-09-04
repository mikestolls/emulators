#pragma once

#include <SFML/Graphics.hpp>

#include "defines.h"

namespace nes
{
	int init_emulator(const std::string& rom_filename)
	{
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