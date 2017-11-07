#pragma once

#include <SFML/Graphics.hpp>

#include "memory_module.h"
#include "gpu.h"

#include "debug_tileset.h"
#include "debug_tilemap.h"
#include "debug_registers.h"

namespace gameboy
{
	class debugger
	{
	public:
		// debugger sprites and textures
		sf::RenderWindow window;

		// debug windows
		debug_tileset tileset_window;
		debug_tilemap tilemap_window;
		debug_registers registers_window;
		
		int initialize()
		{
			// debugger
			window.create(sf::VideoMode(1024, 1024), "Debugger");

			// create tileset sprite
			tileset_window.set_position(16, 16);
			tilemap_window.set_position(16, 300);
			registers_window.set_position(300, 16);
			
			return 0;
		}

		int close()
		{
			window.close();

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

			tileset_window.update();
			tilemap_window.update();
			registers_window.update();

			window.clear();
						
			// render debugger windows
			window.draw(tileset_window.window_sprite);
			window.draw(tilemap_window.window_sprite);
			window.draw(registers_window.window_sprite);

			// display debugger window
			window.display();

			return 0;
		}
	};
}