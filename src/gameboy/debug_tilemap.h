#pragma once

#include "defines.h"
#include "debug_window.h"

#include <SFML/Graphics.hpp>

namespace gameboy
{
	struct debug_tilemap : public debug_window
	{
		#define BORDER_SIZE					2
		#define TITLEBAR_SIZE				15

		#define TILEMAP_TEXTURE_SIZE		256
		
		sf::Texture tilemap_texture;
		sf::Sprite tilemap_sprite;
		u8 tilemap_texture_data[256 * 256 * 4]; // texture will 128 x 128 with 4 bpp

		sf::RectangleShape outer_border;

		sf::Font font;
		sf::Text title_text;

		debug_tilemap() : debug_window()
		{
			// create window texture and sprite
			window_texture.create(TILEMAP_TEXTURE_SIZE + BORDER_SIZE * 2, TILEMAP_TEXTURE_SIZE + BORDER_SIZE * 2 + TITLEBAR_SIZE);
			window_sprite.setTexture(window_texture.getTexture());

			// create window border
			outer_border.setSize(sf::Vector2f(window_texture.getSize()));
			outer_border.setFillColor(sf::Color(180, 180, 180, 255));

			// create tilemap
			tilemap_texture.create(256, 256);
			tilemap_sprite.setTexture(tilemap_texture);
			tilemap_sprite.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);

			// title text
			font.loadFromFile("courbd.ttf");

			title_text.setString("Tileset");
			title_text.setFillColor(sf::Color(0, 0, 0, 255));
			title_text.setFont(font);
			title_text.setCharacterSize(14);
			title_text.setPosition(0, 0);
		}

		void update()
		{
			// render out tileset
			u8* tilemap = memory_module::get_memory(0x9800);

			// render 32 x 32 tilemap
			for (int i = 0; i < 1024; i++)
			{
				// get tile id
				u8* tileset = memory_module::get_memory(0x8000 + (tilemap[i] * 16));

				for (int y = 0; y < 8; y++)
				{
					// render the 8 x 8 tile
					u8 dataA = tileset[0];
					u8 dataB = tileset[1];

					for (int x = 0; x < 8; x++)
					{
						u8 bit = 7 - x; // the bits and pixels are inversed
						u8 color = ((dataA & (1 << bit)) >> bit) | (((dataB & (1 << bit)) >> bit) << 1);

						color = gpu::get_palette_color(color);

						u16 xPos = (i % 32) * 8 + x;
						u16 yPos = (i / 32) * 8 + y;
						u32 pixelPos = (yPos * 256 + xPos) * 4; // the pixel we are drawing * 4 bytes per pixel
						tilemap_texture_data[pixelPos++] = color;
						tilemap_texture_data[pixelPos++] = color;
						tilemap_texture_data[pixelPos++] = color;
						tilemap_texture_data[pixelPos++] = 0xFF;
					}

					tileset += 2;
				}
			}

			tilemap_texture.update(tilemap_texture_data, 256, 256, 0, 0);

			// draw to the window texture
			window_texture.draw(outer_border);
			window_texture.draw(tilemap_sprite);
			window_texture.draw(title_text);

			window_texture.display();
		}
	};
}