#pragma once

#include <SFML/Graphics.hpp>

#include "memory_module.h"
#include "gpu.h"

#include "debug_tileset.h"
#include "debug_tilemap.h"
#include "debug_registers.h"
#include "debug_disassembler.h"
#include "debug_memory.h"
#include "debug_palette.h"

#include "gameboy.h"

namespace gameboy
{
    class debugger_old
    {
    public:
        // render target for debugger
        sf::RenderTexture window_texture;
        sf::Sprite window_sprite = sf::Sprite(window_texture.getTexture());

        // debug windows
        std::vector<debug_window*> debug_windows;
        u16 debug_window_index;

        sf::RectangleShape bottom_bar;

        const float bottom_bar_height = 24;

        debugger_old()
        {

        }

		~debugger_old()
		{
			destroy();
		}

        int initialize(u32 width, u32 height)
        {
            // create window texture and sprite
            bool success = window_texture.resize(sf::Vector2u(width, height));
            window_sprite.setTexture(window_texture.getTexture(), true);

            // Set a flipped view to correct RenderTexture orientation
            sf::View view = window_texture.getView();
            sf::Vector2f viewSize = view.getSize();
            viewSize.y *= -1.0f;
            view.setSize(viewSize);
            window_texture.setView(view);

            // add bottom bar
            bottom_bar.setFillColor(sf::Color(100, 100, 100, 255));
            bottom_bar.setPosition(sf::Vector2f(0, height - bottom_bar_height));
            bottom_bar.setSize(sf::Vector2f((float)width, bottom_bar_height));

            // create debug window
            debug_window* window = new debug_tileset();
            window->set_position(16, 16);
            window->bottom_text.setPosition(sf::Vector2f(0, height - bottom_bar_height));
            debug_windows.push_back(window);

            window = new debug_tilemap((debug_tileset*)window);
            window->set_position(316, 16);
            window->bottom_text.setPosition(sf::Vector2f(0, height - bottom_bar_height));
            debug_windows.push_back(window);

            window = new debug_palette();
            window->set_position(616, 16);
            window->bottom_text.setPosition(sf::Vector2f(0, height - bottom_bar_height));
            debug_windows.push_back(window);

            window = new debug_registers();
            window->set_position(916, 16);
            window->bottom_text.setPosition(sf::Vector2f(0, height - bottom_bar_height));
            debug_windows.push_back(window);
            
            window = new debug_memory();
            window->set_position(16, 678);
            window->bottom_text.setPosition(sf::Vector2f(0, height - bottom_bar_height));
            debug_windows.push_back(window);

            window = new debug_disassembler();
            window->set_position(16, 314);
            window->bottom_text.setPosition(sf::Vector2f(0, height - bottom_bar_height));
            debug_windows.push_back(window);

            debug_window_index = (u16)debug_windows.size() - 1;
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

            return 0;
        }

        void on_keypressed(sf::Keyboard::Key key)
        {
            if (key == sf::Keyboard::Key::Tab)
            {
                u16 prev_index = debug_window_index;
                do
                {
                    debug_windows[debug_window_index]->set_active(false);

                    debug_window_index++;
                    if (debug_window_index >= debug_windows.size())
                    {
                        debug_window_index = 0;
                    }

                    if (debug_windows[debug_window_index]->is_selectable)
                    {
                        debug_windows[debug_window_index]->set_active(true);
                        break;
                    }
                } while (prev_index != debug_window_index);
            }
            else if (key == sf::Keyboard::Key::Space)
            {
                cpu::reset();
                gpu::reset();

#ifdef USE_BOOT_ROM
                boot_rom boot("gameboy/boot.gb");
                memory_module::initialize(&boot, &gameboy::loaded_rom);
#else
                memory_module::initialize(nullptr, &gameboy::loaded_rom);
#endif

                gameboy::cycle_count = 0;
            }
            else if (key == sf::Keyboard::Key::F2)
            {
                u8* ptr = memory_module::get_memory(0x9800, true);
                u8* buffer = new u8[0x401];
                memset(buffer, 0x0, 0x401);
                memcpy(buffer, ptr, 0x400);
                //std::string checksum = buffer;

                printf("Checksum: %s", buffer);
            }
            else
            {
                // forward remaining to active debug window
                debug_windows[debug_window_index]->on_keypressed(key);
            }
        }

        int update()
        {
            window_texture.clear(sf::Color(0, 0, 0, 100));

            window_texture.draw(bottom_bar);

            // update and render the windows
            for (auto itr = debug_windows.begin(); itr != debug_windows.end(); itr++)
            {
                (*itr)->update();

                window_texture.draw((*itr)->window_sprite);

                if ((*itr)->get_active())
                {
                    window_texture.draw((*itr)->bottom_text);
                }
            }

            // display debugger window
            window_texture.display();

            return 0;
        }
    };
}