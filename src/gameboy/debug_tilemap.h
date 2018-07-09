#pragma once

#include "defines.h"
#include "debug_window.h"
#include "debug_tileset.h"

#include <SFML/Graphics.hpp>

namespace gameboy
{
    class debug_tilemap : public debug_window
    {
    public:
        #define TILEMAP_TEXTURE_SIZE		256
        
        sf::Texture tilemap_texture;
        sf::Sprite tilemap_sprite;
        u8 tilemap_texture_data[256 * 256 * 4]; // texture will 128 x 128 with 4 bpp

        u8 tilemap_index;

        debug_tileset* tileset_debug;

        debug_tilemap(debug_tileset* tileset) : debug_window(TILEMAP_TEXTURE_SIZE, TILEMAP_TEXTURE_SIZE)
        {
            tileset_debug = tileset;

            // create tilemap
            tilemap_texture.create(256, 256);
            tilemap_sprite.setTexture(tilemap_texture);
            tilemap_sprite.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);

            title_text.setString("Tilemap: 0x9800");

            bottom_text.setString("(Left / Right) Change Tilemap");

            tilemap_index = 0;
        }

        void update()
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
                if (tileset_debug->tileset_index == 0)
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

            tilemap_texture.update(tilemap_texture_data, 256, 256, 0, 0);

            // draw to the window texture
            window_texture.draw(outer_border);
            window_texture.draw(tilemap_sprite);
            window_texture.draw(title_text);

            window_texture.display();
        }

        void on_keypressed(sf::Keyboard::Key key)
        {
            if (key == sf::Keyboard::Left || key == sf::Keyboard::Right)
            {
                tilemap_index ^= 1;

                std::string title_str = "Tilemap: ";
                if (tilemap_index)
                {
                    title_str.append("0x9C00");
                }
                else
                {
                    title_str.append("0x9800");
                }

                title_text.setString(title_str);
            }
        }
    };
}