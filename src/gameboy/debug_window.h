#pragma once

#include "defines.h"

#include <SFML/Graphics.hpp>

#define BORDER_SIZE					2
#define TITLEBAR_SIZE				18

namespace gameboy
{
    class debug_window
    {
    public:
        sf::RenderTexture window_texture;
        sf::Sprite window_sprite;

        sf::RectangleShape outer_border;

        sf::Font font;
        sf::Text title_text;

        sf::Text bottom_text;

        bool is_active;

        bool is_selectable;

        debug_window(u32 width, u32 height)
        {
            // create window texture and sprite
            window_texture.create(width + BORDER_SIZE * 2, height + BORDER_SIZE * 2 + TITLEBAR_SIZE);
            window_sprite.setTexture(window_texture.getTexture());

            // create window border
            outer_border.setSize(sf::Vector2f(window_texture.getSize()));

            // title text
            font.loadFromFile("courbd.ttf");

            title_text.setString("");
            title_text.setFillColor(sf::Color(0, 0, 0, 255));
            title_text.setFont(font);
            title_text.setCharacterSize(16);
            title_text.setPosition(BORDER_SIZE, 0);

            bottom_text.setFont(font);
            bottom_text.setCharacterSize(16);
            bottom_text.setString("");

            is_selectable = true;

            set_active(false);
        }

        virtual ~debug_window()
        {

        }

        virtual void set_position(float x, float y)
        {
            window_sprite.setPosition(x, y);
        }

        virtual void set_scale(float scale)
        {
            window_sprite.setScale(scale, scale);
        }

        virtual void update()
        {

        }

        virtual void on_keypressed(sf::Keyboard::Key key)
        {

        }

        void set_active(bool active)
        {
            is_active = active;

            if (is_active)
            {
                outer_border.setFillColor(sf::Color(200, 200, 200, 255));
            }
            else
            {
                outer_border.setFillColor(sf::Color(80, 80, 80, 255));
            }
        }

        bool get_active()
        {
            return is_active;
        }
    };
}