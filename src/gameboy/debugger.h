#pragma once

#include <SFML/Graphics.hpp>

#include "cpu.h"
#include "gpu.h"
#include "rom.h"

namespace gameboy
{
	namespace debugger
	{
		// debugger sprites and textures
		sf::RenderWindow window;
		sf::Texture tileset_texture;

		int initialize()
		{
			// debugger
			window.create(sf::VideoMode(512, 512), "Debugger");

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

			return 0;
		}
	}
}