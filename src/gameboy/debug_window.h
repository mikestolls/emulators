#pragma once

#include "defines.h"

#include <SFML/Graphics.hpp>

namespace gameboy
{
	struct debug_window
	{
		sf::RenderTexture window_texture;
		sf::Sprite window_sprite;
		
		debug_window()
		{

		}

		virtual ~debug_window()
		{

		}

		virtual void set_position(float x, float y)
		{
			window_sprite.setPosition(x, y);
		}

		virtual void set_scale(float scale)
		{
			window_sprite.setScale(scale, scale);
		}

		virtual void update()
		{

		}
	};
}