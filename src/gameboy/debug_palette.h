#pragma once

#include "defines.h"
#include "debug_window.h"
#include "debug_tileset.h"

#include <SFML/Graphics.hpp>

namespace gameboy
{
	class debug_palette : public debug_window
	{
	public:
		#define PALETTE_WINDOW_SIZE		256
		
        debug_palette() : debug_window(PALETTE_WINDOW_SIZE, PALETTE_WINDOW_SIZE)
        {
            is_selectable = false;
		
            title_text.setString("Palette");

			bottom_text.setString("");
		}

		void update()
		{
			// draw to the window texture
			window_texture.draw(outer_border);
			window_texture.draw(title_text);

			window_texture.display();
		}

		void on_keypressed(sf::Keyboard::Key key)
		{

		}
	};
}