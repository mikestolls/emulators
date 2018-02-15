#pragma once

#include "defines.h"

#include "gameboy\memory_module.h"

namespace gameboy
{
	namespace gpu
	{
		const bool green_palette = true;

		u8* scanline = 0;
		u8* coincidence_scanline = 0;
		u8* lcd_control = 0;
		u8* lcd_status = 0;
		u8* scrollY = 0;
		u8* scrollX = 0;
		u8* windowX = 0;
		u8* windowY = 0;
		u8* palette_bg = 0;
		u8* sprite_attr = 0;

		const u8 width = 160;
		const u8 height = 144;
		u8 framebuffer[width * height * 4];
		bool lcd_enabled = false;
		s32 horz_cycle_count = 0;
				
		// set and get lcd control flag helpers
		inline void set_lcd_control_flag(u8 flag)
		{
			flag = (1 << flag);
			*lcd_control |= flag;
		}

		inline void clear_lcd_control_flag(u8 flag)
		{
			flag = (1 << flag);
			*lcd_control &= ~flag; // clear the bit
		}

		inline u8 get_lcd_control_flag(u8 flag)
		{
			return ((*lcd_control & (1 << flag)) >> flag);
		}

		inline void clear_all_lcd_control_flags()
		{
			*lcd_control = 0x0;
		}

		//Bit 0 - BG Display(for CGB see below) (0 = Off, 1 = On)
		//Bit 1 - OBJ(Sprite) Display Enable(0 = Off, 1 = On)
		//Bit 2 - OBJ(Sprite) Size(0 = 8x8, 1 = 8x16)
		//Bit 3 - BG Tile Map Display Select(0 = 9800 - 9BFF, 1 = 9C00 - 9FFF)
		//Bit 4 - BG & Window Tile Data Select(0 = 8800 - 97FF, 1 = 8000 - 8FFF)
		//Bit 5 - Window Display Enable(0 = Off, 1 = On)
		//Bit 6 - Window Tile Map Display Select(0 = 9800 - 9BFF, 1 = 9C00 - 9FFF)
		//Bit 7 - LCD Display Enable(0 = Off, 1 = On)

		enum LCD_CONTROL_FLAGS
		{
			FLAG_BG_DISPLAY_ENABLED = 0,
			FLAG_OBJ_DISPLAY_ENABLED,
			FLAG_OBJ_SIZE,
			FLAG_BG_TILEMAP_DISPLAY_SELECT,
			FLAG_BG_WINDOW_TILE_DISPLAY_SELECT,
			FLAG_WINDOW_DISPLAY_ENABLED,
			FLAG_WINDOW_TILEMAP_DISPLAY_SELECT,
			FLAG_LCD_DISPLAY_ENABLED,
		};

		// set and get lcd status mode
		inline void set_lcd_status_mode(u8 mode)
		{
			mode &= 0x3; // just incase
			*lcd_status &= 0xFC; // clear old mode bits
			*lcd_status |= mode;
			*lcd_status |= 0x80; // turn bit 7 on
		}

		inline u8 get_lcd_status_mode()
		{
			return *lcd_status & 0x3;
		}
		
		enum LCD_STATUS_MODES
		{
			MODE_HBLANK = 0,
			MODE_VBLANK,
			MODE_OAM_ACCESS,
			MODE_VRAM_ACCESS,
		};

		// lcd status interrupt flags
		inline void set_lcd_interrupt_flag(u8 flag)
		{
			flag = (1 << flag);
			*lcd_status |= flag;
		}

		inline void clear_lcd_interrupt_flag(u8 flag)
		{
			flag = (1 << flag);
			*lcd_status &= ~flag; // clear the bit
		}

		inline u8 get_lcd_interrupt_flag(u8 flag)
		{
			return ((*lcd_status & (1 << flag)) >> flag);
		}

		inline void clear_all_lcd_interrupt_flags()
		{
			*lcd_status &= 0x87; // take all but bits 3, 4, 5, 6
		}

