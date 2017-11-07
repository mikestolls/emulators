#pragma once

#include "defines.h"

#include <SFML/Graphics.hpp>

#define BORDER_SIZE					2
#define TITLEBAR_SIZE				18

namespace gameboy
{
	struct debug_window
	{
		sf::RenderTexture window_texture;
		sf::Sprite window_sprite;

		sf::RectangleShape outer_border;

		sf::Font font;
		sf::Text title_text;

		debug_window(u32 width, u32 height)
		{
			// create window texture and sprite
			window_texture.create(width + BORDER_SIZE * 2, height + BORDER_SIZE * 2 + TITLEBAR_SIZE);
			window_sprite.setTexture(window_texture.getTexture());

			// create window border
			outer_border.setSize(sf::Vector2f(window_texture.getSize()));
			outer_border.setFillColor(sf::Color(180, 180, 180, 255));

			// title text
			font.loadFromFile("courbd.ttf");

			title_text.setString("");
			title_text.setFillColor(sf::Color(0, 0, 0, 255));
			title_text.setFont(font);
			title_text.setCharacterSize(16);
			title_text.setPosition(0, 0);
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