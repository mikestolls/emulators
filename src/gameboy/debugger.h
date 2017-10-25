#pragma once

#include <SFML/Graphics.hpp>

#include "memory_module.h"

namespace gameboy
{
	class debugger
	{
	public:
		gameboy::memory_module* memory_module;

		// debugger sprites and textures
		sf::RenderWindow window;

		// tileset sprite
		sf::Texture tileset_texture;
		sf::Sprite tileset_sprite;
		u8 tileset_texture_data[128 * 128 * 4]; // texture will 128 x 128 with 4 bpp

		int initialize(gameboy::memory_module* memory)
		{
			memory_module = memory;

			// debugger
			window.create(sf::VideoMode(512, 512), "Debugger");

			// create tileset sprite
			tileset_texture.create(128, 128);
			tileset_sprite.setTexture(tileset_texture);
			tileset_sprite.setScale(4, 4);

			return 0;
		}

		int update_tileset()
		{
			// render out tileset
			u8* tileset = memory_module->get_memory(0x8000);

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

			tileset_texture.update(tileset_texture_data, 128, 128, 0, 0);

			return 0;
		}

		int update()
		{
			sf::Event event;
			
			// poll for debugger
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
				{
					window.setVisible(false);
				}
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::F1))
			{
				window.setVisible(true);
			}

			update_tileset();

			// render the display
			window.draw(tileset_sprite);

			window.display();

			return 0;
		}
	};
}