		enum LCD_INTERRUPT_FLAGS
		{
			FLAG_HBLANK = 3,
			FLAG_VBLANK,
			FLAG_OAM_ACCESS,
			FLAG_COINCIDENCE
		};

		// get sprite attributes
		inline u8 get_sprite_attribute(u8 attribute, u8 flag)
		{
			return ((attribute & (1 << flag)) >> flag);
		}
		
		//Bit 0 - Not Used
		//Bit 1 - Not Used
		//Bit 2 - Not Used
		//Bit 3 - Not Used
		//Bit 4 - Palette Number (0 = 0xFF48, 1 = 0xFF49)
		//Bit 5 - X Flip (0 = None, 1 = Horizontal Flip)
		//Bit 6 - Y Flip (0 = None, 1 = Vertical Flip)
		//Bit 7 - Sprite to Background Priority

		enum SPRITE_ATTRIBUTE_FLAGS
		{
			FLAG_BIT_0 = 0,
			FLAG_BIT_1,
			FLAG_BIT_2,
			FLAG_BIT_3,
			FLAG_SPRITE_PALETTE,
			FLAG_SPRITE_FLIP_X,
			FLAG_SPRITE_FLIP_Y,
			FLAG_SPRITE_PRIORITY,
		};

		int reset()
		{
			horz_cycle_count = 0;
			lcd_enabled = false;
			memset(framebuffer, 0x0, sizeof(framebuffer));
			
			return 0;
		}

		int initialize()
		{
			// setup memory ptrs
			scanline = memory_module::get_memory(0xFF44, true);
			coincidence_scanline = memory_module::get_memory(0xFF45, true);
			lcd_control = memory_module::get_memory(0xFF40, true);
			lcd_status = memory_module::get_memory(0xFF41, true);
			scrollY = memory_module::get_memory(0xFF42, true);
			scrollX = memory_module::get_memory(0xFF43, true);
			windowY = memory_module::get_memory(0xFF4A, true);
			windowX = memory_module::get_memory(0xFF4B, true);
			palette_bg = memory_module::get_memory(0xFF47, true);
			sprite_attr = memory_module::get_memory(0xFE00, true);
			
			reset();

			return 0;
		}

		u32 get_palette_color(u8 palette_color)
		{
			u8 palette = *palette_bg;
			palette &= (0x3 << (palette_color * 2)); // take two bits from palette depending on the passed in color id
			palette >>= (palette_color * 2); // shift palette color back down

			u32 color = 0xFF; // alpha

			if (green_palette)
			{
				switch (palette)
				{
				case 0x00: // white
					color = 0xE0F8D0FF;
					break;
				case 0x1: // light grey
					color = 0x88C070FF;
					break;
				case 0x2: // dark grey
					color = 0x346856FF;
					break;
				case 0x3: // black
					color = 0x081820FF;
					break;
				}
			}
			else
			{
				switch (palette)
				{
				case 0x00: // white
					color = 0xFFFFFFFF;
					break;
				case 0x1: // light grey
					color = 0xCCCCCCFF;
					break;
				case 0x2: // dark grey
					color = 0x777777FF;
					break;
				case 0x3: // black
					color = 0x000000FF;
					break;
				}
			}

			return color;
		}

