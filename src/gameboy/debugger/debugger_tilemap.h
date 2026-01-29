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
        namespace tilemap
        {
            #define TILEMAP_TEXTURE_SIZE		256

            u8 tilemap_index;
            u8 tilemap_texture_data[256 * 256 * 4]; // texture will 128 x 128 with 4 bpp

            sf::Texture tilemap_texture;
            sf::Vector2f tilemap_size;

            int init()
            {
                bool success = tilemap_texture.resize(sf::Vector2u(TILEMAP_TEXTURE_SIZE, TILEMAP_TEXTURE_SIZE));

                if (!success)
                {
                    printf("Error - Resizing debugger texture\n");
                    return -1;
                }

                tilemap_size = sf::Vector2f(TILEMAP_TEXTURE_SIZE, TILEMAP_TEXTURE_SIZE);

                return 0;
            }

            int update()
            {
                // render out tileset
                u16 addr = 0x9800;
                if (tilemap_index != 0)
                {
                    addr = 0x9C00;
                }

                u8* tilemap = memory_module::get_memory(addr, true);

                // render 32 x 32 tilemap
                for (int i = 0; i < 1024; i++)
                {
                    s32 tilesetOffset = 128; // offset depending on tileset used
                    u16 tilesetAddr = 0x8800; // addr of tileset
                    warning("TODO - implement tilemap index debugger");
                    //if (tileset_debug->tileset_index == 0)
                    {
                        tilesetAddr = 0x8000;
                        tilesetOffset = 0;
                    }

                    // get tile id
                    s32 tileId = (s8)tilemap[i] + tilesetOffset;
                    u8* tileset = memory_module::get_memory(tilesetAddr + (tileId * 16), true);

                    for (int y = 0; y < 8; y++)
                    {
                        // render the 8 x 8 tile
                        u8 dataA = tileset[0];
                        u8 dataB = tileset[1];

                        for (int x = 0; x < 8; x++)
                        {
                            u8 bit = 7 - x; // the bits and pixels are inversed
                            u8 palette_color = ((dataA & (1 << bit)) >> bit) | (((dataB & (1 << bit)) >> bit) << 1);

                            u32 color = gpu::get_palette_color(palette_color);

                            u16 xPos = (i % 32) * 8 + x;
                            u16 yPos = (i / 32) * 8 + y;
                            u32 pixelPos = (yPos * 256 + xPos) * 4; // the pixel we are drawing * 4 bytes per pixel

                            tilemap_texture_data[pixelPos++] = (color >> 24) & 0xFF;
                            tilemap_texture_data[pixelPos++] = (color >> 16) & 0xFF;
                            tilemap_texture_data[pixelPos++] = (color >> 8) & 0xFF;
                            tilemap_texture_data[pixelPos++] = 0xFF;
                        }

                        tileset += 2;
                    }
                }

                tilemap_texture.update(tilemap_texture_data, sf::Vector2u(256, 256), sf::Vector2u(0, 0));

                return 0;
            }

            int draw(bool is_focused)
            {
                // title changes based on tileset index
                std::string title_str = "Tilemap: ";
                if (tilemap_index)
                {
                    title_str.append("0x9C00");
                }
                else
                {
                    title_str.append("0x9800");
                }

                if (debugger::debugger_panel_begin(title_str.c_str(), ImVec2(tilemap_size.x, tilemap_size.y), is_focused, 0.9f))
                {
                    ImGui::Image(tilemap_texture, tilemap_size);

                    debugger::debugger_panel_end();
                }

                return 0;
            }

            int process_event(const sf::Event * event)
            {
                if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
                {
                    if (keyPressed->code == sf::Keyboard::Key::Left || keyPressed->code == sf::Keyboard::Key::Right)
                    {
                        tilemap_index ^= 1;
                    }
                }

                return 0;
            }
        }
    }
}
