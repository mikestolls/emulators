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
		std::vector<debug_window*> debug_windows;
		u16 debug_window_index;
				
		int initialize()
		{
			// debugger
			window.create(sf::VideoMode(1024, 1024), "Debugger");
			
			// create debug windows
			debug_window* window = new debug_tileset();
			window->set_position(16, 16);
			debug_windows.push_back(window);

			window = new debug_tilemap((debug_tileset*)window);
			window->set_position(16, 300);
			debug_windows.push_back(window);

			window = new debug_registers();
			window->set_position(300, 16);
			debug_windows.push_back(window);

			debug_window_index = 0;
			debug_windows[debug_window_index]->set_active(true);

			return 0;
		}

		int destroy()
		{
			while (debug_windows.size() > 0)
			{
				debug_window* window = *(--debug_windows.end());
				debug_windows.pop_back();
				delete window;
			}

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
					else if (event.key.code == sf::Keyboard::Tab)
					{
						debug_windows[debug_window_index]->set_active(false);

						debug_window_index++;
						if (debug_window_index >= debug_windows.size())
						{
							debug_window_index = 0;
						}

						debug_windows[debug_window_index]->set_active(true);
					}
					else
					{
						// forward remaining to active debug window
						debug_windows[debug_window_index]->on_keypressed(event.key.code);
					}
				}
			}

			window.clear();
			
			// update and render the windows
			for (auto itr = debug_windows.begin(); itr != debug_windows.end(); itr++)
			{
				(*itr)->update();
				window.draw((*itr)->window_sprite);
			}
						
			// display debugger window
			window.display();

			return 0;
		}
	};
}