		int draw_scanline()
		{
			if (get_lcd_control_flag(FLAG_WINDOW_DISPLAY_ENABLED))
			{
//				warning_assert("window tilemap not implemented yet");
			}

			// only doing background. but will need to merge this with window
			if (get_lcd_control_flag(FLAG_BG_DISPLAY_ENABLED)) 
			{
				// tilemap starting location. tilemap is 32 x 32 bytes that map to a tile
				u16 tilemapAddr = 0x9800;
				if (get_lcd_control_flag(FLAG_BG_TILEMAP_DISPLAY_SELECT))
				{
					tilemapAddr = 0x9C00;
				}

				s32 tilesetOffset = 128; // offset depending on tileset used
				s32 tileSize = 16; // each tile is 16 bytes. 2 x 8 rows of a tile
				u16 tilesetAddr = 0x8800; // addr of tileset
				if (get_lcd_control_flag(FLAG_BG_WINDOW_TILE_DISPLAY_SELECT))
				{
					tilesetAddr = 0x8000;
					tilesetOffset = 0;
				}

				u8 yPos = *scrollY + *scanline;
				u32 tileY = (yPos / 8) * 32; // calc tile offset based on yPos. map is 32 tiles wide
				u8 tileYPixel = yPos % 8; // the row of the specific tile the scanline is on

				// draw the 160 horz pixels
				for (u32 pixel = 0; pixel < 160; pixel++)
				{
					u8 xPos = *scrollX + pixel;
					u32 tileX = xPos / 8; // calc tile offset base on xPos
					u8 tileXPixel = xPos % 8; // the column of the speific tile to draw
					s16 tileId = 0;
					
					if (tilesetOffset != 0) // its a signed value
					{
						tileId = (s8)memory_module::read_memory(tilemapAddr + tileX + tileY);
						tileId += tilesetOffset; // apply offset to the id
					}
					else
					{
						tileId = memory_module::read_memory(tilemapAddr + tileX + tileY);
					}

					// we have the tile id. lets draw pixel
					u8* tileset = memory_module::get_memory(tilesetAddr + (tileId * tileSize) + (tileYPixel * 2));
					u8 dataA = tileset[0];
					u8 dataB = tileset[1];
					u8 bit = 7 - tileXPixel; // the bits and pixels are inversed
					u8 palette_color = ((dataA & (1 << bit)) >> bit) | (((dataB & (1 << bit)) >> bit) << 1);

					u32 color = gpu::get_palette_color(palette_color);

					u32 pixelPos = ((*scanline) * 160 + pixel) * 4; // the pixel we are drawing * 4 bytes per pixel
					framebuffer[pixelPos++] = (color >> 24) & 0xFF;
					framebuffer[pixelPos++] = (color >> 16) & 0xFF;
					framebuffer[pixelPos++] = (color >> 8) & 0xFF;
					framebuffer[pixelPos++] = 0xFF;
				}
			}

			return 0;
		}

		int draw_sprites()
		{
			u8 spriteHeight = (get_lcd_control_flag(FLAG_OBJ_SIZE) == 0 ? 8 : 16);
			u8* spritePtr = sprite_attr;
			u8 sprite_count = 0;
			s32 tileSize = 16; // each tile is 16 bytes. 2 x 8 rows of a tile

			while (sprite_count < 40) // oam has room for 40 sprites with 4 bytes attr each sprite
			{
				sprite_count++;
				u8 yPos = *(spritePtr++) - 16;
				u8 xPos = *(spritePtr++) - 8;
				u8 tileId = *(spritePtr++);
				u8 attr = *(spritePtr++);

				// check if scanline within y_min y_max
				if (*scanline >= yPos && *scanline < yPos + spriteHeight)
				{
					s16 tileY = *scanline - yPos;

					if (get_sprite_attribute(attr, FLAG_SPRITE_FLIP_Y))
					{
						tileY -= spriteHeight;
						tileY *= -1;
					}

					u8* tileset = memory_module::get_memory(0x8000 + (tileId * tileSize) + (tileY * 2));
					u8 dataA = tileset[0];
					u8 dataB = tileset[1];

					// render the 8 pixels of the tiles scanline
					for (u8 pixel = 0; pixel < 8; pixel++)
					{
						s8 bit = 7 - pixel;

						if (get_sprite_attribute(attr, FLAG_SPRITE_FLIP_X))
						{
							bit -= 7;
							bit *= -1;
						}

						u8 palette_color = ((dataA & (1 << bit)) >> bit) | (((dataB & (1 << bit)) >> bit) << 1);

						if (palette_color == 0x0)
						{
							// pixel is transparent
							continue;
						}

						u32 color = gpu::get_palette_color(palette_color);

						u32 pixelPos = ((*scanline) * 160 + xPos + pixel) * 4; // the pixel we are drawing * 4 bytes per pixel
						framebuffer[pixelPos++] = (color >> 24) & 0xFF;
						framebuffer[pixelPos++] = (color >> 16) & 0xFF;
						framebuffer[pixelPos++] = (color >> 8) & 0xFF;
						framebuffer[pixelPos++] = 0xFF;
					}
				}
			}

			return 0;
		}

