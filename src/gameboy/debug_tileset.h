#pragma once

#include "defines.h"
#include "debug_window.h"

#include <SFML/Graphics.hpp>

namespace gameboy
{
	struct debug_tileset : public debug_window
	{
		#define BORDER_SIZE					2
		#define TITLEBAR_SIZE				15

		#define TILESET_TEXTURE_SIZE		128
		
		sf::Texture tileset_texture;
		sf::Sprite tileset_sprite;
		u8 tileset_texture_data[TILESET_TEXTURE_SIZE * TILESET_TEXTURE_SIZE * 4]; // texture will 128 x 128 with 4 bpp

		sf::RectangleShape outer_border;

		sf::Font font;
		sf::Text title_text;

		debug_tileset() : debug_window()
		{
			// create window texture and sprite
			window_texture.create((TILESET_TEXTURE_SIZE * 2) + BORDER_SIZE * 2, (TILESET_TEXTURE_SIZE * 2) + BORDER_SIZE * 2 + TITLEBAR_SIZE);
			window_sprite.setTexture(window_texture.getTexture());

			// create window border
			outer_border.setSize(sf::Vector2f(window_texture.getSize()));
			outer_border.setFillColor(sf::Color(180, 180, 180, 255));

			// tileset sprite
			tileset_texture.create(TILESET_TEXTURE_SIZE, TILESET_TEXTURE_SIZE);
			tileset_sprite.setTexture(tileset_texture);
			tileset_sprite.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);
			tileset_sprite.setScale(2, 2);

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
			u8* tileset = memory_module::get_memory(0x8000);

			// render all 256 tiles
			for (u16 i = 0; i < 256; i++)
			{
				for (u16 y = 0; y < 8; y++)
				{
					// render the 8 x 8 tile
					u8 dataA = tileset[0];
					u8 dataB = tileset[1];

					for (u16 x = 0; x < 8; x++)
					{
						u8 bit = 7 - x; // the bits and pixels are inversed
						u8 color = ((dataA & (1 << bit)) >> bit) | (((dataB & (1 << bit)) >> bit) << 1);

						switch (color)
						{
						case 0x00: // white
							color = 0xFF;
							break;
						case 0x01: // light grey
							color = 0xCC;
							break;
						case 0x10: // dark grey
							color = 0x77;
							break;
						case 0x11: // black
							color = 0x0;
							break;
						}

						u16 xPos = (i % 16) * 8 + x;
						u16 yPos = (i / 16) * 8 + y;
						u16 pixelPos = (yPos * 128 + xPos) * 4; // the pixel we are drawing * 4 bytes per pixel
						tileset_texture_data[pixelPos++] = color;
						tileset_texture_data[pixelPos++] = color;
						tileset_texture_data[pixelPos++] = color;
						tileset_texture_data[pixelPos++] = 0xFF;
					}

					tileset += 2;
				}
			}

			tileset_texture.update(tileset_texture_data, TILESET_TEXTURE_SIZE, TILESET_TEXTURE_SIZE, 0, 0);

			// draw to the window texture
			window_texture.draw(outer_border);
			window_texture.draw(tileset_sprite);
			window_texture.draw(title_text);

			window_texture.display();
		}
	};
}