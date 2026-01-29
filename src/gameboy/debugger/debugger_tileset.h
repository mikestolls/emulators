#pragma once

#include "defines.h"

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include "debugger_helper.h"

namespace gameboy
{
	namespace debugger
	{
        namespace tileset
        {
            u8 tileset_index;
            u8 tileset_texture_data[TILESET_TEXTURE_SIZE * TILESET_TEXTURE_SIZE * 4]; // texture will 128 x 128 with 4 bpp

            sf::Texture tileset_texture;
            sf::Vector2f tileset_size;

            int init()
            {
                bool success = tileset_texture.resize(sf::Vector2u(TILESET_TEXTURE_SIZE, TILESET_TEXTURE_SIZE));

                if (!success)
                {
                    printf("Error - Resizing debugger texture\n");
                    return -1;
                }

                tileset_size = sf::Vector2f(TILESET_TEXTURE_SIZE * 2.0f, TILESET_TEXTURE_SIZE * 2.0f);

                return 0;
            }

            int update()
            {
                // render out tileset
                u16 addr = 0x8000;
                if (tileset_index != 0)
                {
                    addr = 0x8800;
                }

                u8* tileset = memory_module::get_memory(addr, true);

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

                tileset_texture.update(tileset_texture_data, sf::Vector2u(TILESET_TEXTURE_SIZE, TILESET_TEXTURE_SIZE), sf::Vector2u(0, 0));

                return 0;
            }

            int draw(bool is_focused)
            {
                // title changes based on tileset index
                std::string title_str = "Tileset: ";
                if (tileset_index)
                {
                    title_str.append("0x8800");
                }
                else
                {
                    title_str.append("0x8000");
                }

                if (debugger::debugger_panel_begin(title_str.c_str(), ImVec2(tileset_size.x, tileset_size.y), is_focused, 0.7f))
                {
                    ImGui::Image(tileset_texture, tileset_size);

                    debugger::debugger_panel_end();
                }

                return 0;
            }

            int process_event(const sf::Event* event)
            {
                if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
                {
                    if (keyPressed->code == sf::Keyboard::Key::Left || keyPressed->code == sf::Keyboard::Key::Right)
                    {
                        tileset_index ^= 1;
                    }
                }      

                return 0;
            }
        }
	}
}
