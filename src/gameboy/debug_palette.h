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
        #define BOX_SIZE		        16

        #define PALETTE_X				32
        #define PALETTE_Y				32
        #define PALETTE_GAP 			24

        #define PALETTE_WINDOW_SIZE		256

        sf::RectangleShape inner_border;

        sf::RectangleShape box_outer_border;
        sf::RectangleShape box_inner_border;

        debug_palette() : debug_window(PALETTE_WINDOW_SIZE, PALETTE_WINDOW_SIZE)
        {
            is_selectable = false;

            inner_border.setSize(sf::Vector2f(256, 256));
            inner_border.setFillColor(sf::Color(0, 0, 0, 255));
            inner_border.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);

            title_text.setString("Palette");

            box_outer_border.setSize(sf::Vector2f(BOX_SIZE + BORDER_SIZE * 2, BOX_SIZE + BORDER_SIZE * 2));
            box_outer_border.setFillColor(sf::Color(200, 200, 200, 255));
            box_outer_border.setPosition(0, 0);

            box_inner_border.setSize(sf::Vector2f(BOX_SIZE, BOX_SIZE));
            box_inner_border.setFillColor(sf::Color(0, 0, 0, 255));
            box_inner_border.setPosition(BORDER_SIZE, BORDER_SIZE);

            bottom_text.setString("");
        }

        void update()
        {
            // draw to the window texture
            window_texture.draw(outer_border);
            window_texture.draw(inner_border);
            window_texture.draw(title_text);

            // draw the palette boxes
            float x = PALETTE_X;
            float y = PALETTE_Y;
            for (unsigned i = 0; i < 4; i++)
            {
                u32 color = gpu::get_palette_color(i);

                box_outer_border.setPosition(x, y);
                box_inner_border.setPosition(x + BORDER_SIZE, y + BORDER_SIZE);
                box_inner_border.setFillColor(sf::Color(color));

                window_texture.draw(box_outer_border);
                window_texture.draw(box_inner_border);
                
                y += PALETTE_GAP;
            }

            window_texture.display();
        }

        void on_keypressed(sf::Keyboard::Key key)
        {

        }
    };
}