		int update_lcd_scanline()
		{
			u8 lcd_mode = get_lcd_status_mode();
			bool req_lcd_interrupt = false;

			switch (lcd_mode)
			{
			case MODE_HBLANK:
				if (horz_cycle_count >= 204)
				{
					(*scanline)++; // inc scanline
					set_lcd_status_mode(MODE_OAM_ACCESS);

					if (get_lcd_interrupt_flag(FLAG_OAM_ACCESS))
					{
						req_lcd_interrupt = true;
					}
				}
				break;
			case MODE_VBLANK:
				if (horz_cycle_count >= 456) // restart screen refresh
				{
					(*scanline)++; // inc scanline
					horz_cycle_count -= 456;

					if (*scanline >= 154)
					{
						*scanline = 0;
						set_lcd_status_mode(MODE_OAM_ACCESS);

						if (get_lcd_interrupt_flag(FLAG_OAM_ACCESS))
						{
							req_lcd_interrupt = true;
						}
					}
				}
				break;
			case MODE_OAM_ACCESS:
				if (horz_cycle_count >= 284)
				{
					set_lcd_status_mode(MODE_VRAM_ACCESS);
				}
				break;
			case MODE_VRAM_ACCESS:
				if (horz_cycle_count >= 456)
				{
					if (*scanline < 143)
					{
						set_lcd_status_mode(MODE_HBLANK);

						// draw the scan line
						draw_scanline();
						draw_sprites();

						if (get_lcd_interrupt_flag(FLAG_HBLANK))
						{
							req_lcd_interrupt = true;
						}
					}
					else // enter vblank
					{
						set_lcd_status_mode(MODE_VBLANK);

						// draw the scan line
						draw_scanline();
						draw_sprites();

						cpu::set_request_interrupt_flag(cpu::INTERRUPT_VBLANK);
					}
					horz_cycle_count -= 456;
				}
				break;
			}

			if (lcd_mode != get_lcd_status_mode())
			{
				if (req_lcd_interrupt)
				{
					// trigger an lcd interrupt
					cpu::set_request_interrupt_flag(cpu::INTERRUPT_LCD);
				}
			}

			return 0;
		}
		
		int update(u8 cycles)
		{
			if (*coincidence_scanline == *scanline)
			{
				*lcd_status |= (1 << 2); // set bit 2 for coincidence

				if (get_lcd_interrupt_flag(FLAG_COINCIDENCE))
				{
					cpu::set_request_interrupt_flag(cpu::INTERRUPT_LCD);
				}
			}
			else
			{
				*lcd_status &= 0xFB; // take all but bit 2. clear coincidence flag
			}

			if (lcd_enabled != (bool)get_lcd_control_flag(FLAG_LCD_DISPLAY_ENABLED))
			{
				// lcd not enabled. reset scanline and horz cycle count. lcd mode set to 1 (VBlank)
				*scanline = 0;
				lcd_enabled = (bool)get_lcd_control_flag(FLAG_LCD_DISPLAY_ENABLED);
				set_lcd_status_mode(lcd_enabled ? MODE_OAM_ACCESS : MODE_HBLANK);
			}

			if (!lcd_enabled)
			{
				return 0;
			}

			horz_cycle_count += cycles; // dec the horz cycles

			update_lcd_scanline();

			return 0;
		}
	}
}