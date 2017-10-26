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

		sf::Font font;

		// tileset sprite
		sf::Texture tileset_texture;
		sf::Sprite tileset_sprite;
		u8 tileset_texture_data[128 * 128 * 4]; // texture will 128 x 128 with 4 bpp

		// tilemap sprite
		sf::Texture tilemap_texture;
		sf::Sprite tilemap_sprite;
		u8 tilemap_texture_data[256 * 256 * 4]; // texture will 128 x 128 with 4 bpp

		// draw disassembly
		sf::Text disassembly_text;

		int initialize(gameboy::memory_module* memory)
		{
			memory_module = memory;

			// debugger
			window.create(sf::VideoMode(1024, 1024), "Debugger");

			// create tileset sprite
			tileset_texture.create(128, 128);
			tileset_sprite.setTexture(tileset_texture);
			tileset_sprite.setScale(4, 4);

			// create tilemap
			tilemap_texture.create(256, 256);
			tilemap_sprite.setTexture(tilemap_texture);
			tilemap_sprite.setScale(2, 2);
			tilemap_sprite.setPosition(0, 512);

			// disassembly
			font.loadFromFile("arial.ttf");

			disassembly_text.setString("");
			disassembly_text.setFont(font);
			disassembly_text.setCharacterSize(24);
			disassembly_text.setPosition(512, 0);

			return 0;
		}

		int close()
		{
			window.close();

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

		int update_tilemap()
		{
			// render out tileset
			u8* tilemap = memory_module->get_memory(0x9800);

			// render 32 x 32 tilemap
			for (int i = 0; i < 1024; i++)
			{
				// get tile id
				u8* tileset = memory_module->get_memory(0x8000 + (tilemap[i] * 16));

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


			return 0;
		}

		int update_disassembly()
		{
			std::stringstream stream;
			stream << "R.pc: 0x" << std::hex << cpu::R.pc << std::endl;
			stream << "R.a: 0x" << std::hex << (int)cpu::R.a << std::endl;
			stream << "R.f: 0x" << std::hex << (int)cpu::R.f << std::endl;
			stream << "R.b: 0x" << std::hex << (int)cpu::R.b << std::endl;
			stream << "R.c: 0x" << std::hex << (int)cpu::R.c << std::endl;
			stream << "R.d: 0x" << std::hex << (int)cpu::R.d << std::endl;
			stream << "R.e: 0x" << std::hex << (int)cpu::R.e << std::endl;
			stream << "R.h: 0x" << std::hex << (int)cpu::R.h << std::endl;
			stream << "R.l: 0x" << std::hex << (int)cpu::R.l << std::endl;
			stream << "R.af: 0x" << std::hex << cpu::R.af << std::endl;
			stream << "R.bc: 0x" << std::hex << cpu::R.bc << std::endl;
			stream << "R.de: 0x" << std::hex << cpu::R.de << std::endl;
			stream << "R.hl: 0x" << std::hex << cpu::R.hl << std::endl;
			stream << "R.sp: 0x" << std::hex << cpu::R.sp << std::endl;

			disassembly_text.setString(stream.str());

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
				else if (event.type == sf::Event::KeyPressed)
				{
					if (event.key.code == sf::Keyboard::F1)
					{
						window.setVisible(true);
					}
					else if (event.key.code == sf::Keyboard::F5)
					{
						cpu::debugging = !cpu::debugging;
						cpu::debugging_step = false;
						printf("Debugging: %s\n", (cpu::debugging ? "True" : "False"));
					}
					else if (event.key.code == sf::Keyboard::F10)
					{
						cpu::debugging_step = true;
						printf("Debugging Step\n");
					}
				}
			}

			// update debugging data
			update_tileset();
			update_tilemap();
			update_disassembly();

			window.clear();

			// render the display
			window.draw(tileset_sprite);
			window.draw(tilemap_sprite);
			window.draw(disassembly_text);

			window.display();

			return 0;
		}
	};
}