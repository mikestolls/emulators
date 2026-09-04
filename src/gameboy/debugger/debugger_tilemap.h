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

            enum
            {
                MODE_AUTO = 0,
                MODE_FORCE_0,
                MODE_FORCE_1,
            };

            u8 tilemap_mode;
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

                tilemap_index = 0;
                tilemap_mode = MODE_AUTO;
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

                // need to check if tilemap is using signed or unsigned mode
                bool unsigned_mode = false;
                if (tilemap_mode == MODE_AUTO)
                {
                    unsigned_mode = (*gpu::lcd_control & 0x10) != 0;  // Read LCDC bit 4
                }
                else if (tilemap_mode == MODE_FORCE_0)
                {
                    unsigned_mode = true;  // Force 0x8000 mode
                }
                else
                {
                    unsigned_mode = false; // Force 0x8800 mode
                }

                s32 tilesetOffset = unsigned_mode ? 0 : 128; // offset depending on tileset used
                u16 tilesetAddr = unsigned_mode ? 0x8000 : 0x8800; // addr of tileset

                // render 32 x 32 tilemap
                for (int i = 0; i < 1024; i++)
                {
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
                std::string title_str = "Tilemap:";
                title_str.append(tilemap_index ? "(0x9C00)" : "(0x9800)");
                title_str.append("|");

                if (tilemap_mode == MODE_AUTO)
                {
                    bool using_8000 = (*gpu::lcd_control & 0x10) != 0;
                    title_str.append(using_8000 ? "AUTO(0x8000)" : "AUTO(0x8800)");
                }
                else if (tilemap_mode == MODE_FORCE_0)
                {
                    title_str.append("FORCE(0x8000)");
                }
                else
                {
                    title_str.append("FORCE(0x8800)");
                }

                if (debugger::debugger_panel_begin(title_str.c_str(), ImVec2(tilemap_size.x, tilemap_size.y), is_focused, 0.7f))
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
                    else if (keyPressed->code == sf::Keyboard::Key::T)
                    {
                        tilemap_mode = (tilemap_mode + 1) % 3;
                    }
                }

                return 0;
            }
        }
    }
}
