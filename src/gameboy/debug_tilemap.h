#pragma once

#include "defines.h"
#include "debug_window.h"

#include <SFML/Graphics.hpp>

namespace gameboy
{
	struct debug_tilemap : public debug_window
	{
		#define TILEMAP_TEXTURE_SIZE		256
		
		sf::Texture tilemap_texture;
		sf::Sprite tilemap_sprite;
		u8 tilemap_texture_data[256 * 256 * 4]; // texture will 128 x 128 with 4 bpp

		debug_tilemap() : debug_window(TILEMAP_TEXTURE_SIZE, TILEMAP_TEXTURE_SIZE)
		{
			// create tilemap
			tilemap_texture.create(256, 256);
			tilemap_sprite.setTexture(tilemap_texture);
			tilemap_sprite.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);

			title_text.setString("Tilemap");
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